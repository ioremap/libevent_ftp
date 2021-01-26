#pragma once

#include <event2/event.h>
#include <list>
#include <mutex>
#include "thread_task.h"

class work_thread
{
public:
	work_thread(int index);
	virtual ~work_thread();
	void start();
	int set_up();
	void active();
	void main_loop();
	void add_task(thread_task* t);
	static void pipe_read_callback(evutil_socket_t socket, short event, void* arg);
	void on_pipe_read(evutil_socket_t socket, short event);
private:
	work_thread(work_thread& p);
	work_thread& operator=(work_thread& p);

	int m_index;
	int m_pipe_send_fd;
	event_base* m_event_base;
	std::list<thread_task*> task_list;
	std::mutex task_list_mutex;
};
