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
/*
  protobuf version:V2.5.0
  general:
	$export LD_LIBRARY_PATH=$LD_LIBRARY_PATH../easy/dep/protobuf/src/.libs
	$../easy/dep/protobuf/src/.libs/protoc -I./ --cpp_out=. transfer.proto
  compile:
	$g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_EPOLL -D__TEST -o ./bin/srv_test reactor.h reactor.cc socket_ex.h socket_ex.cc event_handle.h event_handle.cc event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_epoll.h reactor_impl_epoll.cc transfer.pb.h transfer.pb.cc server_impl.h server_protobuf_impl.cc ./srv_test/srv_test.h ./srv_test/srv_test.cc -I. -I../easy/src/base -I../easy/dep/protobuf/src/ -L../easy/dep/protobuf/src/.libs -lprotobuf
  run:
    $./bin/srv_test 192.168.22.61 9876
*/
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
#include "packet_handle.h"

#define CC_CALLBACK_0(__selector__,__target__, ...) std::bind(&__selector__,__target__, ##__VA_ARGS__)

const easy_uint32 Server_Impl::max_buffer_size_ = 1024*8;
const easy_uint32 Server_Impl::max_sleep_time_ = 1000*1;

Server_Impl::Server_Impl( Reactor* __reactor,const easy_char* __host,easy_uint32 __port )
    : Event_Handle_Srv(__reactor,__host,__port) {
#ifndef __HAVE_IOCP
    auto __thread_read = std::thread(CC_CALLBACK_0(Server_Impl::_read_thread,this));
    __thread_read.detach();
    auto __thread_write = std::thread(CC_CALLBACK_0(Server_Impl::_write_thread,this));
    __thread_write.detach();
#endif // !__HAVE_IOCP
}

void Server_Impl::on_connected( easy_int32 __fd ) {
#ifndef __HAVE_IOCP
    //	proxy server do not need manager connection information
#if 0
    if (!is_proxy())
#endif
    {
        lock_.acquire_lock();
        connects_[__fd] = buffer_queue_.allocate(__fd,max_buffer_size_);
        connects_copy.push_back(connects_[__fd]);
        lock_.release_lock();
    }
#endif // __HAVE_IOCP
    //	callback connected
    connected(__fd);
}

void Server_Impl::on_read( easy_int32 __fd ) {
    if(/*is_proxy()*/false) {
        _read_directly(__fd);
    } else {
        _read_completely(__fd);
    }
}

easy_int32 Server_Impl::on_packet(easy_int32 __fd,const easy_char* __packet,easy_int32 __length) {
    return -1;
}

easy_int32 Server_Impl::on_packet( easy_int32 __fd,const std::string& __string_packet) {
    handle_packet(__fd,__string_packet,easy_null);
    return -1;
}

void Server_Impl::_read_completely(easy_int32 __fd) {
    //	the follow code is ring_buf's append function actually.
    if(!connects_[__fd]) {
        return;
    }
    Buffer::ring_buffer* __input = connects_[__fd]->input_;
    if(!__input) {
        return;
    }
    easy_int32 __usable_size = 0;
    //	check the peer socket is ok
    static easy_uint8  __check_buf[max_buffer_size_] = {};
    __usable_size = Event_Handle_Srv::read(__fd,(easy_char*)__check_buf,max_buffer_size_,MSG_PEEK);
    if(-1 == __usable_size) {
        return;
    }
#if 0
    //	replace by recv using flag of MSG_PEEK
    _get_usable(__fd,__usable_size);
#endif
    easy_int32 __read_bytes = 0;
    //	case 1: rpos_ <= wpos_
    if (__input->rpos() <= __input->wpos()) {
        if (__input->size() - __input->wpos() >= __usable_size) {
            __read_bytes = Event_Handle_Srv::read(__fd,(easy_char*)__input->buffer() + __input->wpos(),__usable_size);
            if(-1 != __read_bytes && 0 != __read_bytes) {
                __input->set_wpos(__input->wpos() + __read_bytes);
            }
        } else {
            if (__input->size() - __input->wpos() + __input->rpos() > __usable_size) {	// do not >= , reserev at lest on byte avoid data coveage!
                size_t __buf_wpos_tail_left = __input->size() - __input->wpos();
                __read_bytes = Event_Handle_Srv::read(__fd,(easy_char*)__input->buffer() + __input->wpos(),__buf_wpos_tail_left);
                if(-1 != __read_bytes && 0 != __read_bytes) {
                    __input->set_wpos(__input->wpos() + __read_bytes);
                }
                __read_bytes = Event_Handle_Srv::read(__fd,(easy_char*)__input->buffer(),__usable_size - __buf_wpos_tail_left);
                if(-1 != __read_bytes && 0 != __read_bytes) {
                    __input->set_wpos(__usable_size - __buf_wpos_tail_left);
                }
            } else {
                size_t __new_size = (__input->size() + __usable_size - (__input->size() - __input->wpos())) / __input->size() * __input->size();
                __input->reallocate(__new_size,true);
                __read_bytes = Event_Handle_Srv::read(__fd,(easy_char*)__input->buffer() + __input->wpos(),__usable_size);
                if(-1 != __read_bytes && 0 != __read_bytes) {
                    __input->set_wpos(__input->wpos() + __read_bytes);
                }
            }
        }
    }
    //	case 2: rpos_ > wpos_
    else if(__input->rpos() > __input->wpos()) {
        if (__input->rpos() - __input->wpos() <= __usable_size) {	// (rpos_ - wpos_ > cnt)  do not >= , reserev at lest on byte avoid data coveage!
            easy_uint32 __new_size = __input->size() * 2;
            if ( __new_size <= __usable_size) {
                __new_size = __input->size() * 2 + __usable_size / __input->size() * __input->size();
            }
            __input->reallocate(__new_size,true);
        }
        __read_bytes = Event_Handle_Srv::read(__fd,(easy_char*)__input->buffer() + __input->wpos(),__usable_size);
        if(-1 != __read_bytes && 0 != __read_bytes) {
            __input->set_wpos(__input->wpos() + __read_bytes);
        }
    }
}

void Server_Impl::_read_directly(easy_int32 __fd) {
    //	tmp code
    easy_char __read_buf[max_buffer_size_] = {};
    easy_int32 __read_bytes = read_zero_copy(__fd,__read_buf,max_buffer_size_);
    if (-1 != __read_bytes) {
        handle_packet(__fd,__read_buf,__read_bytes);
    }
}

void Server_Impl::_read_thread() {
    std::string 	 __string_packet;
    static const easy_int32 __head_size = sizeof(easy_uint32);
    easy_char __read_buf[max_buffer_size_] = {};
    while (true) {
        lock_.acquire_lock();
        for (std::vector<Buffer*>::iterator __it = connects_copy.begin(); __it != connects_copy.end(); ++__it) {
            if(*__it) {
                Buffer::ring_buffer* __input = (*__it)->input_;
                Buffer::ring_buffer* __output = (*__it)->output_;
                if (!__input || !__output) {
                    continue;
                }
                while (!__input->read_finish()) {
                    easy_uint32 __packet_length = 0;
                    if(__input->read_finish()) {
                        break;
                    }
                    if(!__input->peek((easy_uint8*)&__packet_length,__head_size)) {
                        //	not enough data for read
                        break;
                    }
                    easy_uint16 __real_packet_length = __packet_length & 0x0000ffff;
                    easy_uint16 __real_fd = __packet_length >> 16;
                    if(!__real_packet_length || __real_packet_length > __input->size()) {
                        printf("__packet_length error %d\n",__real_packet_length);
                        break;
                    }
                    memset(__read_buf,0,max_buffer_size_);
                    if (__real_packet_length + __head_size > max_buffer_size_) {
                        printf("__packet_length + __head_size error %d\n",__real_packet_length);
                        break;
                    }
                    __string_packet.clear();
                    if(__input->read(__string_packet,__real_packet_length + __head_size)) {
#ifdef __TEST
                        //	return intact
                        __output->append((easy_uint8*)__string_packet.c_str(),__real_packet_length + __head_size);

#else
                        handle_packet((*__it)->fd_,__string_packet.c_str() + __head_size,&__real_fd);
#endif //__TEST
                    } else {
                        break;
                    }
                }
            }
        }
        lock_.release_lock();
        easy::Util::sleep(max_sleep_time_);
    }
}

void Server_Impl::_write_thread() {
    easy_int32 __fd = -1;
    easy_int32 __invalid_fd = 1;
    while (true) {
        lock_.acquire_lock();
        for (vector_buffer::iterator __it = connects_copy.begin(); __it != connects_copy.end(); ) {
            if(*__it) {
                Buffer::ring_buffer* __output = (*__it)->output_;
                __fd = (*__it)->fd_;
                __invalid_fd = (*__it)->invalid_fd_;
                if(!__invalid_fd) {
                    //	have closed
                    _disconnect(*__it);
                    __it = connects_copy.erase(__it);
                    continue;
                }
                if (!__output) {
                    ++__it;
                    continue;
                }
                easy_int32 __write_bytes = 0;
                if(__output->wpos() > __output->rpos()) {
                    easy_int32 __read_left = __output->wpos() - __output->rpos();
                    __write_bytes = write(__fd,(const easy_char*)__output->buffer() + __output->rpos(),__read_left);
                    if (-1 != __write_bytes && 0 != __write_bytes) {
                        //	fix #20005,old version:__output->set_rpos(__output->wpos());
                        //	__output->wpos() may be change when __output->set_rpos called.
                        __output->set_rpos(__output->rpos() + __write_bytes);
                    }
                } else if(__output->wpos() < __output->rpos()) {
                    easy_int32 __ring_buf_tail_left = __output->size() - __output->rpos();
                    __write_bytes = write(__fd,(const easy_char*)__output->buffer() + __output->rpos(),__ring_buf_tail_left);
                    if (-1 != __write_bytes && 0 != __write_bytes) {
                        __output->set_rpos(__output->size());
                    }
                    easy_int32 __wpos = __output->wpos();
                    __write_bytes = write(__fd,(const easy_char*)__output->buffer(),__wpos);
                    if (-1 != __write_bytes && 0 != __write_bytes) {
                        //	fix #20005,old version:__output->set_rpos(__output->wpos());
                        __output->set_rpos(__write_bytes);
                    }
                }
                ++__it;
            }
        }
        lock_.release_lock();
        easy::Util::sleep(max_sleep_time_*10);
    }
}

void Server_Impl::send_packet( easy_int32 __fd,const easy_char* __packet,easy_int32 __length ) {
#ifndef __HAVE_IOCP
#if 0
    //	actually, this solution is bad for write directly(3200+ < 4200+ kbytes)
    if (connects_[__fd]) {
        if (connects_[__fd]->output_) {
            connects_[__fd]->output_->append((easy_uint8*)__packet,__length);
        }
    }
#else
    write(__fd,__packet,__length);
#endif
#else
    write(__fd,__packet,__length);
#endif // __HAVE_IOCP
}

void Server_Impl::on_disconnect( easy_int32 __fd ) {
#ifndef __HAVE_IOCP
    //	proxy server do not need manager connection information
#if 0
    if (!is_proxy())
#endif
    {
        map_buffer::iterator __it = connects_.find(__fd);
        if (__it != connects_.end()) {
            if (__it->second) {
                __it->second->invalid_fd_ = 0;
                //	callback dis_connected
                dis_connected(__fd);
            }
        }
    }
#else
    //	callback dis_connected
    dis_connected(__fd);
#endif // __HAVE_IOCP
}

void Server_Impl::_disconnect( Buffer* __buffer) {
    if (!__buffer) {
        return;
    }
    map_buffer::iterator __it = connects_.find(__buffer->fd_);
    if (__it != connects_.end()) {
        if (__it->second) {
            connects_.erase(__it);
        }
    }
    buffer_queue_.deallcate(__buffer);
}

Server_Impl::~Server_Impl() {
    buffer_queue_.clear();
}



