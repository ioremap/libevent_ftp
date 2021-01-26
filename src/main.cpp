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

#include <errno.h>


#include <string.h>
#include <iostream>
using namespace std;

#include "thread_pool.h"
#include "thread_task.h"
#include "ftp_factory.h"

static void on_accept(evutil_socket_t socket , short event, void *arg)
{
	cout << "---------main-----on_accept-----------------------!" << endl;

	struct sockaddr_in client_addr;
	socklen_t client_addr_len;

	int client_sockfd = accept(socket, (struct sockaddr*)&client_addr, &client_addr_len);
	if(client_sockfd < 0)
	{
		cout << "---------main-----accept error-------ret:" << client_sockfd << "  reson:" << strerror(errno) << endl;
		return;
	}

	thread_pool* ppool = thread_pool::get_pool();
	if(ppool)
	{
		thread_task* ptask = ftp_factory::get()->create_task();
		if(ptask)
		{

			ptask->sockfd = client_sockfd;
			
			ppool->dispatch(ptask);
		}
	}

}

int main()
{
	thread_pool* p = thread_pool::get_pool();
	if(p)
	{
		p->init(4);
	}


	event_base* pbase = event_base_new();
	if(!pbase)
	{
		cout << "event_base_new error!" << endl;
		return -1;
	}

	struct sockaddr_in listen_addr;
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_port = htons(8080);
	inet_pton(AF_INET, "192.168.0.102", &listen_addr.sin_addr);


	int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sockfd < 0)
	{
		cout << "socket error!" << endl;
		event_base_free(pbase);
		return -1;
	}

	int one = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0)
	{
		cout << "setsockopt error!" << endl;
		close(sockfd);
		event_base_free(pbase);
		return -1;
	}

	if(bind(sockfd, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) < 0)
	{
		cout << "bind error!" << endl;
		close(sockfd);
		event_base_free(pbase);
		return -1;
	}

	listen(sockfd, 10);

	event *pevent_accept = event_new(pbase, sockfd, EV_READ | EV_PERSIST, on_accept, pbase);
	if(!pevent_accept)
	{
		cout << "event_new error!" << endl;
		close(sockfd);
		event_base_free(pbase);
		return -1;
	}

	event_add(pevent_accept, 0);


	event_base_dispatch(pbase);

	event_base_free(pbase);
	
	return 0;
}

