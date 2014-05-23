#include "reactor.h"

#if  defined __HAVE_IOCP
#include "reactor_impl_iocp.h"
#elif defined  __HAVE_SELECT || defined WIN32
#include "reactor_impl_select.h"
#elif defined __HAVE_EPOLL
#include "reactor_impl_epoll.h"
#elif defined __HAVE_POLL
#include "reactor_impl_poll.h"
#endif 

Reactor* Reactor::reactor_ = 0;

Reactor::Reactor()
{
#if defined   __HAVE_IOCP
	reactor_impl_ = new Reactor_Impl_Iocp();
#elif defined  __HAVE_SELECT || defined WIN32
	reactor_impl_ = new Reactor_Impl_Select();
#elif defined  __HAVE_EPOLL
	reactor_impl_ = new Reactor_Impl_Epoll();
#elif defined  __HAVE_POLL
	reactor_impl_ = new Reactor_Impl_Poll();
#endif 
}

Reactor::~Reactor() 
{
	if(reactor_impl_)
	{
		delete reactor_impl_;
		reactor_impl_ = NULL;
	}
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

void Reactor::destory()
{
	if(reactor_)
	{
		delete reactor_;
		reactor_ = NULL;
	}
}
