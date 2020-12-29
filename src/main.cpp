#include <stdio.h>


#include<event.h>
#include<event2/util.h>

#include <zlib.h>

#include <openssl/ssl.h> 
#include <openssl/err.h> 

int main(int argc, char const *argv[])
{

	unsigned char strsrc[]="这些是测试数据。123456789 abcdefghigklmnopqrstuvwxyz\n\t\0abcdefghijklmnopqrstuvwxyz\n"; //包含\0字符
	unsigned char buf[1024]={0};
	unsigned long srclen=sizeof(strsrc);
    unsigned long buflen=sizeof(buf);
	compress(buf,&buflen,strsrc,srclen);
	printf("压缩后实际长度为:%ld\n",buflen);

    struct event_base* base = event_base_new();



    SSL_library_init();                
	SSL_load_error_strings(); 


    printf("=====================================\n");

    event_base_dispatch(base);
	return 0;
}