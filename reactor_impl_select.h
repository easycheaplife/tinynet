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
/************************************************************************/
/*  
 *  a ring buffer to work with network buffer cache 
 *  bugs:
 *  #20003	2014-12-08 
 *  if client request very fast, it will occupy all of resource, other thread will get no resource.
 *
 */
/************************************************************************/
#if defined __WINDOWS || defined WIN32
    #ifndef FD_SETSIZE
    #define FD_SETSIZE      1024
    #endif /* FD_SETSIZE */
    #include <WinSock.h>
#elif defined __LINUX || defined __MACX
    #include <sys/select.h>
    #include <sys/socket.h>
    #include <errno.h>
    #include <unistd.h>
#endif //   __WINDOWS

#include <map>
#include <vector>
#include "reactor_impl.h"

//	struct forward declaration 
struct Event_Handle_Data;

class Reactor_Impl_Select : public Reactor_Impl
{
public:
	Reactor_Impl_Select();
	
	~Reactor_Impl_Select() {}
	
	easy_int32 register_handle(Event_Handle* __handle,easy_int32 __fd,easy_int32 __mask,easy_int32 __connect);
	
	easy_int32 remove_handle(Event_Handle* __handle,easy_int32 __mask);
	
	easy_int32 handle_event(easy_ulong __millisecond);

	easy_int32 handle_close(easy_int32 __fd);
	
	easy_int32 event_loop(easy_ulong __millisecond);

	void broadcast(easy_int32 __fd,const easy_char* __data,easy_uint32 __length);
	
	void write(easy_int32 __fd,const easy_char* __data, easy_int32 __length);
private:
	fd_set 							read_set_;
	
	fd_set 							write_set_;
	
	fd_set 							excepion_set_;
	
	easy_int32						fd_;
	
	easy_int32						max_fd_;
	
	Event_Handle* 					handle_;

	static const easy_uint32		max_sleep_time_;

	std::vector<Event_Handle_Data*>	events_;
};
