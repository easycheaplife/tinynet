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

int Reactor_Impl_Poll::register_handle(Event_Handle* __handle,int __fd,int __mask,int __connect)
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

int Reactor_Impl_Poll::remove_handle(Event_Handle* __handle,int __mask)
{
	return -1;
}

int Reactor_Impl_Poll::handle_event(unsigned long __millisecond)
{
	return -1;
}

int Reactor_Impl_Poll::event_loop(unsigned long __millisecond)
{
	while(1)
	{
		int __positive_num = poll(fd_poll_,cur_poll_fd_num_,-1);
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

void Reactor_Impl_Poll::_add_event(int __fd)
{
	fd_poll_[cur_poll_fd_num_].fd = __fd;
	fd_poll_[cur_poll_fd_num_].revents = POLLIN;
	++cur_poll_fd_num_;	
}
