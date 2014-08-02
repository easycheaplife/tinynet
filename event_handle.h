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
class Reactor;
class Event_Handle
{
public:

	Event_Handle(){}

	virtual ~Event_Handle() { }

	virtual int handle_input(int __fd){ return -1; }
	
	virtual int handle_output(int __fd){ return -1; }
	
	virtual int handle_exception(int __fd){ return -1; }
	
	virtual int handle_close(int __fd){ return -1; }
	
	virtual int handle_timeout(int __fd){ return -1; }

	virtual int	read(int __fd,char* __buf, int __length) { return -1;}

	virtual int write(int __fd,const char* __data, int __length) { return -1; }
	
	Reactor* reactor() const { return reactor_; }
	
protected:
	Event_Handle(Reactor* __reactor){ reactor_ = __reactor;}
	
private:
	Reactor* reactor_;
};