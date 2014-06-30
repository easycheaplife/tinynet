#include "reactor.h"
#include "event_handle_srv.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
class Server_Impl : public Event_Handle_Srv
{
public:
	Server_Impl(Reactor* __reactor,const char* __host = "0.0.0.0",unsigned int __port = 9876) : Event_Handle_Srv(__reactor,__host,__port) {}

	~Server_Impl() {}

	void on_connected(int __fd) { printf("on_connected __fd = %d \n",__fd);}

	void on_read(int __fd) 
	{
		static const int __recv_buf_size = 1024;
		if (0)
		{
			//	just transform data
			char __buf[__recv_buf_size] = {0};
			int __recv_size = Event_Handle_Srv::read(__fd,__buf,__recv_buf_size);
			if (0)
			{
				printf("on_read data is %s,length is %d\n",__buf,__recv_size);
			}
			if(-1 != __recv_size)
			{
				write(__fd,__buf,__recv_size);
			}
		}
		else
		{
			//	read head first.and then read the other msg.just a test code
			static const int __head_size = 12;
			unsigned long __usable_size = 0;
			int __packet_length = 0;
			int __log_level = 0;
			int __frame_number = 0;
			int __head = 0;
			unsigned int __guid = 0;
			unsigned char __packet_head[__head_size] = {};
			int __recv_size = 0;
			while (true)
			{
				_get_usable(__fd,__usable_size);
				if(__usable_size >= __head_size)
				{
					__recv_size = Event_Handle_Srv::read(__fd,(char*)&__packet_head,__head_size);
					if(__head_size != __recv_size)
					{
						printf("error: __recv_size = %d",__recv_size);  
						return ;
					}
					memcpy(&__packet_length,__packet_head,4);
					memcpy(&__head,__packet_head + 4,4);
					memcpy(&__guid,__packet_head + 8,4);
					__log_level = (__head) & 0x000000ff;
					__frame_number = (__head >> 8);
					write(__fd,(char*)__packet_head,__recv_size);
				}
				else
				{
					return;
				}
				_get_usable(__fd,__usable_size);

				if(__usable_size >= __packet_length)
				{
					char __buf[__recv_buf_size] = {0};
					int __recv_size = Event_Handle_Srv::read(__fd,__buf,__packet_length);
					write(__fd,__buf,__recv_size);
				}
				else
				{
					return;
				}
			}
		}
	}
};

int main(int __arg_num,char** args)
{
	/*	
	g++ -g -D__LINUX -D__HAVE_SELECT -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_select.h reactor_impl_select.cc test.cc
	g++ -g -D__LINUX -D__HAVE_EPOLL -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_epoll.h reactor_impl_epoll.cc test.cc
	g++ -g -D__LINUX -D__HAVE_POLL -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_poll.h reactor_impl_poll.cc test.cc
	*/
	if(3 != __arg_num)
	{
		printf("param error,please input correct param! for example: nohup ./transform 192.168.22.63 9876 & \n");
		exit(1);
	}
	char* __host = args[1];
	unsigned int __port = atoi(args[2]);
	Reactor* __reactor = Reactor::instance();
	Server_Impl __event_handle_srv(__reactor,__host,__port);
	__reactor->event_loop(5000*1000);
	return 0;
}
