#pragma once

#include <event2/event.h>

class thread_task
{
public:
	thread_task();
	virtual ~thread_task();
	virtual int init() = 0;

	event_base* pevent_base;
	int sockfd;
	int thread_id;
private:
	thread_task(const thread_task& task);
	thread_task& operator=(const thread_task& task);
};

