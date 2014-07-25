#ifndef client_impl_h__
#define client_impl_h__
#include "event_handle_cli.h"
#include "easy_ring_buffer.h"
#include "easy_allocator.h"

class Client_Impl : public Event_Handle_Cli
{
public:
	Client_Impl(Reactor* __reactor,const char* __host,unsigned int __port = 9876);

	~Client_Impl();

	void on_read(int __fd);

private:
	void	_read_thread();

private:
	easy::EasyRingbuffer<unsigned char,easy::alloc>* ring_buf_;

};

#endif // client_impl_h__
