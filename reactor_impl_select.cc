#include <stdlib.h>
#include "reactor_impl_select.h"
#include "event_handle.h"

Reactor_Impl_Select::Reactor_Impl_Select()
{
	FD_ZERO(&read_set_);  
	FD_ZERO(&write_set_);  
	FD_ZERO(&excepion_set_); 
	handle_ = NULL;
	fd_ = -1;
	max_fd_ = -1;
}
int Reactor_Impl_Select::register_handle(Event_Handle* __handle,int __fd,int __mask,int __connect)
{
	if(kMaskAccept ==__mask)
	{
		fd_ = __fd;
		handle_ = __handle;
	}
	else
	{
		if(1 == __connect)
		{
			events_.insert(std::map<int,Event_Handle*>::value_type(__fd,__handle));
		}
	}
	if(max_fd_ < __fd)
	{
		max_fd_ = __fd;
	}
	return -1;
}
int Reactor_Impl_Select::remove_handle(Event_Handle* __handle,int __mask)
{
	return -1;
}
int Reactor_Impl_Select::handle_event(unsigned long __millisecond)
{
	return -1;
}
int Reactor_Impl_Select::event_loop(unsigned long __millisecond)
{
	while(1)
	{
		FD_ZERO(&read_set_);  
		FD_ZERO(&write_set_);  
		FD_ZERO(&excepion_set_); 
		FD_SET(fd_,&read_set_); 
		FD_SET(fd_,&write_set_); 
		FD_SET(fd_,&excepion_set_); 
		struct timeval __tv;  
		__tv.tv_sec = 0;  
		__tv.tv_usec = __millisecond;
		for(std::map<int,Event_Handle*>::iterator __it = events_.begin(); __it != events_.end(); ++__it)
		{
			FD_SET(__it->first,&read_set_);  
			FD_SET(__it->first,&write_set_);  
			FD_SET(__it->first,&excepion_set_);  
		}
		//	you must set max_fd_ is max use fd under unix/linux system, if not,part of fd will not be detected.
		int __ret = select(max_fd_ + 1,&read_set_,&write_set_,&excepion_set_,/*&__tv*/NULL);
		if ( -1 == __ret )
		{
			perror("error at select");
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
		for(std::map<int,Event_Handle*>::iterator __it = events_.begin(); __it != events_.end(); ++__it)
		{
			if(FD_ISSET(__it->first,&read_set_))
			{
				__it->second->handle_input(__it->first);
			}
			else if(FD_ISSET(__it->first,&write_set_))
			{
				__it->second->handle_output(__it->first);
			}
			else if(FD_ISSET(__it->first,&excepion_set_))
			{
				__it->second->handle_exception(__it->first);
			}
		}
	}
	return -1;
}
