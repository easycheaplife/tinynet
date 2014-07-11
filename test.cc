#include "server_impl.h"
#include "reactor.h"
#include <stdlib.h>
#include <stdio.h>
#ifdef __LINUX
#include "easy_dump.h"
#endif // __LINUX

int main(int __arg_num,char** args)
{
	/*	
	g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_SELECT -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_select.h reactor_impl_select.cc server_impl.h server_impl.cc test.cc 
	g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_EPOLL -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_epoll.h reactor_impl_epoll.cc server_impl.h server_impl.cc test.cc
	g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_POLL -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_poll.h reactor_impl_poll.cc server_impl.h server_impl.cc test.cc
	*/
	if(3 != __arg_num)
	{
		printf("param error,please input correct param! for example: nohup ./transform 192.168.22.63 9876 & \n");
		exit(1);
	}
#ifdef __LINUX
	signal(SIGINT,dump_for_gdb);
	signal(SIGSEGV,dump);
#endif // __LINUX
	char* __host = args[1];
	unsigned int __port = atoi(args[2]);
	Reactor* __reactor = Reactor::instance();
	Server_Impl __event_handle_srv(__reactor,__host,__port);
	static const int __max_time_out = 5000*1000;
	__reactor->event_loop(__max_time_out);
	return 0;
}
