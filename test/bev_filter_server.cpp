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
	int started;
	FILE* fp_out;
	string file_name;
	z_stream* pin_stream;
}server_recv_ctx;

#if 0
static void resource_free(event_base *pbase, bufferevent *pbev, int fd)
{
	if(pbase)
	{
		event_base_free(pbase);
	}

	if(pbev)
	{
		bufferevent_free(pbev);
	}

	if(fd > 0)
	{
		close(fd);
	}
}
#endif


static void bev_read(struct bufferevent *bev, void *ctx)
{
	cout << "in bev_read" << endl;

	server_recv_ctx* pctx = (server_recv_ctx*)ctx;

	if(0 == pctx->started)
	{
		pctx->started = 1;
		
		char filename[64] = {0};
		bufferevent_read(bev, filename, sizeof(filename) - 1);
		
		pctx->file_name = filename;
		pctx->fp_out = fopen(pctx->file_name.c_str(), "wb");
		if(!pctx->fp_out)
		{
			cout << "open file error!!!" << endl;
			bufferevent_free(bev);
			return;
		}

		cout << "filename:" << pctx->file_name << "  echo OK to client!" << endl << endl;
		
		bufferevent_write(bev, "OK", strlen("OK"));
		return;
	}

	while(evbuffer_get_length(bufferevent_get_input(bev)) > 0)
	{
		char temp[512] = {0};
		int read_len = bufferevent_read(bev, temp, sizeof(temp));
		cout << "read_len:" << read_len << endl;
		//cout << "read_data:" << temp << endl;

		if(read_len > 0)
		{
			int ret = fwrite(temp, 1, read_len, pctx->fp_out);
			cout << "fwrite retcode:" << ret << endl;
			fflush(pctx->fp_out);
		}
	}

	cout << "read compeleted!!!" << endl << endl;

}

static void bev_write(struct bufferevent *bev, void *ctx)
{
	cout << "in bev_write" << endl;
}

static void bev_event(struct bufferevent *bev, short what, void *ctx)
{
	cout << "in bev_event" << endl;

	server_recv_ctx* pctx = (server_recv_ctx*)ctx;
	
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
		if(pctx->fp_out)
		{
			fclose(pctx->fp_out);
			pctx->fp_out = NULL;
		}
		delete pctx->pin_stream;
		delete pctx;
		
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
	}
}

static bufferevent_filter_result bev_filter_in(struct evbuffer *src, struct evbuffer *dst, ev_ssize_t dst_limit,
    										  enum bufferevent_flush_mode mode, void *ctx)
{
	cout << "bev_filter_in" << endl;
	
	server_recv_ctx* pctx = (server_recv_ctx*)ctx;
	
	if(0 == pctx->started)
	{
		char temp[1024] = {0};
		int len = evbuffer_remove(src, temp, sizeof(temp));
		
		cout << "filter read raw data, len:" << len << endl;
		
		if(evbuffer_add(dst, temp, len) < 0)
		{
			cout << "evbuffer_add error!!!" << endl;
			return BEV_ERROR;
		}
		return BEV_OK;
	}

	#if 1
	evbuffer_iovec v_in[1];
	int len = evbuffer_peek(src, -1, NULL, v_in, 1);
	if(len <= 0)
	{
		cout << "evbuffer_peek error, len:" << len << endl;
		return BEV_NEED_MORE;
	}

	cout << "evbuffer_peek ret:" << len << endl;
	cout << "v_in[0].iov_len" << v_in[0].iov_len << endl;

	z_stream* pzstream = pctx->pin_stream;
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

	int ret = inflate(pzstream, Z_SYNC_FLUSH);
	if(ret != Z_OK)
	{
		cout << "inflate error, code:" << ret << endl;
		return BEV_ERROR;
	}

	int read_len = v_in[0].iov_len - pzstream->avail_in;
	int write_len = v_out[0].iov_len - pzstream->avail_out;

	cout << "z stream read:" << read_len << " write:" << write_len << endl;

	evbuffer_drain(src, read_len);

	v_out[0].iov_len = write_len;
	evbuffer_commit_space(dst, v_out, 1);

	return BEV_OK;
	#endif
}

void bev_listen_callback(evutil_socket_t socket, short what, void *arg)
{
	cout << "in bev_listen_callback" << endl;

	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	int client_fd = accept(socket, (struct sockaddr*)&client_addr, &client_addr_len);
	if(client_fd < 0)
	{
		cout << "accept error" << endl;
		return;
	}

	event_base* pbase = (event_base*)arg;

	bufferevent *pbev = bufferevent_socket_new(pbase, client_fd, BEV_OPT_CLOSE_ON_FREE);
	if(!pbev)
	{
		cout << "bufferevent_socket_new error" << endl;
		close(client_fd);
		return;
	}

	z_stream* pstream = new z_stream();
	if(!pstream)
	{
		cout << "new z_stream error" << endl;
		bufferevent_free(pbev);
		return;
	}
	inflateInit(pstream);

	server_recv_ctx* pctx = new server_recv_ctx;
	if(!pctx)
	{
		cout << "new server_recv_ctx error" << endl;
		delete pstream;
		bufferevent_free(pbev);
		return;
	}
	pctx->started = 0;
	pctx->fp_out = NULL;
	pctx->pin_stream = pstream;

	bufferevent *pbev_filter = bufferevent_filter_new(pbev, bev_filter_in, NULL, BEV_OPT_CLOSE_ON_FREE, NULL, pctx);
	if(!pbev_filter)
	{
		cout << "bufferevent_filter_new error" << endl;
		delete pstream;
		delete pctx;
		bufferevent_free(pbev);
		return;
	}

	bufferevent_setcb(pbev_filter, bev_read, bev_write, bev_event, pctx);

	if(bufferevent_enable(pbev_filter, EV_READ|EV_WRITE) < 0)
	{
		cout << "bufferevent_enable error!" << endl;
		delete pstream;
		delete pctx;
		bufferevent_free(pbev);
		return;
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

	int listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(listen_fd < 0)
	{
		cout << "socket create error!" << endl;
		resource_free(pbase, NULL, 0);
		return -1;
	}

	evutil_make_listen_socket_reuseable(listen_fd);
	evutil_make_socket_nonblocking(listen_fd);

	if(bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		cout << "bind error!" << endl;
		resource_free(pbase, NULL, 0);
		return -1;
	}

	listen(listen_fd, 10);

	event *pevent = event_new(pbase, listen_fd, EV_READ|EV_PERSIST, bev_listen_callback, pbase);
	if(!pevent)
	{
		cout << "event_new error!" << endl;
		resource_free(pbase, NULL, listen_fd);
		return -1;	
	}

	event_add(pevent, 0);

	event_base_dispatch(pbase);

	event_free(pevent);
	event_base_free(pbase);
	
	return 0;

}

#endif

