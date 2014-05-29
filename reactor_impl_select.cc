#include <stdlib.h>
#include <stdio.h>
#include "reactor_impl_select.h"
#include "event_handle.h"

struct Event_Handle_Data
{
	Event_Handle_Data(int __fd,Event_Handle* __event_handle):fd_(__fd),invalid_fd_(1),event_handle_(__event_handle) {}
	int				invalid_fd_;
	int				fd_;
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
int Reactor_Impl_Select::register_handle(Event_Handle* __handle,int __fd,int __mask,int __connect)
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
#ifdef __USE_STD_MAP
			events_.insert(std::map<int,Event_Handle*>::value_type(__fd,__handle));
#else
			events_.push_back(new Event_Handle_Data(__fd,__handle));
#endif // __USE_STD_MAP
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
	while(true)
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
#ifdef __USE_STD_MAP
		for(std::map<int,Event_Handle*>::iterator __it = events_.begin(); __it != events_.end(); ++__it)
		{
			FD_SET(__it->first,&read_set_);  
			FD_SET(__it->first,&write_set_);  
			FD_SET(__it->first,&excepion_set_);  
		}
#else
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
#ifdef __LINUX
					close((*__it)->fd_);
#else
					closesocket((*__it)->fd_);
#endif //__LINUX
					
					delete (*__it);
					(*__it) = NULL;
					__it = events_.erase(__it);
				}
			}
		}
#endif // __USE_STD_MAP

		//	you must set max_fd_ is max use fd under unix/linux system, if not,part of fd will not be detected.
		//	if write_set_ is not null, that means the write status will be watched to see. 
		int __ret = select(max_fd_ + 1,&read_set_,/*&write_set_*/NULL,&excepion_set_,&__tv);
		if ( -1 == __ret )
		{
			perror("error at select");
#ifndef __LINUX
			DWORD __last_error = ::WSAGetLastError();
			//	usually some socket is closed, such as closesocket called. it maybe exist a invalid socket.
			if(WSAENOTSOCK == __last_error)
			{
				
			}
#endif // __LINUX
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
#ifdef __USE_STD_MAP
		for(std::map<int,Event_Handle*>::iterator __it = events_.begin(); __it != events_.end(); )
		{
			//	something to be read
			if(FD_ISSET(__it->first,&read_set_))
			{
				if(__it->second->handle_input(__it->first))
				{
					//	error happened, disconnect the socket
					events_.erase(__it++);
				}
				else
				{
					++__it;
				}
			}
			//	something to be write
			else if(FD_ISSET(__it->first,&write_set_))
			{
				__it->second->handle_output(__it->first);
				++__it;
			}
			//	exception happened
			else if(FD_ISSET(__it->first,&excepion_set_))
			{
				__it->second->handle_exception(__it->first);
				++__it;
			}
		}
#else
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
#endif // __USE_STD_MAP
	}
	return -1;
}

void Reactor_Impl_Select::broadcast(int __fd,const char* __data,unsigned int __length)
{
#ifdef __USE_STD_MAP
	for(std::map<int,Event_Handle*>::iterator __it = events_.begin(); __it != events_.end(); ++__it)
	{
		if(__fd == __it->first)
		{
			continue;
		}
		else
		{
			write(__it->first,__data,__length);
		}
	}
#else
	for (std::vector<Event_Handle_Data*>::iterator __it = events_.begin(); __it != events_.end(); ++__it)
	{
		if(*__it)
		{
			if(__fd == (*__it)->fd_)
			{
				continue;
			}
			else
			{
				write((*__it)->fd_,__data,__length);
			}
		}
	}
#endif // __USE_STD_MAP

}

void Reactor_Impl_Select::write( int __fd,const char* __data, int __length )
{
	int __send_bytes = send(__fd,__data,__length,0);
	if(-1 == __send_bytes)
	{
#ifndef __LINUX
		DWORD __last_error = ::GetLastError();
		if(WSAEWOULDBLOCK  == __last_error || WSAECONNRESET == __last_error)
		{
			//	close peer socket
			//	closesocket(__fd);	//	don't do this, it will make server shutdown for select error 10038(WSAENOTSOCK)
			handle_close(__fd);
			return;
		}
#else
		//error happend but EAGAIN and EWOULDBLOCK meams that peer socket have been close
		//EWOULDBLOCK means messages are available at the socket and O_NONBLOCK  is set on the socket's file descriptor
		// ECONNRESET means an existing connection was forcibly closed by the remote host
		if((EAGAIN == errno && EWOULDBLOCK == errno) || ECONNRESET == errno)
		{
			//	close peer socket
			handle_close(__fd);
			return;
		}
#endif // __LINUX
		perror("error at send");  
	}
}

int Reactor_Impl_Select::handle_close( int __fd )
{
#ifdef __USE_STD_MAP
	//	warning:do this will be dangous, it causes iterator failed!
	for(std::map<int,Event_Handle*>::iterator __it = events_.begin(); __it != events_.end(); ++__it)
	{
		if(__fd == __it->first)
		{
			events_.erase(__it);
		}
	}
#else
	for (std::vector<Event_Handle_Data*>::iterator __it = events_.begin(); __it != events_.end(); ++__it)
	{
		if(*__it)
		{
			if(__fd == (*__it)->fd_)
			{
				(*__it)->invalid_fd_ = 0;
			}
		}
	}
#endif // __USE_STD_MAP
	return -1;
}
