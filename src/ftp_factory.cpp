#include "ftp_factory.h"
#include "ftp_task.h"
#include "ftp_cmd.h"

ftp_factory::ftp_factory()
{

}

ftp_factory::~ftp_factory()
{

}

thread_task* ftp_factory::create_task()
{
	ftp_server_cmd* pcmd = new ftp_server_cmd();
	if(pcmd)
	{
		ftp_task* pt1 = new ftp_cmd_user();
		if(pt1)
		{
			pcmd->register_cmd("USER", pt1);
		}

		ftp_task* pt2 = new ftp_cmd_port();
		if(pt2)
		{
			pcmd->register_cmd("PORT", pt2);
		}

		ftp_task* pt3 = new ftp_cmd_list();
		if(pt3)
		{
			pcmd->register_cmd("PWD", pt3);
			pcmd->register_cmd("LIST", pt3);
			pcmd->register_cmd("CWD", pt3);
			pcmd->register_cmd("CDUP", pt3);
		}

		ftp_task* pt4 = new ftp_cmd_retr();
		if(pt4)
		{
			pcmd->register_cmd("RETR", pt4);
		}

		ftp_task* pt5 = new ftp_cmd_stor();
		if(pt5)
		{
			pcmd->register_cmd("STOR", pt5);
		}

		return pcmd;
	}

	return NULL;
}
