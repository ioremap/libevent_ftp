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

void bufev_read_cb(struct bufferevent *bev, void *ctx)
{
	cout << "------in bufev_read_cb------" << endl;

	char temp[1024] = {0};

	bufferevent_read(bev, temp, sizeof(temp) - 1);

	cout << "recv client data:" << temp << endl;

	if(strstr(temp, "quit") != NULL)
	{
		bufferevent_free(bev);
		return;
	}

	char respon[1024] = {0};
	snprintf(respon, sizeof(respon), "I recv %lu data.\n", strlen(temp));

	bufferevent_write(bev, respon, strlen(respon));
}

void bufev_write_cb(struct bufferevent *bev, void *ctx)
{
	cout << "------in bufev_write_cb------" << endl;
}

void bufev_event_cb(struct bufferevent *bev, short what, void *ctx)
{
	cout << "------in bufev_event_cb------" << endl;

	if(what & BEV_EVENT_READING)
	{
		cout << "event: BEV_EVENT_READING" << endl;
	}
	
	if(what & BEV_EVENT_WRITING)
	{
		cout << "event: BEV_EVENT_WRITING" << endl;
	}
	
	if(what & BEV_EVENT_EOF)
	{
		cout << "event: BEV_EVENT_EOF" << endl;
	}
	
	if(what & BEV_EVENT_ERROR)
	{
		cout << "event: BEV_EVENT_ERROR" << endl;

		bufferevent_free(bev);
	}
	
	if(what & BEV_EVENT_TIMEOUT)
	{
		cout << "event: BEV_EVENT_TIMEOUT" << endl;

		bufferevent_free(bev);
	}
	
	if(what & BEV_EVENT_CONNECTED)
	{
		cout << "event: BEV_EVENT_CONNECTED" << endl;
	}
}


void buffer_listen_callback(evutil_socket_t socket, short flag, void *arg)
{
	cout << "accept_callback!  " << "socket:" << socket << "flag:" << flag << endl;

	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);

	evutil_socket_t client_connect = accept(socket, (struct sockaddr*)&client_addr, &addr_len);
	if(client_connect < 0)
	{
		cout << "accept error!!!" << endl;
		return;
	}
	else
	{
		char client_addr_str[32] = {0};
		inet_ntop(AF_INET, &client_addr.sin_addr, client_addr_str, sizeof(client_addr_str));

		cout << "accept client connect, ip:" << client_addr_str << endl;
	}

	event_base* pbase = (event_base*)arg;

	bufferevent *pbufferevent = bufferevent_socket_new(pbase, client_connect, BEV_OPT_CLOSE_ON_FREE);
	if(!pbufferevent)
	{
		cout << "bufferevent_socket_new error!" << endl;
	}
	else
	{
		cout << "bufferevent_socket_new success!" << endl;
	}

	if(bufferevent_enable(pbufferevent, EV_READ|EV_WRITE) < 0)
	{
		cout << "bufferevent_enable error!" << endl;
	}

	bufferevent_setwatermark(pbufferevent, EV_READ, 4, 10);

	bufferevent_setcb(pbufferevent, bufev_read_cb, bufev_write_cb, bufev_event_cb, pbase);
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

	evutil_socket_t socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(socket_fd < 0)
	{
		cout << "socket error!  :" << strerror(errno) << endl;	
		event_base_free(pbase);
		return -1;
	}
	evutil_make_listen_socket_reuseable(socket_fd);
	evutil_make_socket_nonblocking(socket_fd);

	struct sockaddr_in listen_addr;
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(5001);
	inet_pton(AF_INET, "localhost", &listen_addr.sin_addr);

	if(bind(socket_fd, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) < 0)
	{
		cout << "bind error!  :" << strerror(errno) << endl;
		event_base_free(pbase);
		close(socket_fd);
		return -1;
	}

	listen(socket_fd, 10);


	event *pevent_socket = event_new(pbase, socket_fd, EV_READ | EV_PERSIST, buffer_listen_callback, pbase);
	if(!pevent_socket)
	{
		cout << "event_new error!" << endl;
		close(socket_fd);
		event_base_free(pbase);
		return -1;
	}
	event_add(pevent_socket, 0);


	event_base_dispatch(pbase);

	event_free(pevent_socket);

	event_base_free(pbase);

	close(socket_fd);

	return 0;

}

#endif

