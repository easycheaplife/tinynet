#ifndef server_impl_h__
#define server_impl_h__
#include <map>
#include <vector>
#include "event_handle_srv.h"
//	the follows files can get from git@github.com:yuyunliuhen/easy.git,make the easy project at the same directory.
#include "easy_ring_buffer.h"
#include "easy_allocator.h"

#define VERSION	1.0.1

#ifndef __USE_CONNECTS_COPY
#define __USE_CONNECTS_COPY
#endif //__USE_CONNECTS_COPY

//	class forward declaration
class Reactor;
struct Buffer; 

class Server_Impl : public Event_Handle_Srv
{
public:
	Server_Impl(Reactor* __reactor,const char* __host = "0.0.0.0",unsigned int __port = 9876);

	~Server_Impl() {}

	void on_connected(int __fd);

	void on_disconnect(int __fd);

	void on_read(int __fd);

private:
	void _read_directly(int __fd);

	void _read(int __fd);

	void _read_thread();

	void _write_thread();

private:
	std::map<int,Buffer*>			connects_;

#ifdef __USE_CONNECTS_COPY
	std::vector<Buffer*>			connects_copy;
#endif //__USE_CONNECTS_COPY

	static const unsigned int		max_buffer_size_;

	static const unsigned int		max_sleep_time_;

	easy::mutex_lock				lock_;
};

#endif // server_impl_h__
