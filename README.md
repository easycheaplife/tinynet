2014-02-17
1 add win32 solution support way of select running at windows.


2014-05-23
1 add client socket and fix some bugs.



usage:
#include "reactor.h"
#include "event_handle_srv.h"
#include <stdio.h>

class Server_Impl : public Event_Handle_Srv
{
public:
	Server_Impl(Reactor* __reactor) : Event_Handle_Srv(__reactor) {}

	~Server_Impl() {}

	void on_connected(int __fd) { printf("on_connected __fd = %d \n",__fd);}

	void on_read(int __fd,const char* __data,unsigned int __length) { printf("on_read data is %s,length is %d\n",__data,__length);}
};

int main()
{
	Reactor* __reactor = Reactor::instance();
	Server_Impl __event_handle_srv(__reactor);
	__reactor->event_loop(5000);
	return 0;
}
