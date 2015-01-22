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

#if defined __WINDOWS || defined WIN32
#include <WinSock2.h>
#elif defined __LINUX || defined __MACX
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h> 		//	gethostname
#include <netdb.h>			//	gethostbyname
#include <sys/ioctl.h>		//	ioctl
#include <execinfo.h>
#endif //   __WINDOWS

#include "event_handle_srv.h"
#include "reactor_impl_select.h"
#include "reactor.h"

Event_Handle_Srv::Event_Handle_Srv(Reactor* __reactor,const easy_char* __host,easy_uint32 __port) : Event_Handle(__reactor),host_(__host),port_(__port)
{
	_init();
	reactor()->reactor_impl()->register_handle(this,get_handle(),kMaskAccept);
};


Event_Handle_Srv::~Event_Handle_Srv()
{

}

easy_int32 Event_Handle_Srv::handle_input(easy_int32 __fd)
{
#ifndef __HAVE_IOCP
	if(__fd == fd_)
	{
		easy_int32 __fd_accept = accept(fd_,NULL,NULL);
		if(-1 != __fd_accept)
		{
			_set_noblock(__fd_accept);
			reactor()->reactor_impl()->register_handle(this,__fd_accept,kMaskRead,1);
			on_connected(__fd_accept);
		}
	}
	else
	{
		//	read data from system buffer and write to ring buffer, that will reduce a memory copy in every data transform
		on_read(__fd);
	}
#else
	on_connected(__fd);
#endif // __HAVE_IOCP
	return 0;
}

easy_int32 Event_Handle_Srv::handle_output(easy_int32 __fd)
{
#ifdef __DEBUG
	printf("handle_outputd\n");
#endif //__DEBUG
	//	test data, if open it, it will cause something wrong
#if 0
	static easy_int32 __data = 0;
	++__data;
	easy_int32 __send_size = send(__fd,(easy_char*)&__data,sizeof(easy_int32),0);
	if( 0 == __send_size )
	{
		perror("error at send");  
		return -1;
	}
#endif
	reactor()->reactor_impl()->register_handle(this,__fd,kMaskRead);
	return -1;
}

easy_int32 Event_Handle_Srv::handle_exception(easy_int32 __fd)
{
	printf("handle_exception\n");
	return -1;
}

easy_int32 Event_Handle_Srv::handle_close(easy_int32 __fd)
{
#ifndef __HAVE_IOCP
#ifdef __DEBUG
#ifdef __LINUX
	const easy_int32 __max_stack_flow = 20;
	void* __array[__max_stack_flow];
	easy_char** __strings;
	size_t __size = backtrace(__array,__max_stack_flow);
	printf("backtrace() returned %d addresses\n", (int)__size);
	__strings = backtrace_symbols(__array,__size);
	if(NULL == __strings)
	{
		perror("backtrace_symbols");
		exit(EXIT_FAILURE);
	}
	fprintf (stderr,"obtained %zd stack frames.nm", __size);
	for (size_t __i = 0; __i < __size; ++__i)
	{
		printf("%s\n", __strings[__i]);
	}
	//	This __strings is malloc(3)ed by backtrace_symbols(), and must be freed here
	free (__strings);
#endif // __LINUX
	printf("socket close %d,errno %d\n",__fd,errno);
#endif // __DEBUG
#endif // __HAVE_IOCP
	on_disconnect(__fd);
	return -1;
}

easy_int32 Event_Handle_Srv::handle_timeout(easy_int32 __fd)
{
	return -1;
}

easy_int32 Event_Handle_Srv::handle_packet( easy_int32 __fd,const easy_char* __packet,easy_int32 __length )
{
#ifdef __HAVE_IOCP
	on_packet(__fd,__packet,__length);
#endif // __HAVE_IOCP
	return -1;
}


void Event_Handle_Srv::_init()
{
#if defined __WINDOWS || defined WIN32
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
#endif //__WINDOWS
	//	the socket that is created will have the overlapped attribute as a default
	fd_ = socket(AF_INET,SOCK_STREAM,0); 
	if ( -1 == fd_ )
	{
		perror("error at socket");
		exit(1);
	}
	struct sockaddr_in __serveraddr;  
	memset(&__serveraddr,0,sizeof(sockaddr_in));  
	__serveraddr.sin_family = AF_INET;  
	__serveraddr.sin_port = htons(port_);  
#if 1
	//	get local ip address
	static const easy_int32 __name_len = 128;
	easy_char __name[__name_len] = {0};
	gethostname(__name,__name_len);
	struct hostent* __host_entry = gethostbyname(__name);
	if(__host_entry)
	{
		printf("hostname: %s \naddress list: \n", __host_entry->h_name);
		for(easy_int32 __i = 0; __host_entry->h_addr_list[__i]; __i++) 
		{
		  	printf("%s\n", inet_ntoa(*(struct in_addr*)(__host_entry->h_addr_list[__i])));
		}
	}
#endif
	__serveraddr.sin_addr.s_addr = inet_addr(host_.c_str());
	easy_int32 __ret = bind(fd_,(sockaddr*)&__serveraddr,sizeof(sockaddr_in));  
	if ( -1 == __ret )
	{
		perror("error at bind");
		exit(1);
	}
	//	A backlog argument of 0 may allow the socket to accept connections, in which case the length of the listen queue may be set to an implementation-defined minimum value.
	__ret = listen(fd_,0);
	if ( -1 == __ret )
	{
		perror("error at bind");
		exit(1);
	}	
	_set_noblock(fd_);
	_set_reuse_addr(fd_);
}

void Event_Handle_Srv::_set_noblock(easy_int32 __fd)
{
#if defined __WINDOWS || defined WIN32
	unsigned long __non_block = 1;
	if (SOCKET_ERROR == ioctlsocket(__fd, FIONBIO, &__non_block))
	{
		printf("_set_noblock() error at ioctlsocket,error code = %d\n", WSAGetLastError());
	}
#elif defined __LINUX || defined __MACX
	easy_int32 __opts = fcntl(__fd,F_GETFL);  
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


void Event_Handle_Srv::_set_reuse_addr(easy_int32 __fd)
{
	easy_int32 __option_name = 1;
	if(setsockopt(__fd, SOL_SOCKET, SO_REUSEADDR, (easy_char*)&__option_name, sizeof(easy_int32)) == -1)  
	{  
		perror("setsockopt SO_REUSEADDR");  
		exit(1);  
	}  
}

void Event_Handle_Srv::_set_no_delay(easy_int32 __fd)
{
#if defined __WINDOWS || defined WIN32
	//	The Nagle algorithm is disabled if the TCP_NODELAY option is enabled 
	easy_int32 __no_delay = TRUE;
	if(SOCKET_ERROR == setsockopt( __fd, IPPROTO_TCP, TCP_NODELAY, (easy_char*)&__no_delay, sizeof(int)))
	{
		perror("setsockopt TCP_NODELAY ");  
		exit(1);  
	}
#endif //__WINDOWS
}

void Event_Handle_Srv::_get_usable( easy_int32 __fd, easy_ulong& __usable_size)
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
#endif //__LINUX
}


void Event_Handle_Srv::broadcast(easy_int32 __fd,const easy_char* __data,easy_uint32 __length)
{
	reactor()->reactor_impl()->broadcast(__fd,__data,__length);
}

easy_int32 Event_Handle_Srv::read( easy_int32 __fd,easy_char* __buf, easy_int32 __length, easy_int32 __flags/* = 0*/ )
{
	if (0 == __length)
	{
		return 0;
	}
	easy_int32 __recv_size = recv(__fd,__buf,__length,__flags);
	//	These calls return the number of bytes received, or -1 if an error occurred.  
	//	The return value will be 0 when the peer has performed an orderly shutdown
	if(0 == __recv_size)
	{
		//	socket close *,errno 11
		reactor()->reactor_impl()->handle_close(__fd);
	}
	else if (-1 == __recv_size)
	{
#if defined __WINDOWS || defined WIN32
		DWORD __last_error = ::GetLastError();
		if(WSAEWOULDBLOCK  == __last_error)
		{
			printf("recv error at %d\n",__last_error);
		}
#elif defined __LINUX || defined __MACX
		if(EAGAIN == errno || EWOULDBLOCK == errno)
		{
			printf("recv errno %d\n",errno);
		}
#endif //__WINDOWS
		else
		{
			reactor()->reactor_impl()->handle_close(__fd);
		}
	}
	return __recv_size;
}

easy_int32 Event_Handle_Srv::read_zero_copy(easy_int32 __fd,easy_char* __buf, easy_int32 __length,easy_int32 __flags /*= 0*/)
{
	return read(__fd,__buf,__length,__flags);
}

easy_int32 Event_Handle_Srv::write( easy_int32 __fd,const easy_char* __data, easy_int32 __length )
{
#ifndef __HAVE_IOCP
	if (0 == __length)
	{
		return 0;
	}
	easy_int32 __send_bytes = send(__fd,__data,__length,0);
	if(-1 == __send_bytes)
	{
#if defined __WINDOWS || defined WIN32
		DWORD __last_error = ::GetLastError();
		if(WSAEWOULDBLOCK  == __last_error)
		{	
			printf("send error at %d\n",__last_error);
		}
#elif defined __LINUX || defined __MACX
		//error happend but EAGAIN and EWOULDBLOCK meams that peer socket have been close
		//EWOULDBLOCK means messages are available at the socket and O_NONBLOCK  is set on the socket's file descriptor
		// ECONNRESET means an existing connection was forcibly closed by the remote host
		if((EAGAIN == errno && EWOULDBLOCK == errno))
		{
			printf("send errno %d\n",errno);
		}
#endif // __WINDOWS
		else
		{
		//	close peer socket
			reactor()->reactor_impl()->handle_close(__fd);
		}
	}
	return __send_bytes;
#else
	return reactor()->reactor_impl()->handle_packet(__fd,__data,__length);
#endif // __HAVE_IOCP
}







