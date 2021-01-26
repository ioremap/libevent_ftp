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

#include <string.h>
#include <iostream>
using namespace std;

void read_callback(evutil_socket_t socket, short flag, void *arg)
{
	cout << "read_callback!  " << "socket:" << socket << "flag:" << flag << endl;

	event* pevent = (event*)arg;

	if(flag & EV_TIMEOUT)
	{
		cout << "read time out!!!" << endl;

		evutil_closesocket(socket);
		event_free(pevent);

		return;
	}

	char read_buf[1024] = {0};

	int read_len = recv(socket, read_buf, sizeof(read_buf) - 1, 0);
	if(read_len <= 0)
	{
		cout << "read error, ret:" << read_len << endl;

		evutil_closesocket(socket);
		event_free(pevent);
	}
	else
	{
		cout << "read len:" << read_len << endl;
		cout << read_buf << endl;

		char send_buf[128] = {0};
		snprintf(send_buf, sizeof(send_buf), "%s", "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT");

		send(socket, send_buf, strlen(send_buf), 0);
	}
}


void listen_callback(evutil_socket_t socket, short flag, void *arg)
{
	cout << "accept_callback!  " << "socket:" << socket << "flag:" << flag << endl;

	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);

	evutil_socket_t client_connect = accept(socket, (struct sockaddr*)&client_addr, &addr_len);
	if(client_connect < 0)
	{
		cout << "accept error!!!" << endl;
	}
	else
	{
		char client_addr_str[32] = {0};
		inet_ntop(AF_INET, &client_addr.sin_addr, client_addr_str, sizeof(client_addr_str));

		cout << "accept client connect, ip:" << client_addr_str << endl;
	}

	event_base* pbase = (event_base*)arg;

	event *pevent_read = event_new(pbase, client_connect, EV_READ | EV_PERSIST, read_callback, event_self_cbarg());
	if(pevent_read)
	{
		timeval timeout;
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		event_add(pevent_read, &timeout);
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


	event *pevent_socket = event_new(pbase, socket_fd, EV_READ | EV_PERSIST, listen_callback, pbase);
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

