#include "thread_pool.h"
#include <thread>
#include <chrono>
#include <iostream>

thread_pool::thread_pool()
{
	std::cout << "thread_pool construct" << std::endl;

	thread_count = 0;
	last_thread_id = -1;
}

thread_pool::~thread_pool()
{
	std::cout << "thread_pool destory" << std::endl;

	for(std::vector<work_thread*>::iterator it = pool.begin(); it != pool.end(); it++)
	{
		if(*it != NULL)
		{
			delete *it;
		}
	}
}

int thread_pool::init(int thread_cnt)
{
	if(thread_cnt <= 0)
	{
		std::cout << "thread_pool::init error, invalid thread count" << std::endl;
		return -1;
	}
	
	thread_count = thread_cnt;

	for(int i = 0; i < thread_count; i++)
	{
		work_thread* p = new work_thread(i + 1); 
		if(p)
		{
			p->start();
			
			pool.push_back(p);

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		else
		{
			std::cout << "new work_thread() error!" << std::endl;
		}
	}

	return 0;
}

void thread_pool::dispatch(thread_task* ptask)
{
	if(!ptask)
	{
		return;
	}

	int thread_id = (last_thread_id + 1) % thread_count;
	last_thread_id = thread_id;

	work_thread* pthread = pool[thread_id];
	if(pthread)
	{
		std::cout << "thread_pool::dispatch  acctive thread id:" << thread_id + 1 << std::endl;
		pthread->add_task(ptask);
		pthread->active();
	}
}


