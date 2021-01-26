#pragma once

#include "thread_task.h"
#include <event2/bufferevent.h>
#include <map>
#include <list>
#include <string>

class ftp_task : public thread_task
{
public:
	ftp_task();
	virtual ~ftp_task();
	int init();
	void set_callback(struct bufferevent* pbev);
	void respon_cmd(std::string msg);
	void send_data_to_client(std::string msg);
	void send_data_to_client(const char* data, int len);
	void connect_to_client();
	void close_bufferevent();
	virtual void on_read(struct bufferevent* pbev);
	virtual void on_write(struct bufferevent* pbev);
	virtual void on_event(struct bufferevent* pbev, short what);
	virtual void parse_cmd(std::string type, std::string msg);
	std::string root_dir;
	std::string current_dir;
	ftp_task* pcmd_task;
	std::string client_ip;
	int client_port;

protected:
	static void read_callback(struct bufferevent* bev, void* arg);
	static void write_callback(struct bufferevent* bev, void* arg);
	static void event_callback(struct bufferevent* bev, short what, void* arg);	
	struct bufferevent* m_pbev;
};



class ftp_server_cmd : public ftp_task
{
public:
	ftp_server_cmd();
	virtual ~ftp_server_cmd();
	int init();
	void on_read(struct bufferevent* pbev);
	void on_write(struct bufferevent* pbev);
	void on_event(struct bufferevent* pbev, short what);
	void register_cmd(std::string cmd, ftp_task* ptask);

private:
	std::map<std::string, ftp_task*> cmds_map;
	std::map<ftp_task*, int> cmds_delete_map;
	
};
