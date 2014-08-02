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
class Event_Handle;

enum Mask
{
	kMaskAccept = 1,
	kMaskRead ,
	kMaskWrite,
	//	for client socket
	kMaskConnect
};


class Reactor_Impl
{
public:
	virtual ~Reactor_Impl() {}
	
	/*
	 *	__connect: is coming connection
	 */
	virtual int register_handle(Event_Handle* __handle,int __fd,int __mask,int __connect = 0) = 0;
	
	virtual int remove_handle(Event_Handle* __handle,int __mask) = 0;
	
	virtual int handle_event(unsigned long __time) = 0;
	
	virtual int event_loop(unsigned long __time) = 0;

	virtual int handle_close(int __fd) = 0;

	//	__fd is the broadcaster
	virtual void broadcast(int __fd,const char* __data,unsigned int __length) = 0;
};
