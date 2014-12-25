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
#include "client_impl.h"
#include "easy_byte_buffer.h"
#include "easy_util.h"

Client_Impl::Client_Impl( Reactor* __reactor,const easy_char* __host,easy_uint32 __port /*= 9876*/ ) : Event_Handle_Cli(__reactor,__host,__port)
{
	ring_buf_ = new easy::EasyRingbuffer<easy_uint8,easy::alloc,easy::mutex_lock>(1024*8);
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
		easy_int32 __read_bytes = 0;
		if(__usable_size <= __ring_buf_tail_left)
		{
			__read_bytes = Event_Handle_Cli::read(__fd,(char*)ring_buf_->buffer() + ring_buf_->wpos(),__usable_size);
			if(-1 != __read_bytes && 0 != __read_bytes)
			{
				ring_buf_->set_wpos(ring_buf_->wpos() + __usable_size);
			}
		}
		else
		{
			//	if not do this,the connection will be closed!
			if(0 != __ring_buf_tail_left)
			{
				__read_bytes = Event_Handle_Cli::read(__fd,(easy_char*)ring_buf_->buffer() +  ring_buf_->wpos(),__ring_buf_tail_left);
				if(-1 != __read_bytes && 0 != __read_bytes)
				{
					ring_buf_->set_wpos(ring_buf_->size());
				}
			}
			easy_int32 __ring_buf_head_left = ring_buf_->rpos();
			easy_int32 __read_left = __usable_size - __ring_buf_tail_left;
			if(__ring_buf_head_left >= __read_left)
			{
				__read_bytes = Event_Handle_Cli::read(__fd,(easy_char*)ring_buf_->buffer(),__read_left);
				if(-1 != __read_bytes && 0 != __read_bytes)
				{
					ring_buf_->set_wpos(__read_left);
				}
			}
			else
			{
				//	maybe some problem here when data not recv completed for epoll ET.you can realloc the input buffer or use while(recv) until return EAGAIN.
				__read_bytes = Event_Handle_Cli::read(__fd,(easy_char*)ring_buf_->buffer(),__ring_buf_head_left);
				if(-1 != __read_bytes && 0 != __read_bytes)
				{
					ring_buf_->set_wpos(__ring_buf_head_left);
				}
			}
		}
	}
}

void Client_Impl::_read_thread()
{
	const easy_int32 __head_size = sizeof(easy_uint16);
	const easy_int32 __recv_buffer_size = 1024*8;
	easy_char __read_buf[__recv_buffer_size] = {};
	while (true)
	{
		while (!ring_buf_->read_finish())
		{
			easy_uint16 __packet_length = 0;
			if(!ring_buf_->peek((easy_uint8*)&__packet_length,__head_size))
			{
				break;
			}
			if(!__packet_length)
			{
				break;
			}
			memset(__read_buf,0,__recv_buffer_size);
			if(ring_buf_->read((unsigned char*)__read_buf,__packet_length + __head_size))
			{
				printf("data send: %s\n",__read_buf + __head_size);
				if (0)
				{
					Event_Handle_Cli::write(__read_buf,__packet_length + __head_size);
				}
				else
				{
					handle_packet(__read_buf,__packet_length + __head_size);
				}
				
			}
			else
			{
				break;
			}
		}
		static const easy_int32 __sleep_time = 1*1000;
		easy::Util::sleep(__sleep_time);
	}
}
