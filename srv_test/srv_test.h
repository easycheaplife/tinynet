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
#ifndef srv_test_h__
#define srv_test_h__
#include "server_impl.h"

class Srv_Test : public Server_Impl
{
public:
	Srv_Test(Reactor* __reactor,const easy_char* __host = "0.0.0.0",easy_uint32 __port = 9876);

	//	for byte stream, it is the  default way
	easy_int32 handle_packet(easy_int32 __fd,const easy_char* __packet,easy_int32 __length);

	//	for protobuf,just return
	easy_int32 handle_packet(easy_int32 __fd,const std::string& __string_packet) { return -1;}

	void connected(easy_int32 __fd);

	void dis_connected(easy_int32 __fd);

	~Srv_Test();
};


#endif // srv_test_h__


