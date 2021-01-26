#include <stdio.h>
#include <signal.h>

#include <unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#include <errno.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>


#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <zlib.h>


#include <string.h>
#include <iostream>
using namespace std;

typedef struct
{
	FILE* fp_read;
	int read_compeleted;
	int read_ok;
	z_stream* pout_stream;
}client_ctx;

#if 0
static void resource_free(event_base *pbase, bufferevent *pbev)
{
	if(pbase)
	{
		event_base_free(pbase);
	}

	if(pbev)
	{
		bufferevent_free(pbev);
	}
}
#endif


static void bev_client_read(struct bufferevent *bev, void *ctx)
{
	cout << "in bev_client_read" << endl;

	client_ctx* pctx = (client_ctx*)ctx;

	char temp[1024] = {0};
	int read_len = -1;
	while((read_len = bufferevent_read(bev, temp, sizeof(temp) - 1)) > 0)
	{
		cout << "client read_len:" << read_len << endl;
		cout << "read data:" << temp << endl;
	}

	cout << "client read completed" << endl;

	if(0 == strcmp(temp, "OK"))
	{
		if(0 == pctx->read_ok)
		{
			pctx->read_ok = 1;

			cout << "===========recv OK==============    start send" << endl;
			bufferevent_trigger(bev, EV_WRITE, 0);
			return;
		}	
	}
}

static void bev_client_write(struct bufferevent *bev, void *ctx)
{
	cout << "in bev_client_write" << endl;

	client_ctx* pctx = (client_ctx*)ctx;

	if(pctx->read_compeleted)
	{
		bufferevent *praw_bev = bufferevent_get_underlying(bev);
		evbuffer *pbuffer = bufferevent_get_output(praw_bev);
		int left_len = evbuffer_get_length(pbuffer);
		if(left_len <= 0)
		{
			cout << "bufferevent_free" << endl;
			bufferevent_free(bev);
			delete pctx->pout_stream;
			delete pctx;
			return;
		}

		bufferevent_flush(bev, EV_WRITE, BEV_FINISHED);
		cout << "bufferevent_flush" << endl;
		return;
	}

	char temp[1024] = {0};
	int len = fread(temp, 1, sizeof(temp), pctx->fp_read);
	if(len <= 0)
	{
		cout << "read file compeleted!!!" << endl;

		fclose(pctx->fp_read);
		pctx->fp_read = NULL;
		pctx->read_compeleted = 1;
		return;
	}

	if(len > 0)
	{
		cout << "write len:" << len << endl << endl;
		bufferevent_write(bev, temp, len);		
	}	
}

static bufferevent_filter_result bev_filter_out(struct evbuffer *src, struct evbuffer *dst, ev_ssize_t dst_limit,
												enum bufferevent_flush_mode mode, void *ctx)
{
	cout << "bev_filter_out" << endl;

	client_ctx* pctx = (client_ctx*)ctx;

	if(0 == pctx->read_ok) 
	{
		char temp[1024] = {0};
		int len = evbuffer_remove(src, temp, sizeof(temp));
		if(evbuffer_add(dst, temp, len) < 0)
		{
			cout << "evbuffer_add error!!!" << endl;
			return BEV_ERROR;
		}

		cout << "fliter out send raw data, len:" << len << endl;

		return BEV_OK;
	}

	evbuffer_iovec v_in[1];
	int len = evbuffer_peek(src, -1, 0, v_in, 1);
	if(len <= 0)
	{
		cout << "fliter dont find data, len:" << len << endl;
		return BEV_OK;
	}

	z_stream* pzstream = pctx->pout_stream;
	if(!pzstream)
	{
		cout << "get z stream error!!!" << endl;
		return BEV_ERROR;
	}

	pzstream->avail_in = v_in[0].iov_len;
	pzstream->next_in = (Byte*)v_in[0].iov_base;

	evbuffer_iovec v_out[1];
	evbuffer_reserve_space(dst, 4096, v_out, 1);

	pzstream->avail_out = v_out[0].iov_len;
	pzstream->next_out = (Byte*)v_out[0].iov_base;

	int ret = deflate(pzstream, Z_SYNC_FLUSH);
	if(ret != Z_OK)
	{
		cout << "deflate error, code:" << ret << endl;
		return BEV_ERROR;
	}
	int read_len = v_in[0].iov_len - pzstream->avail_in;
	int write_len = v_out[0].iov_len - pzstream->avail_out;

	cout << "z stream read:" << read_len << " write:" << write_len << endl;

	evbuffer_drain(src, read_len);

	v_out[0].iov_len = write_len;
	evbuffer_commit_space(dst, v_out, 1);

	return BEV_OK;
}


static void bev_client_event(struct bufferevent *bev, short what, void *ctx)
{
	cout << "in bev_client_event" << endl;
	
	if(what & BEV_EVENT_READING)
	{
		cout << "client event: BEV_EVENT_READING" << endl;
	}

	if(what & BEV_EVENT_WRITING)
	{
		cout << "client event: BEV_EVENT_WRITING" << endl;
	}

	if(what & BEV_EVENT_EOF)
	{
		cout << "client event: BEV_EVENT_EOF" << endl;
		bufferevent_free(bev);
		return;
	}

	if(what & BEV_EVENT_ERROR)
	{
		cout << "client event: BEV_EVENT_ERROR" << endl;
		bufferevent_free(bev);
		return;
	}

	if(what & BEV_EVENT_TIMEOUT)
	{
		cout << "client event: BEV_EVENT_TIMEOUT" << endl;
		bufferevent_free(bev);
		return;
	}

	if(what & BEV_EVENT_CONNECTED)
	{
		cout << "client event: BEV_EVENT_CONNECTED" << endl;

		FILE* fp = fopen("./testdata.txt", "rb");
		if(!fp)
		{
			cout << "open file error!" << endl;
			bufferevent_free(bev);
			return;
		}

		z_stream* pzstream = new z_stream();
		if(!pzstream)
		{
			cout << "new z_stream error" << endl;
			fclose(fp);
			bufferevent_free(bev);
			return;
		}
		deflateInit(pzstream, Z_DEFLATED);

		client_ctx* pctx = new client_ctx;
		if(!pctx)
		{
			cout << "new client_ctx error" << endl;
			fclose(fp);
			delete pzstream;
			bufferevent_free(bev);
			return;
		}
		pctx->fp_read = fp;
		pctx->read_compeleted = 0;
		pctx->read_ok = 0;
		pctx->pout_stream = pzstream;

		bufferevent *pbev_filter = bufferevent_filter_new(bev, NULL, bev_filter_out, BEV_OPT_CLOSE_ON_FREE, NULL, pctx);
		if(!pbev_filter)
		{
			cout << "bufferevent_filter_new error" << endl;
			fclose(fp);
			delete pzstream;
			delete pctx;
			bufferevent_free(bev);
			return;
		}

		bufferevent_setcb(pbev_filter, bev_client_read, bev_client_write, bev_client_event, pctx);

		if(bufferevent_enable(pbev_filter, EV_READ|EV_WRITE) < 0)
		{
			cout << "bufferevent_enable error!" << endl;
			delete pzstream;
			delete pctx;
			bufferevent_free(pbev_filter);
			return;
		}

		bufferevent_write(bev, "testdata.txt", strlen("testdata.txt"));
	}
}

#if 0
int main()
{

	event_base *pbase = event_base_new();
	if(!pbase)
	{
		cout << "event_base_new error!" << endl;
		return -1;
	}


	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5001);
	inet_pton(AF_INET, "localhost", &server_addr.sin_addr);

	bufferevent *pbev = bufferevent_socket_new(pbase, -1, BEV_OPT_CLOSE_ON_FREE);
	if(!pbev)
	{
		cout << "bufferevent_socket_new error!" << endl;
		resource_free(pbase, NULL);
		return -1;
	}

	bufferevent_setcb(pbev, NULL, NULL, bev_client_event, pbev);

	if(bufferevent_enable(pbev, EV_READ|EV_WRITE) < 0)
	{
		cout << "bufferevent_enable error!" << endl;
		resource_free(pbase, pbev);
		return -1;
	}

	if(bufferevent_socket_connect(pbev, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		cout << "bufferevent_socket_connect error!" << endl;
		resource_free(pbase, pbev);
		return -1;
	}

	event_base_dispatch(pbase);


	resource_free(pbase, NULL);

	return 0;
}

#endif

