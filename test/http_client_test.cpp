#include <stdio.h>
#include <signal.h>

#include <unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

#include <errno.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>


#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>

#include <string.h>
#include <iostream>
using namespace std;

void http_client_request_cb(struct evhttp_request *prequest, void *arg)
{
	cout << "-----http_client_request_cb-----" << endl;

	if(!prequest)
	{
		int errcode = EVUTIL_SOCKET_ERROR();
		cout << "prequest be NULL!  code:" << errcode << " reson:" << evutil_socket_error_to_string(errcode) << endl;
		return;
	}

	const char* uri = evhttp_request_get_uri(prequest);
	if(uri)
	{
		cout << "uri:" << uri << endl;
	}

	int status_code = evhttp_request_get_response_code(prequest);
	cout << "status_code:" << status_code << endl;

	const char*respon_line = evhttp_request_get_response_code_line(prequest);
	if(respon_line)
	{
		cout << "respon_line:" << respon_line << endl;
	}

	evbuffer *pinput_buffer = evhttp_request_get_input_buffer(prequest);
	if(pinput_buffer)
	{
		int len = 0;
		char buf[1024] = {0};

		cout << "respon data:" << endl;
		while((len = evbuffer_remove(pinput_buffer, buf, sizeof(buf))) > 0)
		{
			cout << buf << endl;
			memset(buf, 0, sizeof(buf));
		}
	}

}


int main()
{
	event_base *pbase = event_base_new();
	if(!pbase)
	{
		cout << "event_base_new error!" << endl;
		return -1;
	}

	string url = "http://ffmpeg.club/index.html";

	evhttp_uri *phttp_uri = evhttp_uri_parse(url.c_str());
	if(!phttp_uri)
	{
		cout << "evhttp_uri_parse error!" << endl;
		event_base_free(pbase);
		return -1;
	}

	const char* scheme = evhttp_uri_get_scheme(phttp_uri);
	if(!scheme)
	{
		cout << "evhttp_uri_get_scheme error!" << endl;
		event_base_free(pbase);
		return -1;
	}
	cout << "scheme:" << scheme << endl;

	const char* host = evhttp_uri_get_host(phttp_uri);
	if(!host)
	{
		cout << "evhttp_uri_get_host error!" << endl;
		event_base_free(pbase);
		return -1;
	}
	cout << "host:" << host << endl;

	int port = evhttp_uri_get_port(phttp_uri);
	if(port < 0)
	{
		port = 80;
	}
	cout << "port:" << port << endl;

	const char* path = evhttp_uri_get_path(phttp_uri);
	if(path)
	{
		cout << "path:" << path << endl;
	}

	const char* query = evhttp_uri_get_query(phttp_uri);
	if(query)
	{
		cout << "query:" << query << endl;
	}



	bufferevent *pbev = bufferevent_socket_new(pbase, -1, BEV_OPT_CLOSE_ON_FREE);
	if(!pbev)
	{
		cout << "bufferevent_socket_new error!" << endl;
		event_base_free(pbase);
		return -1;
	}

	evhttp_connection *pconnection = evhttp_connection_base_bufferevent_new(pbase, NULL, pbev, host, port);
	if(!pconnection)
	{
		cout << "evhttp_connection_base_bufferevent_new error!" << endl;
		bufferevent_free(pbev);
		event_base_free(pbase);
		return -1;
	}

	evhttp_request *prequest = evhttp_request_new(http_client_request_cb, pbev);
	if(!prequest)
	{
		cout << "evhttp_request_new error!" << endl;
		bufferevent_free(pbev);
		event_base_free(pbase);
		return -1;
	}
	evkeyvalq *poutput_headers = evhttp_request_get_output_headers(prequest);
	if(poutput_headers)
	{
		evhttp_add_header(poutput_headers, "Host", host);
	}	

	if(evhttp_make_request(pconnection, prequest, EVHTTP_REQ_GET, url.c_str()) < 0)
	{
		cout << "evhttp_make_request error!" << endl;
	}

	event_base_dispatch(pbase);

	bufferevent_free(pbev);
	
	event_base_free(pbase);

	return 0;
}

