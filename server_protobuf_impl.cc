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
#endif // __LINUX
#include "server_protobuf_impl.h"
#include "transfer.pb.h"

#define CC_CALLBACK_0(__selector__,__target__, ...) std::bind(&__selector__,__target__, ##__VA_ARGS__)

const unsigned int Server_Impl::max_buffer_size_ = 1024*8;
const unsigned int Server_Impl::max_sleep_time_ = 1000*500;

Server_Impl::Server_Impl( Reactor* __reactor,const char* __host /*= "0.0.0.0"*/,unsigned int __port /*= 9876*/ )
	: Event_Handle_Srv(__reactor,__host,__port) 
{
#ifndef __HAVE_IOCP
	auto __thread_read = std::thread(CC_CALLBACK_0(Server_Impl::_read_thread,this));
	__thread_read.detach();
	auto __thread_write = std::thread(CC_CALLBACK_0(Server_Impl::_write_thread,this));
	__thread_write.detach();
#endif // !__HAVE_IOCP
}

void Server_Impl::on_connected( int __fd )
{
	printf("on_connected __fd = %d \n",__fd);
	lock_.acquire_lock();
	connects_[__fd] = buffer_queue_.allocate(__fd,max_buffer_size_);
	connects_copy.push_back(connects_[__fd]);
	lock_.release_lock();
}

void Server_Impl::on_read( int __fd )
{
#ifdef __HAVE_EPOLL
	_read_completely(__fd);
#else
	_read(__fd);
#endif //__HAVE_EPOLL
}

void Server_Impl::_read( int __fd )
{
	//	the follow code is ring_buf's append function actually.
	unsigned long __usable_size = 0;
	if(!connects_[__fd])
	{
		return;
	}
	Buffer::ring_buffer* __input = connects_[__fd]->input_;
	if(!__input)
	{
		return;
	}
	
	_get_usable(__fd,__usable_size);
	int __ring_buf_tail_left = __input->size() - __input->wpos();
	int __read_bytes = 0;
	if(__usable_size <= __ring_buf_tail_left)
	{
		__read_bytes = Event_Handle_Srv::read(__fd,(char*)__input->buffer() + __input->wpos(),__usable_size);
		if(-1 != __read_bytes && 0 != __read_bytes)
		{
			__input->set_wpos(__input->wpos() + __usable_size);
		}
	}
	else
	{	
		//	if not do this,the connection will be closed!
		if(0 != __ring_buf_tail_left)
		{
			__read_bytes = Event_Handle_Srv::read(__fd,(char*)__input->buffer() +  __input->wpos(),__ring_buf_tail_left);
			if(-1 != __read_bytes && 0 != __read_bytes)
			{
				__input->set_wpos(__input->size());
			}
		}
		int __ring_buf_head_left = __input->rpos();
		int __read_left = __usable_size - __ring_buf_tail_left;
		if(__ring_buf_head_left >= __read_left)
		{
			__read_bytes = Event_Handle_Srv::read(__fd,(char*)__input->buffer(),__read_left);
			if(-1 != __read_bytes && 0 != __read_bytes)
			{
				__input->set_wpos(__read_left);
			}
		}
		else
		{
			//	maybe some problem here when data not recv completed for epoll ET.you can realloc the input buffer or use while(recv) until return EAGAIN.
			__read_bytes = Event_Handle_Srv::read(__fd,(char*)__input->buffer(),__ring_buf_head_left);
			if(-1 != __read_bytes && 0 != __read_bytes)
			{
				__input->set_wpos(__ring_buf_head_left);
			}
		}
	}
}

void Server_Impl::_read_completely(int __fd)
{
	//	the follow code is ring_buf's append function actually.
	unsigned long __usable_size = 0;
	if(!connects_[__fd])
	{
		return;
	}
	Buffer::ring_buffer* __input = connects_[__fd]->input_;
	if(!__input)
	{
		return;
	}
	
	_get_usable(__fd,__usable_size);
	int __ring_buf_tail_left = __input->size() - __input->wpos();
	int __read_bytes = 0;
	if(__usable_size <= __ring_buf_tail_left)
	{
		__read_bytes = Event_Handle_Srv::read(__fd,(char*)__input->buffer() + __input->wpos(),__usable_size);
		if(-1 != __read_bytes && 0 != __read_bytes)
		{
			__input->set_wpos(__input->wpos() + __usable_size);
		}
	}
	else
	{	
		//	if not do this,the connection will be closed!
		if(0 != __ring_buf_tail_left)
		{
			__read_bytes = Event_Handle_Srv::read(__fd,(char*)__input->buffer() +  __input->wpos(),__ring_buf_tail_left);
			if(-1 != __read_bytes && 0 != __read_bytes)
			{
				__input->set_wpos(__input->size());
			}
		}
		int __ring_buf_head_left = __input->rpos();
		int __read_left = __usable_size - __ring_buf_tail_left;
		if(__ring_buf_head_left >= __read_left)
		{
			__read_bytes = Event_Handle_Srv::read(__fd,(char*)__input->buffer(),__read_left);
			if(-1 != __read_bytes && 0 != __read_bytes)
			{
				__input->set_wpos(__read_left);
			}
		}
		else
		{
			//	make sure __read_left is less than __input.size() + __ring_buf_head_left,usually,It's no problem.
			__input->reallocate(__input->size());
			__read_bytes = Event_Handle_Srv::read(__fd,(char*)__input->buffer() + __input->wpos(),__read_left);
			if(-1 != __read_bytes && 0 != __read_bytes)
			{
				__input->set_wpos(__read_left);
			}
#ifdef __DEBUG
			//	test ok! set max_buffer_size_ = 256 will easy to test. 
			printf("__input->reallocate called, __fd = %d,__read_left = %d,buffer left size = %d\n",__fd,__read_left,__input->size() - __input->wpos());
			int __head_size = 12;
			printf("after __input->reallocate called,buffer = %s\n",__input->buffer() + __input->rpos() + __head_size);
#endif //__DEBUG
		}
	}
}

void Server_Impl::_read_thread()
{
	transfer::Packet __packet_protobuf;
	std::string 	 __string_packet;
	static const int __head_size = 4;
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
					int __packet_length = 0;
					int __log_level = 0;
					int __frame_number = 0;
					unsigned char __packet_head[__head_size] = {};
					int __head = 0;
					unsigned int __guid = 0;
					if(!__input->pre_read(__packet_head,__head_size))
					{
						//	not enough data for read
						break;
					}
					__packet_length = (int)*__packet_head;
					printf("__packet_length = %d\n",__packet_length);
					if(!__packet_length)
					{
						printf("__packet_length error\n");
						break;
					}
					__packet_protobuf.Clear();
					__string_packet.clear();
					char __read_buf[max_buffer_size_] = {};
					if(__input->read((unsigned char*)__read_buf,__packet_length + __head_size))
					{
						__string_packet = __read_buf + 4;
						__packet_protobuf.ParseFromString(__string_packet);
#ifdef __DEBUG
						printf("__packet_protobuf.head = %d\n",__packet_protobuf.head());
						printf("__packet_protobuf.guid = %d\n",__packet_protobuf.guid());
						printf("__packet_protobuf.content = %s\n",__packet_protobuf.content().c_str());
#endif //__DEBUG
						__output->append((unsigned char*)__read_buf,__packet_length + __head_size);
					}
					else
					{
						break;
					}
				}
			}
		}
		lock_.release_lock();
#ifdef __LINUX
		usleep(max_sleep_time_);
#endif // __LINUX
	}
}

void Server_Impl::_write_thread()
{
	int __fd = -1;
	int __invalid_fd = 1;
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
				if(__output->wpos() > __output->rpos())
				{
					write(__fd,(const char*)__output->buffer() + __output->rpos(),__output->wpos() - __output->rpos());
				}
				else if(__output->wpos() < __output->rpos())
				{
					write(__fd,(const char*)__output->buffer() + __output->rpos(),__output->size() - __output->rpos());
					write(__fd,(const char*)__output->buffer(),__output->wpos());
				}
				__output->set_rpos(__output->wpos());
				++__it;
			}
		}
		lock_.release_lock();
#ifdef __LINUX
		usleep(max_sleep_time_);
#endif // __LINUX
	}
}

void Server_Impl::on_disconnect( int __fd )
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



