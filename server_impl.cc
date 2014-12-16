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
#include <stdio.h>
#include <string.h>
#include <thread>
#ifdef __LINUX
#include <unistd.h>
#include <sys/time.h>
#endif // __LINUX
#include "server_impl.h"
#include "easy_util.h"

#define CC_CALLBACK_0(__selector__,__target__, ...) std::bind(&__selector__,__target__, ##__VA_ARGS__)

const easy_uint32 Server_Impl::max_buffer_size_ = 1024*8;
const easy_uint32 Server_Impl::max_sleep_time_ = 1000*500;

Server_Impl::Server_Impl( Reactor* __reactor,const easy_char* __host /*= "0.0.0.0"*/,easy_uint32 __port /*= 9876*/ )
	: Event_Handle_Srv(__reactor,__host,__port) 
{
#ifndef __HAVE_IOCP
	auto __thread_read = std::thread(CC_CALLBACK_0(Server_Impl::_read_thread,this));
	__thread_read.detach();
	auto __thread_write = std::thread(CC_CALLBACK_0(Server_Impl::_write_thread,this));
	__thread_write.detach();
#endif // !__HAVE_IOCP
}

void Server_Impl::on_connected( easy_int32 __fd )
{
	printf("on_connected __fd = %d \n",__fd);
	lock_.acquire_lock();
	connects_[__fd] = buffer_queue_.allocate(__fd,max_buffer_size_);
	connects_copy.push_back(connects_[__fd]);
	lock_.release_lock();
}

void Server_Impl::on_read( easy_int32 __fd )
{
	_read_completely(__fd);
}

void Server_Impl::_read_completely(easy_int32 __fd)
{
	//	the follow code is ring_buf's append function actually.
	easy_ulong __usable_size = 0;
	if(!connects_[__fd])
	{
		return;
	}
	Buffer::ring_buffer* __input = connects_[__fd]->input_;
	if(!__input)
	{
		return;
	}

	easy_int32 __read_bytes = 0;
	_get_usable(__fd,__usable_size);
	//	case 1: rpos_ <= wpos_
	if (__input->rpos() <= __input->wpos())
	{
		if (__input->size() - __input->wpos() >= __usable_size)
		{
			__read_bytes = Event_Handle_Srv::read(__fd,(easy_char*)__input->buffer() + __input->wpos(),__usable_size);
			if(-1 != __read_bytes && 0 != __read_bytes)
			{
				__input->set_wpos(__input->wpos() + __read_bytes);
			}
		}
		else
		{
			if (__input->size() - __input->wpos() + __input->rpos() > __usable_size)	// do not >= , reserev at lest on byte avoid data coveage!
			{
				size_t __buf_wpos_tail_left = __input->size() - __input->wpos();
				__read_bytes = Event_Handle_Srv::read(__fd,(easy_char*)__input->buffer() + __input->wpos(),__buf_wpos_tail_left);
				if(-1 != __read_bytes && 0 != __read_bytes)
				{
					__input->set_wpos(__input->wpos() + __read_bytes);
				}
				__read_bytes = Event_Handle_Srv::read(__fd,(easy_char*)__input->buffer(),__usable_size - __buf_wpos_tail_left);
				if(-1 != __read_bytes && 0 != __read_bytes)
				{
					__input->set_wpos(__usable_size - __buf_wpos_tail_left);
				}
			}
			else
			{
				size_t __new_size = (__input->size() + __usable_size - (__input->size() - __input->wpos())) / __input->size() * __input->size();
				__input->reallocate(__new_size,true);
				__read_bytes = Event_Handle_Srv::read(__fd,(easy_char*)__input->buffer() + __input->wpos(),__usable_size);
				if(-1 != __read_bytes && 0 != __read_bytes)
				{
					__input->set_wpos(__input->wpos() + __read_bytes);
				}
			}
		}
	}
	//	case 2: rpos_ > wpos_ 
	else if(__input->rpos() > __input->wpos())
	{
		if (__input->rpos() - __input->wpos() <= __usable_size)	// (rpos_ - wpos_ > cnt)  do not >= , reserev at lest on byte avoid data coveage!
		{
			easy_uint32 __new_size = __input->size() * 2;
			if ( __new_size <= __usable_size)
			{
				__new_size = __input->size() * 2 + __usable_size / __input->size() * __input->size();
			}
			__input->reallocate(__new_size,true);
		}
		__read_bytes = Event_Handle_Srv::read(__fd,(easy_char*)__input->buffer() + __input->wpos(),__usable_size);
		if(-1 != __read_bytes && 0 != __read_bytes)
		{
			__input->set_wpos(__input->wpos() + __read_bytes);
		}
	}
}

void Server_Impl::_read_thread()
{
	static const easy_int32 __head_size = 12;
	easy_char __read_buf[max_buffer_size_] = {};
	while (true)
	{
		lock_.acquire_lock();
#ifdef __DEBUG
		struct timeval __start_timeval;
		gettimeofday(&__start_timeval, NULL);
		easy_long __start_time = __start_timeval.tv_usec;
#endif //__DEBUG
		for (std::vector<Buffer*>::iterator __it = connects_copy.begin(); __it != connects_copy.end(); ++__it)
		{
			if(*__it)
			{
				Buffer::ring_buffer* __input = (*__it)->input_;
				Buffer::ring_buffer* __output = (*__it)->output_;
				if (!__input || !__output)
				{
					continue;
				}
#ifdef __DEBUG
				if(0 == (*__it)->fd_ % 1000)
				{
					printf("fd =%d,rpos = %d, wpos = %d\n",(*__it)->fd_,__input->rpos(),__input->wpos());
				}
#endif //__DEBUG
				while (!__input->read_finish())
				{
					easy_int32 __packet_length = 0;
					easy_int32 __log_level = 0;
					easy_int32 __frame_number = 0;
					easy_uint8 __packet_head[__head_size] = {};
					easy_int32 __head = 0;
					easy_uint32 __guid = 0;
					if(__input->read_finish())
					{
						break;
					}
					if(!__input->pre_read((easy_uint8*)&__packet_head,__head_size))
					{
						//	not enough data for read
						break;
					}
					memcpy(&__packet_length,__packet_head,4);
#if 0
					memcpy(&__head,__packet_head + 4,4);
					memcpy(&__guid,__packet_head + 8,4);
#endif
					if(!__packet_length || __packet_length > __input->size())
					{
						printf("__packet_length error %d\n",__packet_length);
						break;
					}
#if 0
					__log_level = (__head) & 0x000000ff;
					__frame_number = (__head >> 8);
#endif
					memset(__read_buf,0,max_buffer_size_);
					if (__packet_length + __head_size > max_buffer_size_)
					{
						printf("__packet_length + __head_size error %d\n",__packet_length);
						break;
					}
					if(__input->read((easy_uint8*)__read_buf,__packet_length + __head_size))
					{
						__output->append((easy_uint8*)__read_buf,__packet_length + __head_size);
					}
					else
					{
						break;
					}
				}
			}
		}
#ifdef __DEBUG
		struct timeval __end_timeval;
		gettimeofday(&__end_timeval, NULL);
		easy_long __end_time = __end_timeval.tv_usec;
		easy_long __time_read = __end_time - __start_time;
		printf("start time = %ld, end time = %ld,server impl time read = %ld\n",__start_time,__end_time,__time_read);
#endif //__DEBUG
		lock_.release_lock();
		easy::Util::sleep(max_sleep_time_);
	}
}

void Server_Impl::_write_thread()
{
	easy_int32 __fd = -1;
	easy_int32 __invalid_fd = 1;
	while (true)
	{
		lock_.acquire_lock();
		for (vector_buffer::iterator __it = connects_copy.begin(); __it != connects_copy.end(); )
		{
			if(*__it)
			{
				Buffer::ring_buffer* __output = (*__it)->output_;
				__fd = (*__it)->fd_;
				__invalid_fd = (*__it)->invalid_fd_;
				if(!__invalid_fd)
				{
					//	have closed
					_disconnect(*__it);
					__it = connects_copy.erase(__it);
					continue;
				}
				if (!__output)
				{
					++__it;
					continue;
				}
				easy_int32 __write_bytes = 0;
				if(__output->wpos() > __output->rpos())
				{
					easy_int32 __read_left = __output->wpos() - __output->rpos();
					__write_bytes = write(__fd,(const easy_char*)__output->buffer() + __output->rpos(),__read_left);
					if (-1 != __write_bytes && 0 != __write_bytes)
					{
						__output->set_rpos(__output->wpos());
					}
				}
				else if(__output->wpos() < __output->rpos())
				{
					easy_int32 __ring_buf_tail_left = __output->size() - __output->rpos();
					__write_bytes = write(__fd,(const easy_char*)__output->buffer() + __output->rpos(),__ring_buf_tail_left);
					if (-1 != __write_bytes && 0 != __write_bytes)
					{
						__output->set_rpos(__output->size());
					}
					easy_int32 __wpos = __output->wpos();
					__write_bytes = write(__fd,(const easy_char*)__output->buffer(),__wpos);
					if (-1 != __write_bytes && 0 != __write_bytes)
					{
						__output->set_rpos(__output->wpos());
					}
				}
				++__it;
			}
		}
		lock_.release_lock();
		easy::Util::sleep(max_sleep_time_);
	}
}

void Server_Impl::on_disconnect( easy_int32 __fd )
{
	map_buffer::iterator __it = connects_.find(__fd);
	if (__it != connects_.end())
	{
		if (__it->second)
		{
			__it->second->invalid_fd_ = 0;
		}
	}
}

void Server_Impl::_disconnect( Buffer* __buffer)
{
	if (!__buffer)
	{
		return;
	}
	map_buffer::iterator __it = connects_.find(__buffer->fd_);
	if (__it != connects_.end())
	{
		if (__it->second)
		{
			connects_.erase(__it);
		}
	}
	buffer_queue_.deallcate(__buffer);
}

Server_Impl::~Server_Impl()
{
	buffer_queue_.clear();
}



