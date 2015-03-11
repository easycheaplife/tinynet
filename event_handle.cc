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
#include "event_handle.h"

#include <stdio.h>
#include <stdlib.h>

#if defined __WINDOWS || defined WIN32
#ifndef FD_SETSIZE
#define FD_SETSIZE      1024
#endif /* FD_SETSIZE */
#include <WinSock2.h>
#elif defined __LINUX || defined __MACX
#include <sys/socket.h>
#include <netinet/in.h>

#include <strings.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h> 		//	gethostname
#include <netdb.h>			//	gethostbyname
#include <sys/ioctl.h>		//	ioctl
#include <execinfo.h>
#endif //   __WINDOWS
#include "reactor.h"
#include "reactor_impl.h"

easy_int32 Event_Handle::read( easy_int32 __fd,easy_char* __buf, easy_int32 __length,easy_int32 __flags /*= 0*/ )
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
			//	do nothing
		}
#elif defined __LINUX || defined __MACX
		if(EAGAIN == errno || EWOULDBLOCK == errno)
		{
			//	do nothing
		}
#endif //__WINDOWS
		else
		{
			reactor()->reactor_impl()->handle_close(__fd);
		}
	}
	return __recv_size;
}

easy_int32 Event_Handle::write( easy_int32 __fd,const easy_char* __data, easy_int32 __length )
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
			//	do nothing
		}
#elif defined __LINUX || defined __MACX
		//error happend but EAGAIN and EWOULDBLOCK meams that peer socket have been close
		//EWOULDBLOCK means messages are available at the socket and O_NONBLOCK  is set on the socket's file descriptor
		// ECONNRESET means an existing connection was forcibly closed by the remote host
		if((EAGAIN == errno || EWOULDBLOCK == errno))
		{
			//	do nothing
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
