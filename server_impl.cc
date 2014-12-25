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
#if defined __LINUX || defined __MACX
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#endif // __LINUX
#include "server_impl.h"
#include "easy_util.h"

#define CC_CALLBACK_0(__selector__,__target__, ...) std::bind(&__selector__,__target__, ##__VA_ARGS__)

const easy_uint32 Server_Impl::max_buffer_size_ = 1024*8;
const easy_uint32 Server_Impl::max_sleep_time_ = 1000*500;

Server_Impl::Server_Impl( Reactor* __reactor,const easy_char* __host ,easy_uint32 __port  )
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
#ifndef __HAVE_IOCP
	lock_.acquire_lock();
	connects_[__fd] = buffer_queue_.allocate(__fd,max_buffer_size_);
	connects_copy.push_back(connects_[__fd]);
	lock_.release_lock();
#endif // __HAVE_IOCP
	//	callback connected 
	connected(__fd);
}

void Server_Impl::on_read( easy_int32 __fd )
{
	_read_completely(__fd);
}

easy_int32 Server_Impl::on_packet( easy_int32 __fd,const easy_char* __packet,easy_int32 __length )
{
	handle_packet(__fd,__packet,__length);
	return -1;
}

easy_int32 Server_Impl::on_packet(easy_int32 __fd,const std::string& __string_packet)
{
	//	no necessary implementation here
	return -1;
}

void Server_Impl::_read_completely(easy_int32 __fd)
{
	//	the follow code is ring_buf's append function actually.
	if(!connects_[__fd])
	{
		return;
	}
	Buffer::ring_buffer* __input = connects_[__fd]->input_;
	if(!__input)
	{
		return;
	}
	easy_int32 __usable_size = 0;
	//	check the peer socket is ok
	static easy_uint8  __check_buf[max_buffer_size_] = {};
	__usable_size = Event_Handle_Srv::read(__fd,(easy_char*)__check_buf,max_buffer_size_,MSG_PEEK);
	if(-1 == __usable_size)
	{
		return;
	}
#if 0
	//	replace by recv using flag of MSG_PEEK
	_get_usable(__fd,__usable_size);
#endif
	easy_int32 __read_bytes = 0;
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
	static const easy_int32 __head_size = sizeof(easy_uint16);
	easy_char __read_buf[max_buffer_size_] = {};
	while (true)
	{
		lock_.acquire_lock();
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
				while (!__input->read_finish())
				{
                    easy_uint16 __packet_length = 0;
					if(__input->read_finish())
					{
						break;
					}
					if(!__input->peek((easy_uint8*)&__packet_length,__head_size))
					{
						//	not enough data for read
						break;
					}
					if(!__packet_length || __packet_length > __input->size())
					{
						printf("__packet_length error %d\n",__packet_length);
						break;
					}
					memset(__read_buf,0,max_buffer_size_);
					if (__packet_length + __head_size > max_buffer_size_)
					{
						printf("__packet_length + __head_size error %d\n",__packet_length);
						break;
					}
					if(__input->read((easy_uint8*)__read_buf,__packet_length + __head_size))
					{
						if (0)
						{
							__output->append((easy_uint8*)__read_buf,__packet_length + __head_size);
						}
						else
						{
							handle_packet((*__it)->fd_,__read_buf,__packet_length + __head_size);
						}
					}
					else
					{
						break;
					}
				}
			}
		}
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

void Server_Impl::send_packet( easy_int32 __fd,const easy_char* __packet,easy_int32 __length )
{
#ifndef __HAVE_IOCP
	if (connects_[__fd])
	{
		if (connects_[__fd]->output_)
		{
			connects_[__fd]->output_->append((easy_uint8*)__packet,__length);
		}
	}
#else
	write(__fd,__packet,__length);
#endif // __HAVE_IOCP
}

void Server_Impl::on_disconnect( easy_int32 __fd )
{
#ifndef __HAVE_IOCP
	map_buffer::iterator __it = connects_.find(__fd);
	if (__it != connects_.end())
	{
		if (__it->second)
		{
			__it->second->invalid_fd_ = 0;
			//	callback dis_connected
			dis_connected(__fd);
		}
	}
#else
	//	callback dis_connected
	dis_connected(__fd);
#endif // __HAVE_IOCP
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





