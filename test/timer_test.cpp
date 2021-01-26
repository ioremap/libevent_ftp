#include <stdio.h>
#include <signal.h>

#include <event2/event.h>

#include <string.h>
#include <iostream>
using namespace std;


void timer_callback(evutil_socket_t socket, short flag, void *arg)
{
	cout << "timer_callback!  " << "socket:" << socket << "flag:" << flag << endl;

	event* ptimer = (event*)arg;

	timeval t;
	t.tv_sec = 3;
	t.tv_usec = 0;

	if(0 == evtimer_pending(ptimer, &t))
	{
		evtimer_del(ptimer);

		evtimer_add(ptimer, &t);
	}
}

void timer2_callback(evutil_socket_t socket, short flag, void *arg)
{
	cout << "timer2_callback!  " << "socket:" << socket << "flag:" << flag << endl;
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

	event *pev_timer = evtimer_new(pbase, timer_callback, event_self_cbarg());
	if(!pev_timer)
	{
		cout << "evtimer_new error!" << endl;
		event_base_free(pbase);
		return -1;
	}

	timeval t;
	t.tv_sec = 3;
	t.tv_usec = 0;
	evtimer_add(pev_timer, &t);



	event* pev_timer2 = event_new(pbase, -1, EV_PERSIST, timer2_callback, event_self_cbarg());
	if(!pev_timer2)
	{
		cout << "evsignal_new error!" << endl;
		event_free(pev_timer);
		event_base_free(pbase);
		return -1;
	}

	timeval t2;
	t2.tv_sec = 1;
	t2.tv_usec = 0;
	if(event_add(pev_timer2, &t2) < 0)
	{
		cout << "event_add error!" << endl;
	}


	event_base_dispatch(pbase);


	event_free(pev_timer);

	event_free(pev_timer2);

	event_base_free(pbase);

	return 0;

}

#endif

