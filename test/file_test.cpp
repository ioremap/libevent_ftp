#include <stdio.h>
#include <signal.h>

#include <unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#include <event2/event.h>

#include <string.h>
#include <iostream>
using namespace std;


void event_callback(evutil_socket_t socket, short flag, void *arg)
{
	cout << "event_callback!  " << "socket:" << socket << "flag:" << flag << endl;
}

#if 0
int main()
{
	event_config *pconfig = event_config_new();
	if(!pconfig)
	{
		cout << "event_config_new error!" << endl;
		return -1;
	}

	if(event_config_require_features(pconfig, EV_FEATURE_FDS) < 0)
	{
		cout << "event_config_require_features error!" << endl;
		event_config_free(pconfig);
		return -1;
	}

	event_base *pbase = event_base_new_with_config(pconfig);
	if(!pbase)
	{
		cout << "event_base_new_with_config error!" << endl;
		event_config_free(pconfig);
		return -1;
	}


	int fd = open("../test.txt", O_RDONLY | O_NONBLOCK);
	if(fd)
	{
		cout << "open file success!!!" << endl;
	}
	else
	{
		cout << "open file failed!!!" << endl;
	}


	event *pevent = event_new(pbase, fd, EV_READ | EV_PERSIST, event_callback, event_self_cbarg());
	if(!pevent)
	{
		cout << "event_new error!" << endl;	
	}
	else
	{
		event_add(pevent, 0);
	}
	

	event_base_dispatch(pbase);

	if(fd > 0)
	{
		close(fd);
	}
	
	event_config_free(pconfig);

	event_base_free(pbase);

}


#endif


