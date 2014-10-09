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
 ****************************************************************************/
#include <thread>
#include "client_protobuf_impl.h"
#include "easy_byte_buffer.h"

Client_Impl::Client_Impl( Reactor* __reactor,const char* __host,unsigned int __port /*= 9876*/ ) : Event_Handle_Cli(__reactor,__host,__port)
{
	ring_buf_ = new easy::EasyRingbuffer<unsigned char,easy::alloc>(1024*64*100);
	//	start read thread
	auto __thread_ = std::thread(CC_CALLBACK_0(Client_Impl::_read_thread,this));
	__thread_.detach();

	//	work thread can work now
	star_work_thread();
}

Client_Impl::~Client_Impl()
{

}

void Client_Impl::on_read( int __fd )
{
	if(0)
	{
		char __buf[64*1024] = {0};
		int __recv_size = Event_Handle_Cli::read(__fd,__buf,64*1024);
		if(-1 != __recv_size)
		{
			ring_buf_->append((const unsigned char*)__buf,__recv_size);
		}
	}
	else
	{
		//	the follow code is ring_buf's append function actually.
		unsigned long __usable_size = 0;
		_get_usable(__fd,__usable_size);
		int __ring_buf_tail_left = ring_buf_->size() - ring_buf_->wpos();
		if(__usable_size <= __ring_buf_tail_left)
		{
			Event_Handle_Cli::read(__fd,(char*)ring_buf_->buffer() + ring_buf_->wpos(),__usable_size);
			ring_buf_->set_wpos(ring_buf_->wpos() + __usable_size);
		}
		else
		{
			//	if not do this,the connection will be closed!
			if(0 != __ring_buf_tail_left)
			{
				Event_Handle_Cli::read(__fd,(char*)ring_buf_->buffer() +  ring_buf_->wpos(),__ring_buf_tail_left);
				ring_buf_->set_wpos(ring_buf_->size());
			}
			int __ring_buf_head_left = ring_buf_->rpos();
			int __read_left = __usable_size - __ring_buf_tail_left;
			if(__ring_buf_head_left >= __read_left)
			{
				Event_Handle_Cli::read(__fd,(char*)ring_buf_->buffer(),__read_left);
				ring_buf_->set_wpos(__read_left);
			}
			else
			{
				Event_Handle_Cli::read(__fd,(char*)ring_buf_->buffer(),__ring_buf_head_left);
				ring_buf_->set_wpos(__ring_buf_head_left);
			}
		}
	}
}

void Client_Impl::_read_thread()
{
	const int __head_size = 12;
	const int __recv_buffer_size = 1024;
	while (true)
	{
		int __packet_length = 0;
		int __log_level = 0;
		int __frame_number = 0;
		unsigned char __packet_head[__head_size] = {};
		int __head = 0;
		unsigned int __guid = 0;
		std::string __device_name;
		if(!ring_buf_->pre_read((unsigned char*)&__packet_head,__head_size))
		{
			continue;
		}
		memcpy(&__packet_length,__packet_head,4);
		memcpy(&__head,__packet_head + 4,4);
		memcpy(&__guid,__packet_head + 8,4);
		if(!__packet_length)
		{
			continue;
		}
		__log_level = (__head) & 0x000000ff;
		__frame_number = (__head >> 8);
		char __read_buf[__recv_buffer_size] = {};
		if(!ring_buf_->read((unsigned char*)__read_buf,__packet_length + __head_size))
		{
			continue;
		}
		printf("data send %s\n",__read_buf + __head_size);
		Event_Handle_Cli::write(__read_buf,__packet_length + __head_size);
	}
}
