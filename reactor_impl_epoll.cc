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
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "reactor_impl_epoll.h"
#include "event_handle.h"
#include "reactor.h"

Reactor_Impl_Epoll::Reactor_Impl_Epoll()
{
	fd_ = -1;
	fd_epoll_ = -1;
	handle_ = NULL;
	memset(&ev_,0,sizeof(struct epoll_event));
	memset(events_,0,sizeof(struct epoll_event)*MAX_EVENTS);
	_init();
}

easy_int32 Reactor_Impl_Epoll::register_handle(Event_Handle* __handle,easy_int32 __fd,easy_int32 __mask,easy_int32 __connect)
{
	if(kMaskAccept ==__mask)
	{
		fd_ = __fd;
		handle_ = __handle;
		_add_event(__fd,EPOLLIN|EPOLLET);
	}
	else if(kMaskRead ==__mask)
	{
		if(1 == __connect)
		{
			_add_event(__fd,EPOLLIN|EPOLLET);
		}
		else
		{
			_mod_event(__fd,EPOLLIN|EPOLLET);
		}		
	}
	else if(kMaskWrite ==__mask)
	{
		_mod_event(__fd,EPOLLOUT|EPOLLET);
	}
	return -1;
}

easy_int32 Reactor_Impl_Epoll::remove_handle(Event_Handle* __handle,easy_int32 __mask)
{
	return -1;
}

easy_int32 Reactor_Impl_Epoll::handle_event(easy_ulong __millisecond)
{
	return -1;
}

easy_int32 Reactor_Impl_Epoll::event_loop(easy_ulong __millisecond)
{
	while(1)
	{
		easy_int32 __nfds = epoll_wait(fd_epoll_, events_, MAX_EVENTS, -1);
		if( -1 == __nfds )
		{
			  //	for gdb
			  if(EINTR == errno)
			  {
					 perror("signal EINTR received,ignore it!");
					continue;
			  }
		      perror("epoll_wait error");
			  printf("errno=%d\n",errno);
		      exit(EXIT_FAILURE);
		}
		for(easy_int32 __i = 0; __i < __nfds; ++__i)
		{
		      if (events_[__i].data.fd == fd_) 
		      {
			    handle_->handle_input(fd_);
		      }
		      else if(events_[__i].events&EPOLLIN)  
		      {
			  handle_->handle_input(events_[__i].data.fd);
		      }
		      else if(events_[__i].events&EPOLLOUT)  
		      {
			  handle_->handle_output(events_[__i].data.fd);
		      }
		}
	}	
	return -1;
}

void Reactor_Impl_Epoll::_init()
{
	//	The size is not the maximum size of the backing store but just a hint to the  kernel  about  how  to  dimension  internal  structures.
        //	Since Linux 2.6.8, the size argument is unused.
	fd_epoll_ = epoll_create(1024);
	if(-1 == fd_epoll_)
	{
		perror("epoll_create error");
		exit(EXIT_FAILURE);
	}
}

void Reactor_Impl_Epoll::_add_event(easy_int32 __fd,uint32_t __event)
{
	ev_.data.fd = __fd;  
    ev_.events = __event;   
    if( -1 == epoll_ctl(fd_epoll_,EPOLL_CTL_ADD,__fd,&ev_))
	{
		perror("epoll_ctl EPOLL_CTL_ADD");
		exit(EXIT_FAILURE);
	}
}

void Reactor_Impl_Epoll::_mod_event(easy_int32 __fd,uint32_t __event)
{
	ev_.data.fd = __fd;  
    ev_.events = __event;   
    if( -1 == epoll_ctl(fd_epoll_,EPOLL_CTL_MOD,__fd,&ev_))
	{
		perror("epoll_ctl EPOLL_CTL_MOD");
		exit(EXIT_FAILURE);
	}
}

easy_int32 Reactor_Impl_Epoll::handle_close( easy_int32 __fd )
{
	handle_->handle_close(__fd);
	close(__fd);
	return -1;
}

void Reactor_Impl_Epoll::broadcast(easy_int32 __fd,const easy_char* __data,easy_uint32 __length)
{
	//	do nothing,broadcast should be done at layer of logic.
	//	to be continue ...
}

