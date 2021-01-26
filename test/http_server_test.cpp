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


void http_request_cb(struct evhttp_request *prequest, void *arg)
{
	cout << "---------------http_request_cb----------------" << endl;

	const char *uri = evhttp_request_get_uri(prequest);
	cout << "uri:" << uri << endl << endl;

	evhttp_cmd_type requesu_method = evhttp_request_get_command(prequest);
	switch(requesu_method)
	{
		case EVHTTP_REQ_GET:
			cout << "[request line]  request method: GET" << endl << endl;
			break;
		case EVHTTP_REQ_POST:
			cout << "[request line]  request method: POST" << endl << endl;
			break;
		case EVHTTP_REQ_HEAD:
			cout << "[request line]  request method: HEAD" << endl << endl;
			break;
		default:
			cout << "unknow request method!" << endl << endl;
			break;
	}

	cout << "--------------------request head--------------------------------" << endl;
	evkeyvalq *prequest_header = evhttp_request_get_input_headers(prequest);
	if(prequest_header)
	{
		evkeyval* p = prequest_header->tqh_first;
		while(p)
		{
			cout << "request field:" << p->key << "  value:" << p->value << endl;
			p = p->next.tqe_next;
		}
	}
	cout << "--------------------request head--------------------------------" << endl << endl;

	cout << "--------------------request data--------------------------------" << endl;
	evbuffer *pbuffer = evhttp_request_get_input_buffer(prequest);
	if(pbuffer)
	{
		char temp[1024] = {0};
		while(evbuffer_get_length(pbuffer) > 0)
		{
			evbuffer_remove(pbuffer, temp, sizeof(temp) - 1);

			cout << temp ;

			memset(temp, 0, sizeof(temp));
		}
	}
	cout << "--------------------request data--------------------------------" << endl << endl;


	evbuffer *poutbufer = evhttp_request_get_output_buffer(prequest);
	evkeyvalq *prespon_header = evhttp_request_get_output_headers(prequest);

	string file_path = ".";
	file_path += uri;
	if(file_path == "/")
	{
		file_path += "index.html";
	}
	cout << "file_path:" << file_path << endl;

	int pos = file_path.rfind('.');
	string postfix = file_path.substr(pos+1, file_path.size() - (pos + 1));
	cout << "postfix:" << postfix << endl;

	if((postfix == "jpg") || (postfix == "png") || (postfix == "gif"))
	{
		string value = "image/" + postfix;
		evhttp_add_header(prespon_header, "Content-Type", value.c_str());
	}
	else if(postfix == "zip")
	{
		evhttp_add_header(prespon_header, "Content-Type", "application/zip");
	}
	else if(postfix == "html")
	{
		evhttp_add_header(prespon_header, "Content-Type", "text/html;charset=UTF-8");
	}
	else if(postfix == "css")
	{
		evhttp_add_header(prespon_header, "Content-Type", "text/css");
	}
	else
	{
		cout << "dont support postfix!" << endl;
		evhttp_send_reply(prequest, HTTP_NOTFOUND, "", NULL);
	}
	
	FILE* fp = fopen(file_path.c_str(), "rb");
	if(!fp)
	{
		cout << "open file error!" << endl;
		evhttp_send_reply(prequest, HTTP_NOTFOUND, "", NULL);
		return;
	}

	int len = 0;
	char buf[512] = {0};
	while((len = fread(buf, 1, sizeof(buf), fp)) > 0)
	{
		evbuffer_add(poutbufer, buf, len);
	}

	fclose(fp);
	
	cout << "--------------------send response--------------------------------" << endl;
	evhttp_send_reply(prequest, HTTP_OK, "", poutbufer);

}

#if 0
int main()
{
	event_base *pbase = event_base_new();
	if(!pbase)
	{
		cout << "event_base_new error!" << endl;
		return -1;
	}

	evhttp *phttp = evhttp_new(pbase);
	if(!phttp)
	{
		cout << "evhttp_new error!" << endl;
		event_base_free(pbase);
		return -1;
	}

	if(evhttp_bind_socket(phttp, "192.168.0.102", 8080) < 0)
	{
		cout << "evhttp_bind_socket error!" << endl;
		evhttp_free(phttp);
		event_base_free(pbase);
		return -1;
	}
	

	evhttp_set_gencb(phttp, http_request_cb, NULL);


	event_base_dispatch(pbase);

	evhttp_free(phttp);
	
	event_base_free(pbase);

	return 0;
}

#endif

