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
#include "srv_test.h"
#include "reactor.h"
#include <stdlib.h>
#include <stdio.h>
#ifdef __LINUX
#include "easy_dump.h"
#endif // __LINUX

Srv_Test::Srv_Test(Reactor* __reactor,const easy_char* __host /*= "0.0.0.0"*/,easy_uint32 __port/* = 9876*/)
	: Server_Impl(__reactor,__host,__port) 
{

}

Srv_Test::~Srv_Test()
{

}

easy_int32 Srv_Test::handle_packet( easy_int32 __fd,const easy_char* __packet,easy_int32 __length )
{
	send_packet(__fd,__packet,__length);
	return -1;
}

void Srv_Test::connected( easy_int32 __fd )
{
	printf("connected __fd = %d \n",__fd);
}

void Srv_Test::dis_connected( easy_int32 __fd )
{
	printf("dis_connected __fd = %d \n",__fd);
}

easy_int32 main(easy_int32 __arg_num,easy_char** args)
{
	/*	
		g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_SELECT -o ./bin/srv_test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_select.h reactor_impl_select.cc server_impl.h server_impl.cc ./srv_test/srv_test.h ./srv_test/srv_test.cc  -I../easy/src/base -I.
		g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_EPOLL -o ./bin/srv_test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_epoll.h reactor_impl_epoll.cc server_impl.h server_impl.cc ./srv_test/srv_test.h ./srv_test/srv_test.cc -I../easy/src/base -I.
		g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_POLL -o ./bin/srv_test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_poll.h reactor_impl_poll.cc server_impl.h server_impl.cc ./srv_test/srv_test.h ./srv_test/srv_test.cc -I../easy/src/base -I.
	*/
	if(3 != __arg_num)
	{
		printf("param error,please input correct param! for example: nohup ./srv_test 192.168.22.63 9876 & \n");
		exit(1);
	}
#ifdef __LINUX
	signal(SIGSEGV,dump);
	//	when calls send() function twice if peer socket is closed, the SIG_PIPE signal will be trigger. and the SIG_PIPE 's default action is exit process.
	//	just ignore it! if use gdb debug,add 'handle SIGPIPE nostop print' or 'handle SIGPIPE nostop noprint' before run.
	signal(SIGPIPE,SIG_IGN);
#endif // __LINUX
	easy_char* __host = args[1];
	easy_uint32 __port = atoi(args[2]);
#ifdef __REACTOR_SINGLETON
	Reactor* __reactor = Reactor::instance();
#else
	Reactor* __reactor = new Reactor();
#endif // __REACTOR_SINGLETON
	Srv_Test __srv_test(__reactor,__host,__port);
	static const easy_int32 __max_time_out = 5000*1000;
	__reactor->event_loop(__max_time_out);
#ifdef __REACTOR_SINGLETON
	delete __reactor;
	__reactor = NULL;
#endif // __REACTOR_SINGLETON
	return 0;
}

