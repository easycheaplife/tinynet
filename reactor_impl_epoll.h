#include <sys/epoll.h>
#include "reactor_impl.h"

#define MAX_EVENTS 10

class Reactor_Impl_Epoll : public Reactor_Impl
{
public:
	Reactor_Impl_Epoll();
	
	~Reactor_Impl_Epoll() {}
	
	int register_handle(Event_Handle* __handle,int __fd,int __mask,int __connect);
	
	int remove_handle(Event_Handle* __handle,int __mask);
	
	int handle_event(unsigned long __millisecond);
	
	int event_loop(unsigned long __millisecond);
private:
	void _init();
	
	void _add_event(int __fd,uint32_t __event);
	
	void _mod_event(int __fd,uint32_t __event);
	
	int						fd_;

	int						fd_epoll_;
	
	struct epoll_event 				ev_;
	
	struct epoll_event				events_[MAX_EVENTS];
	
	Event_Handle* 					handle_;
};
