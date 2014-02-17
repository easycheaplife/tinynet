#include "reactor.h"
#include "event_handle_srv.h"
int main()
{
	/*	
	g++ -g -D__LINUX -D__HAVE_SELECT -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_select.h reactor_impl_select.cc test.cc
	g++ -g -D__LINUX -D__HAVE_EPOLL -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_epoll.h reactor_impl_epoll.cc test.cc
	g++ -g -D__LINUX -D__HAVE_POLL -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_poll.h reactor_impl_poll.cc test.cc
	*/
	Reactor* __reactor = Reactor::instance();
	Event_Handle_Srv __event_handle_srv(__reactor);
	__reactor->event_loop(5000);
	return 0;
}
