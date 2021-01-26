#include "ftp_task.h"
#include <iostream>
#include <string.h>
#include <string>
#include <arpa/inet.h>

ftp_task::ftp_task():thread_task()
{
	std::cout << "ftp_task::ftp_task" << std::endl;

	root_dir = "/home/share/";
	current_dir = "/home/share/libevent_ftp/libevent_ftp/Makefile";
	pcmd_task = NULL;
	m_pbev = NULL;
	client_port = 0;
}

ftp_task::~ftp_task()
{
	std::cout << "ftp_task::~ftp_task" << std::endl;
}

int ftp_task::init()
{
	std::cout << "ftp_task::init" << std::endl;
	m_pbev = NULL;

	return 0;
}

void ftp_task::on_read(struct bufferevent * pbev)
{

}

void ftp_task::on_write(struct bufferevent * pbev)
{

}

void ftp_task::on_event(struct bufferevent * pbev, short what)
{

}

void ftp_task::parse_cmd(std::string type, std::string msg)
{

}

void ftp_task::send_data_to_client(std::string msg)
{
	if(!m_pbev)
	{
		return;
	}

	bufferevent_write(m_pbev, msg.c_str(), msg.size());
}

void ftp_task::send_data_to_client(const char* data, int len)
{
	if(!m_pbev)
	{
		return;
	}

	bufferevent_write(m_pbev, data, len);
}

void ftp_task::respon_cmd(std::string msg)
{
	if(!pcmd_task->m_pbev)
	{
		return;
	}

	bufferevent_write(pcmd_task->m_pbev, msg.c_str(), msg.size());
}

void ftp_task::connect_to_client()
{
	if(client_ip.empty() || client_port <= 0)
	{
		std::cout << "ftp_task::connect_to_client error, invalid ip or port" << std::endl;
		return;
	}

	close_bufferevent();

	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(client_port);
	inet_pton(AF_INET, client_ip.c_str(), &client_addr.sin_addr);

	bufferevent *pbev = bufferevent_socket_new(pevent_base, -1, BEV_OPT_CLOSE_ON_FREE);
	if(!pbev)
	{
		std::cout << "ftp_task::connect_to_client bufferevent_socket_new error" << std::endl;
		return;
	}

	m_pbev = pbev;

	set_callback(pbev);

	struct timeval read_timeout = {10, 0};
	bufferevent_set_timeouts(pbev, &read_timeout, NULL);

	if(bufferevent_socket_connect(pbev, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0)
	{
		std::cout << "ftp_task::connect_to_client connect error" << std::endl;
		bufferevent_free(pbev);
		return;
	}

	std::cout << "ftp_task::connect_to_client success" << std::endl;
}

void ftp_task::close_bufferevent()
{
	if(m_pbev)
	{
		bufferevent_free(m_pbev);
		m_pbev = NULL;
	}
}

void ftp_task::set_callback(struct bufferevent* pbev)
{
	bufferevent_setcb(pbev, read_callback, write_callback, event_callback, this);

	bufferevent_enable(pbev, EV_READ|EV_WRITE);
}

void ftp_task::read_callback(struct bufferevent* bev, void* arg)
{
	ftp_task* ptask = (ftp_task*)arg;
	ptask->on_read(bev);
}

void ftp_task::write_callback(struct bufferevent* bev, void* arg)
{
	ftp_task* ptask = (ftp_task*)arg;
	ptask->on_write(bev);
}

void ftp_task::event_callback(struct bufferevent* bev, short what, void* arg)
{
	ftp_task* ptask = (ftp_task*)arg;
	ptask->on_event(bev, what);
}





ftp_server_cmd::ftp_server_cmd():ftp_task()
{

}

ftp_server_cmd::~ftp_server_cmd()
{
	bufferevent_free(m_pbev);

	for(std::map<ftp_task*, int>::iterator it = cmds_delete_map.begin(); it != cmds_delete_map.end(); it++)
	{
		ftp_task* p = it->first;
		p->close_bufferevent();
		delete p;
	}
}

int ftp_server_cmd::init()
{
	std::cout << "ftp_server_cmd::init" << std::endl;
	
	if(!pevent_base)
	{
		std::cout << "ftp_server_cmd::init error, pevent_base be NULL" << std::endl;
		return -1;
	}
	
	if(sockfd < 0)
	{
		std::cout << "ftp_server_cmd::init error, sockfd error" << std::endl;
		return -1;
	}
	
	bufferevent *pbev = bufferevent_socket_new(pevent_base, sockfd, BEV_OPT_CLOSE_ON_FREE);
	if(!pbev)
	{
		std::cout << "ftp_server_cmd::init error, bufferevent_socket_new error" << std::endl;
		delete this;
		return -1;
	}

	m_pbev = pbev;

	set_callback(pbev);

	struct timeval read_timeout = {300, 0};
	bufferevent_set_timeouts(pbev, &read_timeout, NULL);
	
	std::cout << "ftp_server_cmd::init success pevent_base:" << pevent_base << " sockfd:" << sockfd << " pbev:" << pbev << std::endl;


	std::string welcome = "220 welcome to my ftp server\r\n";
	bufferevent_write(pbev, welcome.c_str(), welcome.size());

	return 0;
}

void ftp_server_cmd::on_read(struct bufferevent * pbev)
{
	std::cout << "ftp_server_cmd::on_read  in thread:" << thread_id << std::endl;

	char temp[1024] = {0};
	int len = -1;
	std::string type = "";
	int find_type = 0;
	while((len = bufferevent_read(pbev, temp, len)) > 0)
	{
		std::cout << "read_data:" << temp << std::endl;

		if(find_type == 0)
		{
			for(int i = 0; i < len; i++)
			{
				if((temp[i] == ' ') || (temp[i] == '\r'))
				{
					find_type = 1;
					break;
				}
				type += temp[i];
			}
		}		
	}

	if(find_type)
	{
		std::cout << "type:" << type << std::endl;
		if(cmds_map.find(type) == cmds_map.end())
		{
			std::string response = "200 OK\r\n";
			bufferevent_write(pbev, response.c_str(), response.size());
			return;
		}
		
		ftp_task* ptask = cmds_map[type];
		if(!ptask)
		{
			std::string response = "200 OK\r\n";
			bufferevent_write(pbev, response.c_str(), response.size());
			return;
		}

		ptask->pcmd_task = this;
		ptask->pevent_base = pevent_base;

		if(type != "PORT")
		{
			ptask->client_ip = client_ip;
			ptask->client_port = client_port;
		}		

		ptask->parse_cmd(type, temp);

		if(type == "PORT")
		{
			client_ip = ptask->client_ip;
			client_port = ptask->client_port;
		}
	}
}

void ftp_server_cmd::on_write(struct bufferevent * pbev)
{
	std::cout << "ftp_server_cmd::on_write in thread:" << thread_id << std::endl;
}

void ftp_server_cmd::on_event(struct bufferevent * pbev, short what)
{
	std::cout << "ftp_server_cmd::on_event  in thread:" << thread_id << std::endl;

	if(what & BEV_EVENT_EOF)
	{
		std::cout << "BEV_EVENT_EOF" << std::endl;
		//bufferevent_free(pbev);
		delete this;
		return;
	}

	if(what & BEV_EVENT_ERROR)
	{
		std::cout << "BEV_EVENT_ERROR" << std::endl;
		//bufferevent_free(pbev);
		delete this;
		return;
	}

	if(what & BEV_EVENT_TIMEOUT)
	{
		std::cout << "BEV_EVENT_TIMEOUT" << std::endl;
		//bufferevent_free(pbev);
		delete this;
		return;
	}
}

void ftp_server_cmd::register_cmd(std::string cmd, ftp_task* ptask)
{
	if(!ptask)
	{
		std::cout << "ftp_server_cmd::register_cmd error, ptask be NULL" << std::endl;
		return;
	}

	if(cmds_map.find(cmd) != cmds_map.end())
	{
		std::cout << "ftp_server_cmd::register_cmd error, cmd registed" << std::endl;
		return;
	}

	cmds_map[cmd] = ptask;
	cmds_delete_map[ptask] = 1;
}

