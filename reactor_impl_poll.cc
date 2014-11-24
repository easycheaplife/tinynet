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
#include "reactor_impl_poll.h"
#include "event_handle.h"
#include "reactor.h"

Reactor_Impl_Poll::Reactor_Impl_Poll()
{
	fd_ = -1;
	cur_poll_fd_num_ = 0;
	handle_ = NULL;
	memset(fd_poll_,-1,sizeof(struct pollfd)*MAX_POLL_FD);
}

easy_int32 Reactor_Impl_Poll::register_handle(Event_Handle* __handle,easy_int32 __fd,easy_int32 __mask,easy_int32 __connect)
{
	if(kMaskAccept ==__mask)
	{
		fd_ = __fd;
		handle_ = __handle;
		_add_event(__fd);
	}
	else if(kMaskRead ==__mask)
	{
		if(1 == __connect)
		{
			_add_event(__fd);
		}		
	}
	else if(kMaskWrite ==__mask)
	{
		
	}
	return -1;
}

easy_int32 Reactor_Impl_Poll::remove_handle(Event_Handle* __handle,easy_int32 __mask)
{
	return -1;
}

easy_int32 Reactor_Impl_Poll::handle_event(easy_ulong __millisecond)
{
	return -1;
}

easy_int32 Reactor_Impl_Poll::event_loop(easy_ulong __millisecond)
{
	while(1)
	{
		easy_int32 __positive_num = poll(fd_poll_,cur_poll_fd_num_,-1);
		if( -1 == __positive_num )
		{
			perror("poll");
			exit(1);
		}
		else if(0 == __positive_num)
		{
			perror("timeout");
			continue;
		}
		for(int __i = 0; __i < cur_poll_fd_num_; ++__i)
		{
			if(fd_poll_[__i].revents & POLLIN)
			{
				if(fd_ == fd_poll_[__i].fd)
				{
					handle_->handle_input(fd_poll_[__i].fd);
				}
				else
				{
					handle_->handle_output(fd_poll_[__i].fd);
				}
			}
		}
	}	
	return -1;
}

void Reactor_Impl_Poll::_add_event(easy_int32 __fd)
{
	fd_poll_[cur_poll_fd_num_].fd = __fd;
	fd_poll_[cur_poll_fd_num_].revents = POLLIN;
	++cur_poll_fd_num_;	
}

easy_int32 Reactor_Impl_Poll::handle_close( easy_int32 __fd )
{
	return -1;
}

