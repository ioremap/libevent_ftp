#include "ftp_cmd.h"
#include <iostream>
#include <errno.h>
#include <string.h>

void ftp_cmd_user::parse_cmd(std::string type, std::string msg)
{
	std::cout << "ftp_cmd_user::parse_cmd  type:" << type << " msg:" << msg << std::endl;

	std::string res = "230 Login successful.\r\n";
	respon_cmd(res);
}




void ftp_cmd_list::parse_cmd(std :: string type, std :: string msg)
{
	std::cout << "ftp_cmd_list::parse_cmd  type:" << type << " msg:" << msg << std::endl;

	if(type == "PWD")
	{
		std::string respon = "257 \"" + pcmd_task->current_dir + "\" is current dir.\r\n";
		respon_cmd(respon);
	}
	else if(type == "LIST")
	{
		connect_to_client();

		std::cout << "send 150 on cmd port" << std::endl;

		respon_cmd("150 Here comes the directory listing.\r\n");

		std::cout << "send data on data port" << std::endl;

		std::string data = get_file_list(pcmd_task->current_dir);
		send_data_to_client(data.c_str(), data.size());
	}
	else if(type == "CWD")
	{
		int pos = msg.rfind(" ") + 1;
		std::string path = msg.substr(pos, msg.size() - pos - 2);

		if(path[path.size() - 1] != '/')
		{
			path += '/';
		}

		pcmd_task->current_dir = path;

		std::cout << "new path:" << path << std::endl;
		respon_cmd("250 Directory success change.\r\n");
	}
	else if(type =="CDUP")
	{
		std::string path = pcmd_task->current_dir;
		if(path[path.size() - 1] == '/')
		{
			path = path.substr(0, path.size() - 1);
		}

		int pos = msg.rfind('/');
		path = path.substr(0, pos);

		pcmd_task->current_dir = path;

		std::cout << "new path:" << path << std::endl;
		respon_cmd("250 Directory success change.\r\n");
	}
}

void ftp_cmd_list::on_write(struct bufferevent * pbev)
{
	std::cout << "ftp_cmd_list::on_write" << std::endl;

	std::cout << "data send compeleted, send 226 on cmd port" << std::endl;

	respon_cmd("226 Transfer compelete.\r\n");

	close_bufferevent();
}

void ftp_cmd_list::on_event(struct bufferevent* pbev, short what)
{
	std::cout << "ftp_cmd_list::on_event" << std::endl;

	if(what & BEV_EVENT_CONNECTED)
	{
		std::cout << "event BEV_EVENT_CONNECTED" << std::endl;
		return;
	}

	if((what & BEV_EVENT_ERROR) || (what & BEV_EVENT_EOF) || (what & BEV_EVENT_TIMEOUT))
	{
		std::cout << "event:" << what << " ERR:" << BEV_EVENT_ERROR << " EOF:" << BEV_EVENT_EOF << "TIMEOUT:" << BEV_EVENT_TIMEOUT  << std::endl;
		close_bufferevent();
		return;
	}
}

std::string ftp_cmd_list::get_file_list(std::string path)
{
	return path;
}




void ftp_cmd_port::parse_cmd(std :: string type, std :: string msg)
{
	std::cout << "ftp_cmd_port::parse_cmd  type:" << type << " msg:" << msg << std::endl;

	int num1, num2, num3, num4, num5, num6;
	char temp[12] = {0};
	sscanf(msg.c_str(), "%s %d,%d,%d,%d,%d,%d", temp, &num1, &num2, &num3, &num4, &num5, &num6);

	client_ip = std::to_string(num1) + "." + std::to_string(num2) + "." + std::to_string(num3) + "." + std::to_string(num4);
	client_port = num5*256 + num6;

	std::cout << "client ip:" << client_ip << "  client port:" << client_port << std::endl;

	std::string respon = "200 OK\r\n";
	respon_cmd(respon);

}




ftp_cmd_retr::ftp_cmd_retr()
{
	fp = NULL;
}

ftp_cmd_retr::~ftp_cmd_retr()
{
	if(fp)
	{
		fclose(fp);
	}
}

void ftp_cmd_retr::parse_cmd(std :: string type, std :: string msg)
{
	std::cout << "ftp_cmd_retr::parse_cmd  type:" << type << " msg:" << msg << std::endl;

	int pos = msg.rfind(' ');
	std::string file = msg.substr(pos + 1, msg.size() - (pos + 1) - 2);
	std::cout << "file name:" << file << std::endl;

	fp = fopen(file.c_str(), "rb");
	if(!fp)
	{
		std::cout << "open file error!   reson:" << strerror(errno) << std::endl;
		respon_cmd("450 file open failed!\r\n");
		return;
	}

	connect_to_client();

	respon_cmd("150 file OK!\r\n");

	bufferevent_trigger(m_pbev, EV_WRITE, 0);
}

void ftp_cmd_retr::on_write(struct bufferevent* pbev)
{
	std::cout << "ftp_cmd_retr::on_write" << std::endl;

	if(!fp)
	{
		if(m_pbev)
		{
			close_bufferevent();
		}
		return;
	}

	char buf[512] = {0};
	int len = 0;
	while((len = fread(buf, 1, sizeof(buf), fp)) > 0)
	{
		std::cout << "read_len:" << len << std::endl;
		send_data_to_client(buf, len);
	}

	fclose(fp);
	fp = NULL;

	respon_cmd("226 Transfer compelete!\r\n");
}

void ftp_cmd_retr::on_event(struct bufferevent* pbev, short what)
{
	std::cout << "ftp_cmd_retr::on_event" << std::endl;

	if(what & BEV_EVENT_CONNECTED)
	{
		std::cout << "event BEV_EVENT_CONNECTED" << std::endl;
		return;
	}

	if((what & BEV_EVENT_ERROR) || (what & BEV_EVENT_EOF) || (what & BEV_EVENT_TIMEOUT))
	{
		std::cout << "event:" << what << " ERR:" << BEV_EVENT_ERROR << " EOF:" << BEV_EVENT_EOF << "TIMEOUT:" << BEV_EVENT_TIMEOUT  << std::endl;
		close_bufferevent();

		if(fp)
		{
			fclose(fp);
			fp = NULL;
		}
		
		return;
	}
}




ftp_cmd_stor::ftp_cmd_stor()
{
	fp = NULL;
}

ftp_cmd_stor::~ftp_cmd_stor()
{
	if(fp)
	{
		fclose(fp);
	}
}

void ftp_cmd_stor::parse_cmd(std :: string type, std :: string msg)
{
	std::cout << "ftp_cmd_stor::parse_cmd  type:" << type << " msg:" << msg << std::endl;

	int pos = msg.rfind(' ');
	std::string file = msg.substr(pos + 1, msg.size() - (pos + 1) - 2);
	std::cout << "file name:" << file << std::endl;

	fp = fopen(file.c_str(), "wb");
	if(!fp)
	{
		std::cout << "open file error!   reson:" << strerror(errno) << std::endl;
		respon_cmd("450 file open failed!\r\n");
		return;	
	}

	connect_to_client();

	respon_cmd("125 file OK!\r\n");

	bufferevent_trigger(m_pbev, EV_READ, 0);
}

void ftp_cmd_stor::on_read(struct bufferevent* pbev)
{
	std::cout << "ftp_cmd_stor::on_read" << std::endl;

	if(!fp)
	{
		return;
	}
	
	char buf[512] = {0};
	int len = 0;
	while((len = bufferevent_read(m_pbev, buf, sizeof(buf))) > 0)
	{
		std::cout << "read_len:" << len << std::endl;

		int write_len = fwrite(buf, 1, len, fp);

		std::cout << "write_len:" << write_len << std::endl;
	}
}

void ftp_cmd_stor::on_event(struct bufferevent* pbev, short what)
{
	std::cout << "ftp_cmd_stor::on_event" << std::endl;

	if(what & BEV_EVENT_CONNECTED)
	{
		std::cout << "event BEV_EVENT_CONNECTED" << std::endl;
		return;
	}

	if((what & BEV_EVENT_ERROR) || (what & BEV_EVENT_EOF) || (what & BEV_EVENT_TIMEOUT))
	{
		std::cout << "event:" << what << " ERR:" << BEV_EVENT_ERROR << " EOF:" << BEV_EVENT_EOF << "TIMEOUT:" << BEV_EVENT_TIMEOUT  << std::endl;

		respon_cmd("226 Transfer compelete!\r\n");

		close_bufferevent();

		if(fp)
		{
			fclose(fp);
			fp = NULL;
		}
		return;
	}
}

