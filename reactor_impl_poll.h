#include <poll.h>
#include <string.h>
#include "reactor_impl.h"

#define MAX_POLL_FD 10

class Reactor_Impl_Poll : public Reactor_Impl
{
public:
	Reactor_Impl_Poll();
	
	~Reactor_Impl_Poll() {}
	
	int register_handle(Event_Handle* __handle,int __fd,int __mask,int __connect);
	
	int remove_handle(Event_Handle* __handle,int __mask);
	
	int handle_event(unsigned long __millisecond);

	int handle_close(int __fd);
	
	int event_loop(unsigned long __millisecond);

	//	__fd is the broadcaster
	void broadcast(int __fd,const char* __data,unsigned int __length) {}
private:

	void _add_event(int __fd);
	
	int						fd_;
	
	int						cur_poll_fd_num_;

	struct pollfd 					fd_poll_[MAX_POLL_FD];  
	
	Event_Handle* 					handle_;
};
