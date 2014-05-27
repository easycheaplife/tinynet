#include <stdlib.h>			//	exit
#include <netinet/in.h>		//	sockaddr_in
#include <strings.h>		//	bzero
#include <arpa/inet.h>		//	inet_addr
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <string.h>

char __send_buf[256];
int main()
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(-1 == sock)
	{
		std::cout << "error at socket"<<std::endl;
		exit(1);
	}
	struct sockaddr_in clientaddr;
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = inet_addr("192.168.22.63");
	clientaddr.sin_port = htons(9876);
	int res = connect(sock,(sockaddr*)&clientaddr,sizeof(sockaddr_in));
	if(-1 == res)
	{
		std::cout << "error at connect"<<std::endl;
		exit(1);
	}
	int __length = strlen("nice to meeet you!");
	int __log_level = 1;
	int __head = 0;
	//	set head
	__head |= (__length << 16);
	__head = __head |= (__log_level << 12);
	//	get head
	int __res_length = 0;
	__res_length = __head >> 16;
	int __res_log_level = 0;
	__res_log_level = (__head >> 12) & 0x0000000f;
	
	for(;;)
	{
		memset(__send_buf,0,256);
		strcpy(__send_buf,"nice to meeet you!");
		//	use packet head
		if(1)
		{
			send(sock,(void*)&__head,4,0);
		}
		else
		{
			send(sock,(void*)&__length,4,0);
		}
		int send_bytes = send(sock,(void*)__send_buf,__length,0);
		if(-1 != send_bytes)
		{
			std::cout << send_bytes << " bytes data send: " << "nice to meeet you!" << std::endl;
		}
		/*
		int recv_data = 0;
		int recv_bytes = recv(sock,(void*)&recv_data,sizeof(int),0);
		if(-1 != recv_bytes)
		{
			std::cout << recv_bytes << " bytes data recv: " << recv_data << std::endl;
		}
		*/
		usleep(1000*10);
	}
}
