#include <stdlib.h>
#include <stdio.h>
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

int Reactor_Impl_Epoll::register_handle(Event_Handle* __handle,int __fd,int __mask,int __connect)
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

int Reactor_Impl_Epoll::remove_handle(Event_Handle* __handle,int __mask)
{
	return -1;
}

int Reactor_Impl_Epoll::handle_event(unsigned long __millisecond)
{
	return -1;
}

int Reactor_Impl_Epoll::event_loop(unsigned long __millisecond)
{
	while(1)
	{
		int __nfds = epoll_wait(fd_epoll_, events_, MAX_EVENTS, -1);
		if( -1 == __nfds )
		{
		      perror("epoll_wait error");
		      exit(EXIT_FAILURE);
		}
		for(int __i = 0; __i < __nfds; ++__i)
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

void Reactor_Impl_Epoll::_add_event(int __fd,uint32_t __event)
{
	ev_.data.fd = __fd;  
    	ev_.events = __event;   
    	if( -1 == epoll_ctl(fd_epoll_,EPOLL_CTL_ADD,__fd,&ev_))
	{
		perror("epoll_ctl EPOLL_CTL_ADD");
		exit(EXIT_FAILURE);
	}
}

void Reactor_Impl_Epoll::_mod_event(int __fd,uint32_t __event)
{
	ev_.data.fd = __fd;  
    	ev_.events = __event;   
    	if( -1 == epoll_ctl(fd_epoll_,EPOLL_CTL_MOD,__fd,&ev_))
	{
		perror("epoll_ctl EPOLL_CTL_MOD");
		exit(EXIT_FAILURE);
	}
}

int Reactor_Impl_Epoll::handle_close( int __fd )
{
	close(__fd);
	return -1;
}

void Reactor_Impl_Epoll::broadcast(int __fd,const char* __data,unsigned int __length)
{
	//	do nothing,broadcast should be done at layer of logic.
}
