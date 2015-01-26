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
#include "easy_util.h"


Client_Impl::Client_Impl( Reactor* __reactor,const easy_char* __host,easy_uint32 __port /*= 9876*/ ) : Event_Handle_Cli(__reactor,__host,__port)
{
	ring_buf_ = new easy::EasyRingbuffer<unsigned char,easy::alloc,easy::mutex_lock>(1024*8);
	//	start read thread
	auto __thread_ = std::thread(CC_CALLBACK_0(Client_Impl::_read_thread,this));
	__thread_.detach();

	//	work thread can work now
	star_work_thread();
}

Client_Impl::~Client_Impl()
{

}

void Client_Impl::on_read( easy_int32 __fd )
{
	if(0)
	{
		easy_char __buf[64*1024] = {0};
		easy_int32 __recv_size = Event_Handle_Cli::read(__fd,__buf,64*1024);
		if(-1 != __recv_size)
		{
			ring_buf_->append((const unsigned char*)__buf,__recv_size);
		}
	}
	else
	{
		//	the follow code is ring_buf's append function actually.
		easy_ulong __usable_size = 0;
		_get_usable(__fd,__usable_size);
		easy_int32 __ring_buf_tail_left = ring_buf_->size() - ring_buf_->wpos();
		if(__usable_size <= __ring_buf_tail_left)
		{
			Event_Handle_Cli::read(__fd,(easy_char*)ring_buf_->buffer() + ring_buf_->wpos(),__usable_size);
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
			easy_int32 __ring_buf_head_left = ring_buf_->rpos();
			easy_int32 __read_left = __usable_size - __ring_buf_tail_left;
			if(__ring_buf_head_left >= __read_left)
			{
				Event_Handle_Cli::read(__fd,(easy_char*)ring_buf_->buffer(),__read_left);
				ring_buf_->set_wpos(__read_left);
			}
			else
			{
				Event_Handle_Cli::read(__fd,(easy_char*)ring_buf_->buffer(),__ring_buf_head_left);
				ring_buf_->set_wpos(__ring_buf_head_left);
			}
		}
	}
}

void Client_Impl::_read_thread()
{
	static const easy_int32 __sleep_time = 1000*100;
	static const easy_int32 __head_size = sizeof(easy_uint32);
	std::string 	 __string_packet;
	while (true)
	{
		easy_uint32 __packet_length = 0;
		if(!ring_buf_->peek((unsigned char*)&__packet_length,__head_size))
		{
			easy::Util::sleep(__sleep_time);
			continue;
		}
		easy_uint16 __real_packet_length = __packet_length & 0x0000ffff;
		easy_uint16 __real_fd = __packet_length >> 16;
		if(!__real_packet_length)
		{
			printf("__packet_length error\n");
			continue;
		}
		__string_packet.clear();
		if(ring_buf_->read(__string_packet,__real_packet_length + __head_size))
		{
			if(is_proxy_client())
			{
				handle_packet(__real_fd,__string_packet.c_str() + __head_size);
			}
			else
			{
				handle_packet(get_handle(),__string_packet.c_str() + __head_size);
			}
		}
		else
		{
			easy::Util::sleep(__sleep_time);
			continue;
		}
		easy::Util::sleep(__sleep_time);
	}
}
