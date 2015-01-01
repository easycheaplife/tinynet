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
#ifndef event_handle_h__
#define event_handle_h__
#include "easy_base_type.h"
class Reactor;
class Event_Handle
{
public:

	Event_Handle(){}

	virtual ~Event_Handle() { }

	virtual easy_int32 handle_input(easy_int32 __fd){ return -1; }

	virtual easy_int32 handle_output(easy_int32 __fd){ return -1; }

	virtual easy_int32 handle_exception(easy_int32 __fd){ return -1; }

	virtual easy_int32 handle_close(easy_int32 __fd){ return -1; }

	virtual easy_int32 handle_timeout(easy_int32 __fd){ return -1; }

	virtual easy_int32 handle_packet(easy_int32 __fd,const easy_char* __packet,easy_int32 __length){ return -1; }

	virtual easy_int32 read(easy_int32 __fd,easy_char* __buf, easy_int32 __length) { return -1;}

	virtual easy_int32 write(easy_int32 __fd,const easy_char* __data, easy_int32 __length) { return -1; }

	Reactor* reactor() const { return reactor_; }

protected:
	Event_Handle(Reactor* __reactor){ reactor_ = __reactor;}

private:
	Reactor* reactor_;
};

#endif // event_handle_h__
