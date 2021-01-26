#include "work_thread.h"
#include <unistd.h>
#include <thread>
#include <iostream>
#include <string>

work_thread::work_thread(int index)
{
	std::cout << "work_thread construct" << std::endl;

	m_index = index;
	m_pipe_send_fd = 0;
	m_event_base = NULL;
}

work_thread::~work_thread()
{
	std::cout << "work_thread destory" << std::endl;
}

void work_thread::start()
{
	set_up();
	
	std::thread t(&work_thread::main_loop, this);

	t.detach();
}

int work_thread::set_up()
{
	int fds[2];
	if(pipe(fds) != 0)
	{
		std::cout << "work_thread set_up error, pipe error" << std::endl;
		return -1;
	}

	m_pipe_send_fd = fds[1];

	event_config *pevent_config = event_config_new();
	if(!pevent_config)
	{
		std::cout << "work_thread set_up event_config_new error" << std::endl;
		return -1;
	}
	event_config_set_flag(pevent_config, EVENT_BASE_FLAG_NOLOCK);

	m_event_base = event_base_new_with_config(pevent_config);
	if(!m_event_base)
	{
		event_config_free(pevent_config);
		std::cout << "work_thread set_up event_base_new error" << std::endl;
		return -1;
	}
	event_config_free(pevent_config);

	event *pevent_pipe_read = event_new(m_event_base, fds[0], EV_READ|EV_PERSIST, pipe_read_callback, this);
	if(!pevent_pipe_read)
	{
		std::cout << "event_new error" << std::endl;
	}
	event_add(pevent_pipe_read, 0);

	return 0;
}

void work_thread::active()
{
	std::cout << "work_thread::active thread_id:" << m_index << std::endl;
	
	if(0 != m_pipe_send_fd)
	{
		write(m_pipe_send_fd, "S", 1);
	}	
}

void work_thread::add_task(thread_task* t)
{
	if(t)
	{
		t->pevent_base = m_event_base;
		t->thread_id = m_index;

		task_list_mutex.lock();
		task_list.push_back(t);
		task_list_mutex.unlock();

		std::cout << "work_thread::add_task thread_id:" << m_index << " m_event_base:" << m_event_base << std::endl;
	}
}

void work_thread::main_loop()
{
	std::cout << "work_thread::main_loop thread_id:" << m_index << std::endl;

	event_base_dispatch(m_event_base);

	event_base_free(m_event_base);
}

void work_thread::on_pipe_read(evutil_socket_t socket, short event)
{
	char buf[2] = {0};
	int len = read(socket, buf, sizeof(buf));
	if(len <= 0)
	{
		std::cout << "recv error, ret:" << len << std::endl;
		return;
	}

	thread_task* ptask = NULL;
	
	task_list_mutex.lock();
	if(!task_list.empty())
	{
		ptask = task_list.front();
		task_list.pop_front();
	}
	task_list_mutex.unlock();

	if(ptask)
	{
		std::cout <<"work_thread::on_pipe_read init a work task  thread_id:" << m_index << std::endl;
		ptask->init();
	}
}

void work_thread::pipe_read_callback(evutil_socket_t socket, short event, void* arg)
{
	work_thread* pthis_thread = (work_thread*)arg;
	if(pthis_thread)
	{
		pthis_thread->on_pipe_read(socket, event);
	}
}


