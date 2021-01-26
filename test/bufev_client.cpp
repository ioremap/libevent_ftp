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

#include <string.h>
#include <iostream>
using namespace std;

void client_read_cb(struct bufferevent *bev, void *ctx)
{
	cout << "client_read_cb" << endl;

	char temp[1024] = {0};
	bufferevent_read(bev, temp, sizeof(temp) - 1);
	cout << "recv server data:" << temp << endl;
}

void client_write_cb(struct bufferevent *bev, void *ctx)
{
	cout << "client_write_cb" << endl;

	FILE* fp = (FILE*)ctx;
	if(!fp)
	{
		cout << "fp is null!!!!!" << endl;
		return;
	}

	char temp[1024] = {0};
	int read_len = fread(temp, 1, sizeof(temp)-1, fp);
	if(read_len <= 0)
	{
		cout << "read eof or error, retcode:" << read_len << endl;
		fclose(fp);
		bufferevent_disable(bev, EV_WRITE);
		return;
	}

	cout << "write_len:" << read_len << endl;

	bufferevent_write(bev, temp, read_len);
}

void client_event_cb(struct bufferevent *bev, short what, void *ctx)
{
	cout << "client_event_cb" << endl;

	if(what & BEV_EVENT_READING)
	{
		cout << "event BEV_EVENT_READING" << endl;
	}

	if(what & BEV_EVENT_WRITING)
	{
		cout << "event BEV_EVENT_WRITING" << endl;
	}

	if(what & BEV_EVENT_EOF)
	{
		cout << "event BEV_EVENT_EOF" << endl;
		bufferevent_free(bev);
	}

	if(what & BEV_EVENT_ERROR)
	{
		cout << "event BEV_EVENT_ERROR" << endl;
		bufferevent_free(bev);
		return;
	}

	if(what & BEV_EVENT_TIMEOUT)
	{
		cout << "event BEV_EVENT_TIMEOUT" << endl;
		bufferevent_free(bev);
		return;
	}

	if(what & BEV_EVENT_CONNECTED)
	{
		cout << "event BEV_EVENT_CONNECTED" << endl;
		cout << "connect to server success!!!" << endl;

		bufferevent_trigger(bev, EV_WRITE, 0);
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

	FILE* fp =fopen("./src/timer_test.cpp", "rb");
	if(!fp)
	{
		cout << "open file failed!" << endl;
		event_base_free(pbase);
		return -1;
	}


	bufferevent *pbufev_client = bufferevent_socket_new(pbase, -1, BEV_OPT_CLOSE_ON_FREE);
	if(!pbufev_client)
	{
		cout << "bufferevent_new error!" << endl;
		event_base_free(pbase);
		return -1;
	}

	bufferevent_enable(pbufev_client, EV_READ|EV_WRITE);

	bufferevent_setcb(pbufev_client, client_read_cb, client_write_cb, client_event_cb, fp);


	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5001);
	inet_pton(AF_INET, "localhost", &server_addr.sin_addr);
	

	if(bufferevent_socket_connect(pbufev_client, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		cout << "bufferevent_socket_connect error!" << endl;
	}


	event_base_dispatch(pbase);

	event_base_free(pbase);

}

#endif


