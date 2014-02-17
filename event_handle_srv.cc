#include <stdio.h>
#include <stdlib.h>

#ifndef __LINUX
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h> 		//	gethostname
#include <netdb.h>			//	gethostbyname
#endif //__LINUX_

#include "event_handle_srv.h"
#include "reactor_impl_select.h"
#include "reactor.h"

Event_Handle_Srv::Event_Handle_Srv(Reactor* __reactor) : Event_Handle(__reactor)
{
	_init();
	reactor()->reactor_impl()->register_handle(this,get_handle(),kMaskAccept);
};

int Event_Handle_Srv::handle_input(int __fd)
{
	if(__fd == fd_)
	{
		int __fd_accept = accept(fd_,NULL,NULL);
		if(-1 != __fd_accept)
		{
			_set_noblock(__fd_accept);
			reactor()->reactor_impl()->register_handle(this,__fd_accept,kMaskRead,1);
		}
	}
	else
	{
		printf("new data comming,wiatting for recv\n");
#if 1
		char __buf[8192] = {0};
		int __recv_size = recv(__fd,__buf,8192,0);
		if(0 == __recv_size)
		{
			perror("error at recv");  
			return -1;
		}
#endif
		reactor()->reactor_impl()->register_handle(this,__fd,kMaskWrite);
	}
	return -1;
}

int Event_Handle_Srv::handle_output(int __fd)
{
	printf("handle_outputd\n");
#if 1
	static int __data = 0;
	++__data;
	int __send_size = send(__fd,(char*)&__data,sizeof(int),0);
	if( 0 == __send_size )
	{
		perror("error at send");  
		return -1;
	}
#endif
	reactor()->reactor_impl()->register_handle(this,__fd,kMaskRead);
	return -1;
}

int Event_Handle_Srv::handle_exception(int __fd)
{
	printf("handle_exception\n");
	return -1;
}

int Event_Handle_Srv::handle_close(int __fd)
{
	printf("handle_close\n");
	return -1;
}

int Event_Handle_Srv::handle_timeout(int __fd)
{
	printf("handle_timeout\n");
	return -1;
}

void Event_Handle_Srv::_init(unsigned int __port)
{
#ifndef __LINUX
	WORD __version_requested = MAKEWORD(2,2);
	WSADATA __data;
	if (0 != WSAStartup( __version_requested, &__data))
	{
		//Tell the user that we could not find a usable WinSock DLL.
		return;
	}
	if ( LOBYTE( __data.wVersion ) != 2 ||
		HIBYTE( __data.wVersion ) != 2 )
	{
		// Tell the user that we could not find a usable WinSock DLL.
		WSACleanup();
		return;
	}
#endif //__LINUX
	fd_ = socket(AF_INET,SOCK_STREAM,0); 
	if ( -1 == fd_ )
	{
		perror("error at socket");
		exit(1);
	}
	struct sockaddr_in __serveraddr;  
	memset(&__serveraddr,0,sizeof(sockaddr_in));  
	__serveraddr.sin_family = AF_INET;  
	__serveraddr.sin_port = htons(__port);  
#if 1
	//	get local ip address
	static const int __name_len = 128;
	char __name[__name_len] = {0};
	gethostname(__name,__name_len);
	struct hostent* __host_entry = gethostbyname(__name);
	if(__host_entry)
	{
		printf("hostname: %s \n address list: \n", __host_entry->h_name);
		for(int __i = 0; __host_entry->h_addr_list[__i]; __i++) 
		{
		  	printf("%s\n", inet_ntoa(*(struct in_addr*)(__host_entry->h_addr_list[__i])));
		}
	}
#endif
	__serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	int __ret = bind(fd_,(sockaddr*)&__serveraddr,sizeof(sockaddr_in));  
	if ( -1 == fd_ )
	{
		perror("error at bind");
		exit(1);
	}
	//	A backlog argument of 0 may allow the socket to accept connections, in which case the length of the listen queue may be set to an implementation-defined minimum value.
	__ret = listen(fd_,0);
	if ( -1 == fd_ )
	{
		perror("error at bind");
		exit(1);
	}	
	_set_noblock(fd_);
	int __option_name = 1;
	if(setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, (char*)&__option_name, sizeof(int)) == -1)  
	{  
		perror("setsockopt");  
		exit(1);  
	}  
}

void Event_Handle_Srv::_set_noblock(int __fd)
{
#ifndef __LINUX
	unsigned long __non_block = 1;
	if (SOCKET_ERROR == ioctlsocket(__fd, FIONBIO, &__non_block))
	{
		printf("_set_noblock() error at ioctlsocket,error code = %d\n", WSAGetLastError());
	}
#else
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
#endif //__LINUX
}
