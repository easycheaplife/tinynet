#include "reactor.h"

#ifdef __HAVE_SELECT
#include "reactor_impl_select.h"
#endif //__HAVE_SELECT

#ifdef __HAVE_POLL
#include "reactor_impl_poll.h"
#endif //__HAVE_POLL

#ifdef __HAVE_EPOLL
#include "reactor_impl_epoll.h"
#endif //__HAVE_POLL

Reactor* Reactor::reactor_ = 0;

Reactor::Reactor()
{
#ifdef  __HAVE_SELECT
	reactor_impl_ = new Reactor_Impl_Select();
#endif 
#ifdef  __HAVE_POLL
	reactor_impl_ = new Reactor_Impl_Poll();
#endif
#ifdef  __HAVE_EPOLL
	reactor_impl_ = new Reactor_Impl_Epoll();
#endif
}

Reactor::~Reactor() 
{
	delete reactor_impl_;
}

Reactor* Reactor::instance()
{
	if( 0 ==  Reactor::reactor_)
	{
		Reactor::reactor_ = new Reactor();
	}
	return Reactor::reactor_;
}

int Reactor::register_handle(Event_Handle* __handle,int __fd,int __mask)
{

	return -1;
}

int Reactor::remove_handle(Event_Handle* __handle,int __mask)
{
	return -1;
}

int Reactor::handle_event(unsigned long __millisecond)
{
	
	return -1;
}

int Reactor::event_loop(unsigned long __millisecond)
{
	reactor_impl_->event_loop(__millisecond);
	return -1;
}
