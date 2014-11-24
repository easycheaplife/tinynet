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
#ifndef client_protobuf_impl_h__
#define client_protobuf_impl_h__
#include "event_handle_cli.h"
#include "easy_ring_buffer.h"
#include "easy_allocator.h"

class Client_Impl : public Event_Handle_Cli
{
public:
	Client_Impl(Reactor* __reactor,const easy_char* __host,easy_uint32 __port);

	virtual ~Client_Impl();

	void on_read(easy_int32 __fd);

	virtual easy_int32 handle_packet(easy_int32 __fd,easy_uint32 __packet_id,const std::string& __string_packet) = 0;

private:
	void	_read_thread();

private:
	easy::EasyRingbuffer<easy_uint8,easy::alloc>* ring_buf_;

};

#endif // client_protobuf_impl_h__
