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

const easy_uint32 Client_Impl::max_buffer_size_ = 1024*8;
const easy_uint32 Client_Impl::max_sleep_time_ = 1000*10;
Client_Impl::Client_Impl( Reactor* __reactor,const easy_char* __host,easy_uint32 __port /*= 9876*/ ) : Event_Handle_Cli(__reactor,__host,__port)
{
	ring_buf_ = new easy::EasyRingbuffer<unsigned char,easy::alloc,easy::mutex_lock>(max_buffer_size_);
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
#if 1
	//	the follow code is ring_buf's append function actually.
	easy_ulong __usable_size = 0;
	_get_usable(__fd,__usable_size);
	easy_int32 __ring_buf_tail_left = ring_buf_->size() - ring_buf_->wpos();
	easy_int32 __read_bytes = 0;
	if(__usable_size <= __ring_buf_tail_left)
	{
		__read_bytes = Event_Handle_Cli::read(__fd,(easy_char*)ring_buf_->buffer() + ring_buf_->wpos(),__usable_size);
		if(-1 != __read_bytes && 0 != __read_bytes)
		{
			ring_buf_->set_wpos(ring_buf_->wpos() + __read_bytes);
		}
	}
	else
	{
		__read_bytes = Event_Handle_Cli::read(__fd,(char*)ring_buf_->buffer() +  ring_buf_->wpos(),__ring_buf_tail_left);
		if(-1 != __read_bytes && 0 != __read_bytes)
		{
			ring_buf_->set_wpos(ring_buf_->wpos() + __read_bytes);
		}
		easy_int32 __ring_buf_head_left = ring_buf_->rpos();
		easy_int32 __read_left = __usable_size - __ring_buf_tail_left;
		if(__ring_buf_head_left > __read_left)
		{
			__read_bytes = Event_Handle_Cli::read(__fd,(easy_char*)ring_buf_->buffer(),__read_left);
			if(-1 != __read_bytes && 0 != __read_bytes)
			{
				ring_buf_->set_wpos(__read_bytes);
			}
		}
		else
		{
			__read_bytes = Event_Handle_Cli::read(__fd,(easy_char*)ring_buf_->buffer(),__ring_buf_head_left);
			if(-1 != __read_bytes && 0 != __read_bytes)
			{
				ring_buf_->set_wpos(__read_bytes);
			}
			//	not read completely from system cache, read next read event.
		}
	}
#else
	_read_directly(__fd);
#endif
}

void Client_Impl::_read_thread()
{
	static const easy_int32 __head_size = sizeof(easy_uint32);
	std::string 	 __string_packet;
	while (true)
	{
		easy_uint32 __packet_length = 0;
		if(!ring_buf_->peek((unsigned char*)&__packet_length,__head_size))
		{
			easy::Util::sleep(max_sleep_time_);
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
				handle_packet(__real_fd,__string_packet.c_str() + __head_size,easy_null);
			}
			else
			{
				handle_packet(get_handle(),__string_packet.c_str() + __head_size,easy_null);
			}
		}
		else
		{
			easy::Util::sleep(max_sleep_time_);
			continue;
		}
		easy::Util::sleep(max_sleep_time_);
	}
}

void Client_Impl::_read_directly(easy_int32 __fd)
{
	easy_uint32 __packet_length = 0;
	easy_uint8  __buf[max_buffer_size_] = {};
	static const easy_int32 __head_size = sizeof(easy_uint32);
	while (true)
	{
		__packet_length = 0;
		easy_int32 __read_bytes = Event_Handle_Cli::read(__fd,(easy_char*)&__packet_length,__head_size,MSG_PEEK);
		if( __head_size != __read_bytes)
		{
			break;
		}
		easy_uint16 __real_packet_length = __packet_length & 0x0000ffff;
		easy_uint16 __real_fd = __packet_length >> 16;
		if(!__real_packet_length)
		{
			printf("__packet_length error\n");
			break;
		}
		__read_bytes = Event_Handle_Cli::read(__fd,(easy_char*)__buf,__real_packet_length,MSG_PEEK);
		if (__read_bytes < __real_packet_length)
		{
			break;
		}
		memset(__buf,0,max_buffer_size_);
		std::string __string_packet;
		__read_bytes = Event_Handle_Cli::read(__fd,(easy_char*)__buf,__real_packet_length + __head_size);
		if(-1 != __read_bytes && 0 != __read_bytes)
		{
			__string_packet.insert(0,(const char*)__buf,__real_packet_length + __head_size);
			if(is_proxy_client())
			{
				handle_packet(__real_fd,__string_packet.c_str() + __head_size,easy_null);
			}
			else
			{
				handle_packet(get_handle(),__string_packet.c_str() + __head_size,easy_null);
			}
		}
		easy::Util::sleep(max_sleep_time_);
	}
}
