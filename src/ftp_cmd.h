#pragma once

#include "ftp_task.h"

class ftp_cmd_user : public ftp_task
{
public:
	ftp_cmd_user()
	{
	}
	~ftp_cmd_user()
	{
	}
	void parse_cmd(std::string type, std::string msg);
};


class ftp_cmd_list : public ftp_task
{
public:
	ftp_cmd_list()
	{

	}
	~ftp_cmd_list()
	{

	}
	void parse_cmd(std::string type, std::string msg);
	void on_write(struct bufferevent* pbev);
	void on_event(struct bufferevent* pbev, short what);
private:
	std::string get_file_list(std::string path);
};

class ftp_cmd_port : public ftp_task
{
public:
	ftp_cmd_port()
	{

	}
	~ftp_cmd_port()
	{

	}
	void parse_cmd(std::string type, std::string msg);
};

class ftp_cmd_retr : public ftp_task
{
public:
	ftp_cmd_retr();
	~ftp_cmd_retr();
	void parse_cmd(std::string type, std::string msg);
	void on_write(struct bufferevent* pbev);
	void on_event(struct bufferevent* pbev, short what);

private:
	FILE* fp;
};

class ftp_cmd_stor : public ftp_task
{
public:
	ftp_cmd_stor();
	~ftp_cmd_stor();
	void parse_cmd(std::string type, std::string msg);
	void on_read(struct bufferevent* pbev);
	void on_event(struct bufferevent* pbev, short what);

private:
	FILE* fp;
};

