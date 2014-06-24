#include <stdlib.h>			//	exit
#include <netinet/in.h>		//	sockaddr_in
#include <strings.h>		//	bzero
#include <arpa/inet.h>		//	inet_addr
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
static std::string __random_string[] = 
{
	"[0x000085e4][T]AdvertisingIndentitifer: '', IdentifierForVendor: '', DeviceName: 'PC', ModelName: 'x86', SystemName: '', SystemVersion: '', HardwareID: '74d435046509'",
	"nice to meet you!",
	"It is the tears of the earth that keep here smiles in bloom.",
	"The mighty desert is burning for the love of a blade of grass who shakes her head and laughs and flies away.",
	"If you shed tears when you miss the sun, you also miss the stars.",
	"Her wishful face haunts my dreams like the rain at night.",
	"Once we dreamt that we were strangers.We wake up to find that we  were dear to each other.",
	"Sorrow is hushed into peace in my heart like the evening among the silent trees.",
	"Some unseen fingers, like an idle breeze, are playing upon my heart the music of the ripples.",
	"Listen, my heart, to the whispers of the world with which it makes love to you.",
	"Do not seat your love upon a precipice because it is high.",
	"I sit at my window this morning where the world like a passer-by stops for a moment, nods to me and goes.",
	"There little thoughts are the rustle of leaves; they have their whisper of joy in my mind.",
	"What you are you do not see, what you see is your shadow.",
	"My wishes are fools, they shout across thy song, my Master.Let me but listen.",
	"They throw their shadows before them who carry their lantern on their back.",
	"That I exist is  a perpetual surprise which is life.",
	"We, the rustling leaves, have a voice that answers the storms,but who are you so silent?I am a mere flower.",
	"Do not blame your food because you have no appetite.",
	"Success is not final, failure is not fatal: it is the courage to continue that counts.",
	"I cannot tell why this heart languishes in silence.It is for small needs it never asks, or knows or remembers.",
	"The bird wishes it were a cloud.The cloud wishes it were a bird."
};

static int __random_string_size = 22;

void 	_set_noblock(int __fd)
{
	int __opts = fcntl(__fd,F_GETFL);  
	if(0 > __opts)  
    {  
      	perror("error at fcntl(sock,F_GETFL)");  
       	exit(1);  
    }  
	 __opts = __opts | O_NONBLOCK;  
	if( 0 > fcntl(__fd,F_SETFL,__opts) )  
	{  
       	perror("error at fcntl(sock,F_SETFL)");  
       	exit(1);  
   	}  
}

void output(const char* __fmt,...)
{
#ifdef __DEBUG
    static const int __output_size = 1024;
    char __buf[__output_size] = {};
    va_list __args;
    va_start(__args,__fmt);
    vsnprintf(__buf, __output_size, __fmt, __args);
    va_end(__args);
    printf("%s\n",__buf);
#endif //__DEBUG
}

void test_4_transform_monitor(int sock)
{
	/*
	xx | xx | xx  | xx
		length
	xx        | xx | xx  | xx
	log level | 	frame number(index)
	*/
	srand( (unsigned)time(NULL)); 
	int __log_level = 1;
	int __frame_number = 7;
	int __guid = 15;
	int __res_frane_number = 0;
	int __res_log_level = 0;
	int __head = 0;
	//	set head
	__head |= (__frame_number << 8);
	__head |= (__log_level);
	
	//	get head
	__res_frane_number = (__head ) >> 8;
	__res_log_level = (__head) & 0x000000ff;

	int __random_index = 0;
	
	char __send_buf[256];
	char __recv_buf[256];
	static const int __packet_head_size = 12;
	unsigned char __packet_head[__packet_head_size] = {};
	for(int __i = 0; ; ++__i)
	{
		//	the first time must send the DeviceName
		if(0 != __i)
		{
			__random_index = rand()%__random_string_size;
		}
		int __length = __random_string[__random_index].size();
		
		memset(__send_buf,0,256);
		strcpy(__send_buf,__random_string[__random_index].c_str());
		//	use packet head
		memset(__packet_head,0,__packet_head_size);
		memcpy(__packet_head,(void*)&__length,4);
		memcpy(__packet_head + 4,(void*)&__head,4);
		memcpy(__packet_head + 8,(void*)&__guid,4);
		send(sock,(void*)&__packet_head,12,0);

		int send_bytes = send(sock,(void*)__send_buf,__length,0);
		if(-1 != send_bytes)
		{
			output("%d byte send: %s",send_bytes,__random_string[__random_index].c_str());
		}
		//	receive data
		__length = 0;
		__head = 0;
		__guid = 0;
		memset(__packet_head,0,__packet_head_size);
		int recv_bytes = recv(sock,(void*)&__packet_head,__packet_head_size,0);
		if(12 != recv_bytes)
		{
			std::cout << " __packet_head error! "<< std::endl;
		}
		memcpy(&__length,(void*)__packet_head,4);
		memcpy(&__head,(void*)(__packet_head + 4),4);
		memcpy(&__guid,(void*)(__packet_head + 8),4);
		
		memset(__recv_buf,0,256);
		recv_bytes = recv(sock,(void*)__recv_buf,__length,0);
		if(-1 != recv_bytes)
		{
			output("%d bytes data recv: %s",recv_bytes,__recv_buf);
		}
		usleep(1000*100);
	}
}
int main(int __arg_num, char** __args)
{
	if(3 != __arg_num)
	{
		std::cout << "param error,please input correct param,for example : ./echo_c 192.168.22.61 9876" <<std::endl;
		exit(1);
	}
	const char* __host = __args[1];
	unsigned int __port = atoi(__args[2]);
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == sock)
	{
		std::cout << "error at socket"<<std::endl;
		exit(1);
	}
	struct sockaddr_in clientaddr;
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = inet_addr(__host);
	clientaddr.sin_port = htons(__port);
	int res = connect(sock,(sockaddr*)&clientaddr,sizeof(sockaddr_in));
	if(-1 == res)
	{
		std::cout << "error at connect,errno = "<< errno <<std::endl;
		exit(1);
	}
	test_4_transform_monitor(sock);
}
