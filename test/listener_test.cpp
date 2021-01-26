#include <stdio.h>

#include <signal.h>
#include <string.h>

#include <event2/event.h>
#include <event2/listener.h>

#include <zlib.h>

#include <openssl/ssl.h> 
#include <openssl/err.h> 

#include <iostream>
using namespace std;

void listen_cb(struct evconnlistener *plistener, evutil_socket_t socket, struct sockaddr *paddr, int socklen, void *pdata)
{

}

#if 0
int main()
{

	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		cout << "ignore signal SIGPIPE error!" << endl;
		return -1;
	}

	event_base * pbase = event_base_new();
	if(NULL == pbase)
	{
		cout << "event_base_new error!" << endl;
		return -1;
	}

	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(8000);


	evconnlistener *plistener = evconnlistener_new_bind(pbase, listen_cb, pbase, 
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 10, (sockaddr *)&addr, sizeof(addr));
	if(NULL == plistener)
	{
		event_base_free(pbase);
		cout << "evconnlistener_new_bind error!" << endl;
		return -1;
	}

	cout << "evconnlistener_new_bind success!" << endl;

	event_base_dispatch(pbase);

	cout << "event_base_dispatch end!" << endl;

	evconnlistener_free(plistener);
	event_base_free(pbase);

	return 0;
}
#endif


