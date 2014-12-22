/****************************************************************************
 Copyright (c) 2013-2014  King Lee

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
#include <stdlib.h>
#include <stdio.h>
#include "reactor_impl_select.h"
#include "event_handle.h"
#include "easy_allocator.h"
#include "easy_util.h"

const easy_uint32 Reactor_Impl_Select::max_sleep_time_ = 1000*100;

struct Event_Handle_Data : public easy::my_alloc
{
	Event_Handle_Data(easy_int32 __fd,Event_Handle* __event_handle):invalid_fd_(-1),fd_(__fd),event_handle_(__event_handle) {}
	easy_int32		invalid_fd_;
	easy_int32		fd_;
	Event_Handle*	event_handle_;
};

Reactor_Impl_Select::Reactor_Impl_Select()
{
	FD_ZERO(&read_set_);  
	FD_ZERO(&write_set_);  
	FD_ZERO(&excepion_set_); 
	handle_ = NULL;
	fd_ = -1;
	max_fd_ = -1;
}
easy_int32 Reactor_Impl_Select::register_handle(Event_Handle* __handle,easy_int32 __fd,easy_int32 __mask,easy_int32 __connect)
{
	if(kMaskAccept ==__mask || kMaskConnect == __mask)
	{
		fd_ = __fd;
		handle_ = __handle;
	}
	else
	{
		if(1 == __connect)
		{
			events_.push_back(new Event_Handle_Data(__fd,__handle));
		}
	}
	if(max_fd_ < __fd)
	{
		max_fd_ = __fd;
	}
	return -1;
}
easy_int32 Reactor_Impl_Select::remove_handle(Event_Handle* __handle,easy_int32 __mask)
{
	return -1;
}
easy_int32 Reactor_Impl_Select::handle_event(easy_ulong __millisecond)
{
	return -1;
}
easy_int32 Reactor_Impl_Select::event_loop(easy_ulong __millisecond)
{
	while(true)
	{
		FD_ZERO(&read_set_);  
		FD_ZERO(&write_set_);  
		FD_ZERO(&excepion_set_); 
		FD_SET(fd_,&read_set_); 
		FD_SET(fd_,&write_set_); 
		FD_SET(fd_,&excepion_set_);
        struct timeval __tv;
#ifndef __MACX
		__tv.tv_sec = 0;  
		__tv.tv_usec = __millisecond;
#else
        __tv.tv_sec = __millisecond/1000/1000;
        __tv.tv_usec = 0;
#endif //__MACX

		for (std::vector<Event_Handle_Data*>::iterator __it = events_.begin(); __it != events_.end(); )
		{
			if(*__it)
			{
				if((*__it)->invalid_fd_)
				{
					FD_SET((*__it)->fd_,&read_set_);  
					FD_SET((*__it)->fd_,&write_set_);  
					FD_SET((*__it)->fd_,&excepion_set_);  
					++__it;
				}
				else
				{
					//	socket is to be closed,release all resource
#if defined __LINUX || defined __MACX
					close((*__it)->fd_);
#else
					closesocket((*__it)->fd_);
#endif //__LINUX
					(*__it)->event_handle_->handle_close((*__it)->fd_);
					
					delete (*__it);
					(*__it) = NULL;
					__it = events_.erase(__it);
				}
			}
		}

		//	you must set max_fd_ is max use fd under unix/linux system, if not,part of fd will not be detected.
		//	if write_set_ is not null, that means the write status will be watched to see. 
		//	FD_SETSIZE = 64 at windows,so the max number of fd is FD_SETSIZE.if you want change it value, define before winsock.h.
		easy_int32 __ret = select(max_fd_ + 1,&read_set_,/*&write_set_*/NULL,&excepion_set_,&__tv);
		if ( -1 == __ret )
		{
			perror("error at select");
#if defined __WINDOWS || defined WIN32
			DWORD __last_error = ::WSAGetLastError();
			//	usually some socket is closed, such as closesocket called. it maybe exist a invalid socket.
			if(WSAENOTSOCK == __last_error)
			{
				
			}
#endif // __WINDOWS
			exit(1);
		}
		else if ( 0 == __ret )
		{
			handle_->handle_timeout(fd_);
			continue;
		}
		if(FD_ISSET(fd_,&read_set_))
		{
			handle_->handle_input(fd_);
			continue;
		}

		for (std::vector<Event_Handle_Data*>::iterator __it = events_.begin(); __it != events_.end(); ++__it)
		{
			if (*__it)
			{
				//	something to be read
				if(FD_ISSET((*__it)->fd_,&read_set_))
				{
					if((*__it)->event_handle_->handle_input((*__it)->fd_))
					{
						//	error happened, disconnect the socket
						(*__it)->invalid_fd_ = 0;
					}
				}
				//	something to be write
				else if(FD_ISSET((*__it)->fd_,&write_set_))
				{
					(*__it)->event_handle_->handle_output((*__it)->fd_);
				}
				//	exception happened
				else if(FD_ISSET((*__it)->fd_,&excepion_set_))
				{
					(*__it)->event_handle_->handle_exception((*__it)->fd_);
				}
			}
		}
		//	fix bug #20003
		easy::Util::sleep(max_sleep_time_);
	}
	return -1;
}

void Reactor_Impl_Select::broadcast(easy_int32 __fd,const easy_char* __data,easy_uint32 __length)
{
	for (std::vector<Event_Handle_Data*>::iterator __it = events_.begin(); __it != events_.end(); ++__it)
	{
		if(*__it)
		{
			if (0)
			{
				//	except me
				if(__fd == (*__it)->fd_)
				{
					continue;
				}
				else
				{
					(*__it)->event_handle_->write((*__it)->fd_,__data,__length);
				}
			}
			else
			{
				//	all
				(*__it)->event_handle_->write((*__it)->fd_,__data,__length);
			}
		}
	}
}

easy_int32 Reactor_Impl_Select::handle_close( easy_int32 __fd )
{
	for (std::vector<Event_Handle_Data*>::iterator __it = events_.begin(); __it != events_.end(); ++__it)
	{
		if(*__it)
		{
			if(__fd == (*__it)->fd_)
			{
				(*__it)->invalid_fd_ = 0;
				break;
			}
		}
	}
	return -1;
}
