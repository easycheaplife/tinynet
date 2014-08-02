/****************************************************************************
 Copyright (c) 2013 King Lee

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
	
	int register_handle(Event_Handle* __handle,int __fd,int __mask,int __connect);
	
	int remove_handle(Event_Handle* __handle,int __mask);
	
	int handle_event(unsigned long __millisecond);

	int handle_close(int __fd);
	
	int event_loop(unsigned long __millisecond);

	//	__fd is the broadcaster
	void broadcast(int __fd,const char* __data,unsigned int __length);

	void write(int __fd,const char* __data, int __length);

private:
	void _init();
	
	void _add_event(int __fd,uint32_t __event);
	
	void _mod_event(int __fd,uint32_t __event);
	
	int						fd_;

	int						fd_epoll_;
	
	struct epoll_event 				ev_;
	
	struct epoll_event				events_[MAX_EVENTS];
	
	Event_Handle* 					handle_;
};
