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
#include "socket_ex.h"

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
	_set_noblock(fd_);
}

void Event_Handle_Cli::_set_noblock(easy_int32 __fd)
{
	Socket_Ex::set_noblock(__fd);
}

easy_int32 Event_Handle_Cli::write( const easy_char* __data,easy_uint32 __length )
{
	return Event_Handle::write(fd_,__data,__length);
}

easy_int32 Event_Handle_Cli::write( std::string& __data)
{
	return write(__data.c_str(),__data.length());
}

void Event_Handle_Cli::_work_thread()
{
	reactor()->event_loop(5000*1000);
}

void Event_Handle_Cli::_set_reuse_addr( easy_int32 __fd )
{
	Socket_Ex::set_reuse_addr(__fd);
}

void Event_Handle_Cli::_set_no_delay( easy_int32 __fd )
{
	Socket_Ex::set_no_delay(__fd);
}

void Event_Handle_Cli::_get_usable( easy_int32 __fd, easy_ulong& __usable_size)
{
	Socket_Ex::get_usable(__fd,__usable_size);
}

easy_int32 Event_Handle_Cli::read( easy_int32 __fd,easy_char* __buf, easy_int32 __length,easy_int32 __flags/* = 0*/ )
{
	return Event_Handle::read(__fd,__buf,__length,__flags);
}

void Event_Handle_Cli::star_work_thread()
{
	//	start work thread
	auto __thread_ = std::thread(CC_CALLBACK_0(Event_Handle_Cli::_work_thread,this));
	__thread_.detach();
}




