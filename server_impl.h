#ifndef server_impl_h__
#define server_impl_h__
#include "event_handle_srv.h"

class Reactor;

class Server_Impl : public Event_Handle_Srv
{
public:
	Server_Impl(Reactor* __reactor,const char* __host = "0.0.0.0",unsigned int __port = 9876);

	~Server_Impl() {}

	void on_connected(int __fd);

	void on_read(int __fd);
};

#endif // server_impl_h__
