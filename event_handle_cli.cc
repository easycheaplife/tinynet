/****************************************************************************
 Copyright (c) 2013-2014 King Lee

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string>
//	for c++	11
#include <thread>
#include <functional>

#if defined __WINDOWS || defined WIN32
#include <WinSock2.h>
#elif defined __LINUX || defined __MACX
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h> 		//	gethostname
#include <netdb.h>			//	gethostbyname
#include <sys/ioctl.h>
#endif //__LINUX_

#include "event_handle_cli.h"
#include "reactor_impl_select.h"
#include "reactor.h"

Event_Handle_Cli::Event_Handle_Cli(Reactor* __reactor,const easy_char* __host,easy_uint32 __port) : Event_Handle(__reactor),host_(__host),port_(__port)
{
	_init(__port);
	reactor()->reactor_impl()->register_handle(this,get_handle(),kMaskConnect);
};

easy_int32 Event_Handle_Cli::handle_input(easy_int32 __fd)
{
	on_read(__fd);
	return -1;
}

easy_int32 Event_Handle_Cli::handle_output(easy_int32 __fd)
{
	reactor()->reactor_impl()->register_handle(this,__fd,kMaskRead);
	return -1;
}

easy_int32 Event_Handle_Cli::handle_exception(easy_int32 __fd)
{
	return -1;
}

easy_int32 Event_Handle_Cli::handle_close(easy_int32 __fd)
{
	return -1;
}

easy_int32 Event_Handle_Cli::handle_timeout(easy_int32 __fd)
{
	return -1;
}

void Event_Handle_Cli::_init(easy_uint32 __port)
{
#if defined __WINDOWS || defined WIN32
	easy_uint16 __version_requested = MAKEWORD(2,2);
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
#endif //__WINDOWS
	fd_ = socket(AF_INET,SOCK_STREAM,0); 
	if ( -1 == fd_ )
	{
		perror("error at socket");
		exit(1);
	}
	struct sockaddr_in __clientaddr;  
	memset(&__clientaddr,0,sizeof(sockaddr_in));  
	__clientaddr.sin_family = AF_INET;  
	__clientaddr.sin_port = htons(port_);  
	__clientaddr.sin_addr.s_addr = inet_addr(host_.c_str());
	easy_int32 __res = connect(fd_,(sockaddr*)&__clientaddr,sizeof(sockaddr_in));
	if(-1 == __res)
	{
		perror("error at connect");
		exit(1);
	}
}

void Event_Handle_Cli::_set_noblock(easy_int32 __fd)
{
#if defined __WINDOWS || defined WIN32
	easy_ulong __non_block = 1;
	if (SOCKET_ERROR == ioctlsocket(__fd, FIONBIO, &__non_block))
	{
		printf("_set_noblock() error at ioctlsocket,error code = %d\n", WSAGetLastError());
	}
#elif defined __LINUX || defined __MACX
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
#endif //__WINDOWS
}

void Event_Handle_Cli::write( const easy_char* __data,easy_uint32 __length )
{
	easy_int32 __send_bytes = send(fd_,__data,__length,0);
	if(-1 == __send_bytes)
	{
#if defined __WINDOWS || defined WIN32
		easy_ulong __last_error = ::GetLastError();
		if(WSAEWOULDBLOCK  == __last_error)
		{
			//	disconnect from server
			handle_close(fd_);
			return;
		}
#elif defined __LINUX || defined __MACX
		//error happend but EAGAIN and EWOULDBLOCK meams that peer socket have been close
		//EWOULDBLOCK means messages are available at the socket and O_NONBLOCK  is set on the socket's file descriptor
		if(EAGAIN == errno && EWOULDBLOCK == errno)
		{
			//	disconnect from server
			handle_close(fd_);
			return;
		}
#endif // __WINDOWS
		perror("error at send");  
	}
}

void Event_Handle_Cli::write( std::string& __data)
{
	write(__data.c_str(),__data.length());
}

void Event_Handle_Cli::_work_thread()
{
	reactor()->event_loop(5000*1000);
}

void Event_Handle_Cli::_set_reuse_addr( easy_int32 __fd )
{
	easy_int32 __option_name = 1;
	if(setsockopt(__fd, SOL_SOCKET, SO_REUSEADDR, (easy_char*)&__option_name, sizeof(easy_int32)) == -1)  
	{  
		perror("setsockopt SO_REUSEADDR ");  
		exit(1);  
	}  
}

void Event_Handle_Cli::_set_no_delay( easy_int32 __fd )
{
#if defined __WINDOWS || defined WIN32
	//	The Nagle algorithm is disabled if the TCP_NODELAY option is enabled 
	easy_int32 __no_delay = TRUE;
	if(SOCKET_ERROR == setsockopt( __fd, IPPROTO_TCP, TCP_NODELAY, (char*)&__no_delay, sizeof(int)))
	{
		perror("setsockopt TCP_NODELAY");  
		exit(1);  
	}
#endif // __WINDOWS
}

void Event_Handle_Cli::_get_usable( easy_int32 __fd, easy_ulong& __usable_size)
{
#if defined __WINDOWS || defined WIN32
	if(SOCKET_ERROR == ioctlsocket(__fd, FIONREAD, &__usable_size))
	{
		printf("ioctlsocket failed with error %d\n", WSAGetLastError());
	}
#elif defined __LINUX || defined __MACX
	if(ioctl(__fd,FIONREAD,&__usable_size))
	{
		perror("ioctl FIONREAD");
	}
#endif //__WINDOWS
}

easy_int32 Event_Handle_Cli::read( easy_int32 __fd,easy_char* __buf, easy_int32 __length )
{
	easy_int32 __recv_size = recv(__fd,__buf,__length,0);
	if(0 == __recv_size)
	{
		
	}
	else if (-1 == __recv_size)
	{
#if defined __WINDOWS || defined WIN32
		easy_ulong __last_error = ::GetLastError();
		if(WSAEWOULDBLOCK  == __last_error)
		{
			//	close peer socket
		}
		closesocket(fd_);
#elif defined __LINUX || defined __MACX
		if(EAGAIN == errno || EWOULDBLOCK == errno)
		{

		}
		close(fd_);
#endif //__LINUX
	}
	return __recv_size;
}

void Event_Handle_Cli::star_work_thread()
{
	//	start work thread
	auto __thread_ = std::thread(CC_CALLBACK_0(Event_Handle_Cli::_work_thread,this));
	__thread_.detach();
}




