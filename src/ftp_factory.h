#pragma once
#include "thread_task.h"

class ftp_factory
{
public:
	virtual ~ftp_factory();
	
	static ftp_factory* get()
	{
		static ftp_factory s;
		return &s;
	}

	thread_task* create_task();

private:
	ftp_factory();
	ftp_factory(const ftp_factory& f);
	ftp_factory& operator=(const ftp_factory& f);
};

