#include <stdio.h>

#include <signal.h>
#include <string.h>

#include <event2/event.h>
#include <event2/listener.h>

#include <iostream>
using namespace std;


#if 0

int main()
{
	const char **psupported_methods = event_get_supported_methods();
	if(psupported_methods)
	{
		for (int i = 0; psupported_methods[i] != NULL; i++)
		{
			cout << psupported_methods[i] << endl;
		}
	}

	
	event_config *pconfig = event_config_new();
	if(NULL == pconfig)
	{
		cout << "event_config_new error!" << endl;
		return -1;
	}

	event_config_require_features(pconfig, EV_FEATURE_ET|EV_FEATURE_O1);

	event_config_avoid_method(pconfig, "poll"); /*排除掉事件模型*/


	event_base *pbase = event_base_new_with_config(pconfig);
	if(NULL == pbase)
	{
		event_config_free(pconfig);
		cout << "event_base_new_with_config error!" << endl;
		return -1;
	}
	else
	{
		event_config_free(pconfig);
	}

	cout << "current method:" << event_base_get_method(pbase) << endl;


	int features = event_base_get_features(pbase);
	if(features & EV_FEATURE_ET)
	{
		cout << "support EV_FEATURE_ET" << endl;
	}
	if(features & EV_FEATURE_O1)
	{
		cout << "support EV_FEATURE_O1" << endl;
	}
	if(features & EV_FEATURE_FDS)
	{
		cout << "support EV_FEATURE_FDS" << endl;
	}
	if(features & EV_FEATURE_EARLY_CLOSE)
	{
		cout << "support EV_FEATURE_EARLY_CLOSE" << endl;
	}

	event_base_free(pbase);

}

#endif



