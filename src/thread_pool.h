#pragma once

#include <vector>
#include "work_thread.h"
#include "thread_task.h"

class work_thread;
class thread_task;

class thread_pool
{
public:
	virtual ~thread_pool();

	static thread_pool* get_pool()
	{
		static thread_pool p;
		return &p;
	}

	int init(int thread_count = 4);
	void dispatch(thread_task* ptask);

private:
	thread_pool();
	thread_pool(const thread_pool& pool);
	thread_pool& operator=(const thread_pool& pool);

	int thread_count;
	int last_thread_id;
	std::vector<work_thread*> pool;
};

