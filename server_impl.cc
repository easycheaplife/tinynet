#include "server_impl.h"
#include <stdio.h>
#include <string.h>

Server_Impl::Server_Impl( Reactor* __reactor,const char* __host /*= "0.0.0.0"*/,unsigned int __port /*= 9876*/ )
	: Event_Handle_Srv(__reactor,__host,__port) 
{

}

void Server_Impl::on_connected( int __fd )
{
	printf("on_connected __fd = %d \n",__fd);
}

void Server_Impl::on_read( int __fd )
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


