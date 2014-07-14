#ifndef server_impl_h__
#define server_impl_h__
#include <map>
#include <vector>
#include <list>
#include "event_handle_srv.h"
//	the follows files can get from git@github.com:yuyunliuhen/easy.git,make the easy project at the same directory.
#include "easy_ring_buffer.h"
#include "easy_allocator.h"
#include "easy_locked_queue.h"

#define VERSION	1.0.1

#ifndef __USE_CONNECTS_COPY
#define __USE_CONNECTS_COPY
#endif //__USE_CONNECTS_COPY

//	class forward declaration
class Reactor;
struct Buffer; 

struct Buffer
{
	typedef easy::EasyRingbuffer<unsigned char,easy::alloc>	ring_buffer;
	static const size_t MAX_POOL_SIZE = 50000;
	typedef  int _Key;

	ring_buffer*	input_;
	ring_buffer*	output_;
	int				fd_;
	int				invalid_fd_;
	Buffer(int __fd,unsigned int __max_buffer_size)
	{
		input_ = new easy::EasyRingbuffer<unsigned char,easy::alloc>(__max_buffer_size);
		output_ = new easy::EasyRingbuffer<unsigned char,easy::alloc>(__max_buffer_size);
		fd_ = __fd;
		invalid_fd_ = 1;
	}
	void init(int __fd,unsigned int __max_buffer_size)
	{
		input_->reset();
		output_->reset();
		fd_ = __fd;
		invalid_fd_ = 1;
	}

	void clear()
	{
		if(input_)
		{
			delete input_;
			input_ = NULL;
		}
		if(output_)
		{
			delete output_;
			output_ = NULL;
		}
	}
};

class Server_Impl : public Event_Handle_Srv
{
public:
	Server_Impl(Reactor* __reactor,const char* __host = "0.0.0.0",unsigned int __port = 9876);

	~Server_Impl();

	void on_connected(int __fd);

	void on_disconnect(int __fd);

	void on_read(int __fd);

private:
	void _read_directly(int __fd);

	void _read(int __fd);

	void _read_thread();

	void _write_thread();

	void _disconnect(Buffer* __buffer);

private:
	typedef	std::map<int,Buffer*>	map_buffer;
	std::map<int,Buffer*>			connects_;

#ifdef __USE_CONNECTS_COPY
	typedef	std::vector<Buffer*>	vector_buffer;
	std::vector<Buffer*>			connects_copy;
#endif //__USE_CONNECTS_COPY

	static const unsigned int		max_buffer_size_;

	static const unsigned int		max_sleep_time_;

	easy::mutex_lock				lock_;

	easy::lock_queue<Buffer,easy::mutex_lock,std::list<Buffer*> >	buffer_queue_;
};

#endif // server_impl_h__
