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
#include <sys/epoll.h>
#include <unistd.h>
#include "reactor_impl.h"

#define MAX_EVENTS 1024

class Reactor_Impl_Epoll : public Reactor_Impl
{
public:
	Reactor_Impl_Epoll();
	
	~Reactor_Impl_Epoll() {}
	
	easy_int32 register_handle(Event_Handle* __handle,easy_int32 __fd,easy_int32 __mask,easy_int32 __connect);
	
	easy_int32 remove_handle(Event_Handle* __handle,easy_int32 __mask);
	
	easy_int32 handle_event(easy_ulong __millisecond);

	easy_int32 handle_close(easy_int32 __fd);
	
	easy_int32 event_loop(easy_ulong __millisecond);

	//	__fd is the broadcaster
	void broadcast(easy_int32 __fd,const easy_char* __data,easy_uint32 __length);

	void write(easy_int32 __fd,const easy_char* __data, easy_int32 __length);

private:
	void _init();
	
	void _add_event(easy_int32 __fd,uint32_t __event);
	
	void _mod_event(easy_int32 __fd,uint32_t __event);
	
	easy_int32						fd_;

	easy_int32						fd_epoll_;
	
	struct epoll_event 				ev_;
	
	struct epoll_event				events_[MAX_EVENTS];
	
	Event_Handle* 					handle_;
};
