#ifndef __LINUX
#include <WinSock.h>
#else
#include <sys/select.h>
#endif //__LINUX

#include <stdio.h>
#include <map>
#include "reactor_impl.h"

class Reactor_Impl_Select : public Reactor_Impl
{
public:
	Reactor_Impl_Select();
	
	~Reactor_Impl_Select() {}
	
	int register_handle(Event_Handle* __handle,int __fd,int __mask,int __connect);
	
	int remove_handle(Event_Handle* __handle,int __mask);
	
	int handle_event(unsigned long __millisecond);
	
	int event_loop(unsigned long __millisecond);
private:
	fd_set 							read_set_;
	
	fd_set 							write_set_;
	
	fd_set 							excepion_set_;
	
	int								fd_;
	
	int								max_fd_;
	
	Event_Handle* 					handle_;
	
	std::map<int,Event_Handle*> 	events_;
};
