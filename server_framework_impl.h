/****************************************************************************
 Copyright (c) 2013-2014 King Lee

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
  protobuf version:V2.5.0
  general:
	$export LD_LIBRARY_PATH=$LD_LIBRARY_PATH../easy/dep/protobuf/src/.libs
	$../easy/dep/protobuf/src/.libs/protoc -I./ --cpp_out=. transfer.proto
  compile:
	$g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_EPOLL -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_epoll.h reactor_impl_epoll.cc transfer.pb.h transfer.pb.cc server_framework_impl.h server_framework_impl.cc test.cc -I../easy/src/base -I../easy/dep/protobuf/src/ -L../easy/dep/protobuf/src/.libs -lprotobuf
  run: 
    $./test 192.168.22.61 9876
 ****************************************************************************/
#ifndef server_framework_impl_h__
#define server_framework_impl_h__
#include <map>
#include <vector>
#include <list>
#include "event_handle_srv.h"
//	the follows files can get from git@github.com:yuyunliuhen/easy.git,make the easy project at the same directory.
#include "easy_ring_buffer.h"
#include "easy_allocator.h"
#include "easy_lock.h"
#include "easy_locked_queue.h"

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
	Server_Impl(Reactor* __reactor,const char* __host,unsigned int __port);

	virtual ~Server_Impl();

	void on_connected(int __fd);

	void on_disconnect(int __fd);

	void on_read(int __fd);

	virtual int handle_packet(int __fd,unsigned int __packet_id,const std::string& __string_packet) = 0;

private:
	void _read(int __fd);
	
	void _read_completely(int __fd);

	void _read_thread();

	void _write_thread();

	void _disconnect(Buffer* __buffer);

private:
	typedef	std::map<int,Buffer*>	map_buffer;
	std::map<int,Buffer*>			connects_;

	typedef	std::vector<Buffer*>	vector_buffer;
	std::vector<Buffer*>			connects_copy;

	static const unsigned int		max_buffer_size_;

	static const unsigned int		max_sleep_time_;

	easy::mutex_lock				lock_;

	easy::lock_queue<Buffer,easy::mutex_lock,std::list<Buffer*> >	buffer_queue_;
};

#endif // server_framework_impl_h__
