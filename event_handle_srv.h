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
#include <string>
#include "event_handle.h"

class Event_Handle_Srv : public  Event_Handle
{
public:
	Event_Handle_Srv(Reactor* __reactor,const easy_char* __host,easy_uint32 __port);

	virtual ~Event_Handle_Srv();
	
	//	-1 means error happened, 0 means no error.
	virtual easy_int32 handle_input(easy_int32 __fd);
	
	virtual easy_int32 handle_output(easy_int32 __fd);
	
	virtual easy_int32 handle_exception(easy_int32 __fd);
	
	//	close a socket by special fd
	virtual easy_int32 handle_close(easy_int32 __fd);
	
	virtual easy_int32 handle_timeout(easy_int32 __fd);

	virtual easy_int32 handle_packet(easy_int32 __fd,const easy_char* __packet,easy_int32 __length);
	
	virtual easy_int32 get_handle() const { return fd_;}

	void	broadcast(easy_int32 __fd,const easy_char* __data, easy_uint32 __length);

	//	read data from network cache
	easy_int32 read(easy_int32 __fd,easy_char* __buf, easy_int32 __length,easy_int32 __flags = 0); 

	//	read data from network hardware device
	easy_int32 read_zero_copy(easy_int32 __fd,easy_char* __buf, easy_int32 __length,easy_int32 __flags = 0);

	//	write data to peer 
	easy_int32 write(easy_int32 __fd,const easy_char* __data, easy_int32 __length);

public:
	//	pure virtual function, subclass must define it.
	virtual void on_connected(easy_int32 __fd) = 0;

	virtual void on_read(easy_int32 __fd) = 0;

	virtual void on_disconnect(easy_int32 __fd) = 0;

	//	for byte stream
	virtual easy_int32 on_packet(easy_int32 __fd,const easy_char* __packet,easy_int32 __length) = 0;

	//	for protobuf
	virtual easy_int32 on_packet(easy_int32 __fd,const std::string& __string_packet) = 0;

protected:
	void 	_init();
	
	void 	_set_noblock(easy_int32 __fd);

	void	_set_reuse_addr(easy_int32 __fd);

	void	_get_usable( easy_int32 __fd, easy_ulong& __usable_size);

	void	_set_no_delay(easy_int32 __fd);
private:
	easy_int32  	fd_;

	std::string		host_;

	easy_uint32		port_;
};