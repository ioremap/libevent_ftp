#include <stdio.h>
#include <signal.h>

#include <event2/event.h>

#include <string.h>
#include <iostream>
using namespace std;

/*
//使用event_new创建事件时，事件状态为non_pending
//当使用event_add添加时后，事件状态变为pending
//事件发生时，event变为active状态，然后执行事件处理函数
//事件处理完成后，如果创建事件时指定了持久标志(EV_PERSIST),会自动再次添加事件，使事件变为pending状态
//如果创建时没有指定EV_PERSIST标志，处理完事件后状态变为non_pending,也就是说事件只会执行一次
*/


void signalint_callback(evutil_socket_t socket, short flag, void *arg)
{
	cout << "signal_callback!  " << "socket:" << socket << "flag:" << flag << endl;
}

void signalterm_callback(evutil_socket_t socket, short flag, void *arg)
{
	cout << "signal_callback!  " << "socket:" << socket << "flag:" << flag << endl;

	static int cnt = 0;

	cnt++;

	event* ptermail_event = (event*)arg;

	if(0 == event_pending(ptermail_event, EV_SIGNAL, 0))
	{
		if(cnt < 3)
		{	
			event_add(ptermail_event, 0);
		}
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

	event* psignal_event = evsignal_new(pbase, SIGINT, signalint_callback, pbase);
	if(!psignal_event)
	{
		cout << "evsignal_new error!" << endl;
		event_base_free(pbase);
		return -1;
	}
	if(event_add(psignal_event, 0) < 0)
	{
		cout << "event_add error!" << endl;
	}

	
	event* ptermail_event = event_new(pbase, SIGTERM, EV_SIGNAL, signalterm_callback, event_self_cbarg());
	if(!ptermail_event)
	{
		cout << "evsignal_new error!" << endl;
		event_free(psignal_event);
		event_base_free(pbase);
		return -1;
	}
	if(event_add(ptermail_event, 0) < 0)
	{
		cout << "event_add error!" << endl;
	}

	event_base_dispatch(pbase);


	event_free(psignal_event);
	event_free(ptermail_event);

	event_base_free(pbase);

}
#endif



