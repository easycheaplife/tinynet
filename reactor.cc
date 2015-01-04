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
#include "reactor.h"

#if  defined __HAVE_IOCP
#include "reactor_impl_iocp.h"
#elif defined  __HAVE_SELECT || defined WIN32
#include "reactor_impl_select.h"
#elif defined __HAVE_EPOLL
#include "reactor_impl_epoll.h"
#elif defined __HAVE_POLL
#include "reactor_impl_poll.h"
#endif 

#ifdef __REACTOR_SINGLETON
Reactor* Reactor::reactor_ = 0;
#endif // __REACTOR_SINGLETON

Reactor::Reactor()
{
#if defined   __HAVE_IOCP
	reactor_impl_ = new Reactor_Impl_Iocp();
#elif defined  __HAVE_SELECT || defined WIN32
	reactor_impl_ = new Reactor_Impl_Select();
#elif defined  __HAVE_EPOLL
	reactor_impl_ = new Reactor_Impl_Epoll();
#elif defined  __HAVE_POLL
	reactor_impl_ = new Reactor_Impl_Poll();
#endif 
}

Reactor::~Reactor() 
{
	if(reactor_impl_)
	{
		delete reactor_impl_;
		reactor_impl_ = NULL;
	}
}
#ifdef __REACTOR_SINGLETON
Reactor* Reactor::instance()
{
	if( 0 ==  Reactor::reactor_)
	{
		Reactor::reactor_ = new Reactor();
	}
	return Reactor::reactor_;
}

void Reactor::destory()
{
	if(reactor_)
	{
		delete reactor_;
		reactor_ = NULL;
	}
}
#endif // __REACTOR_SINGLETON

easy_int32 Reactor::register_handle(Event_Handle* __handle,easy_int32 __fd,easy_int32 __mask)
{

	return -1;
}

easy_int32 Reactor::remove_handle(Event_Handle* __handle,easy_int32 __mask)
{
	return -1;
}

easy_int32 Reactor::handle_event(easy_ulong __millisecond)
{
	
	return -1;
}

easy_int32 Reactor::event_loop(easy_ulong __millisecond)
{
	reactor_impl_->event_loop(__millisecond);
	return -1;
}

