    tinynet is light-weighted network library.it supports select/poll/epoll/iocp multiplexing model.you can switch any one by define different macro.
    compiler:
	make sure easy project at the same directory,easey git address:https://github.com/yuyunliuhen/easy
	(1)	linux
	use gcc,for example:
	g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_SELECT -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_select.h reactor_impl_select.cc server_impl.h server_impl.cc test.cc  -I../easy/src/base
	g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_EPOLL -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_epoll.h reactor_impl_epoll.cc server_impl.h server_impl.cc test.cc -I../easy/src/base
	g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_POLL -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_poll.h reactor_impl_poll.cc server_impl.h server_impl.cc test.cc -I../easy/src/base
	
	you alse can use camke,just execute
	$cmake . & make 
	(2) windows
	use visual studio 2012
	
	usage:
	for example:
	int main(int __arg_num,char** args)
	{
		/*	
		g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_SELECT -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_select.h reactor_impl_select.cc server_impl.h server_impl.cc test.cc  -I../easy/src/base
		g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_EPOLL -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_epoll.h reactor_impl_epoll.cc server_impl.h server_impl.cc test.cc -I../easy/src/base
		g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_POLL -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_poll.h reactor_impl_poll.cc server_impl.h server_impl.cc test.cc -I../easy/src/base
		*/
		if(3 != __arg_num)
		{
			printf("param error,please input correct param! for example: nohup ./transform 192.168.22.63 9876 & \n");
			exit(1);
		}
	#ifdef __LINUX
		signal(SIGSEGV,dump);
	#endif // __LINUX
		char* __host = args[1];
		unsigned int __port = atoi(args[2]);
		Reactor* __reactor = Reactor::instance();
		Server_Impl __event_handle_srv(__reactor,__host,__port);
		static const int __max_time_out = 5000*1000;
		__reactor->event_loop(__max_time_out);
		return 0;
	}
	
	press test:
	(1)use echo c/s model,sends ten packets per/second,100 bytes per/packet,include 12 bytes head size and content;
	(2)test under linux,about 5 connection per/second,20000 connections in all.
	to test program' s stability and the number of connections,just include data sending and parsing,do nothing others.
	
	machine configuration:
	71
	model name      : Intel(R) Xeon(R) CPU E5-2609 0 @ 2.40GHz
	2.6.32-279.el6.x86_64 #1 SMP Fri Jun 22 12:19:21 UTC 2012 x86_64 x86_64 x86_64 GNU/Linux	
	memory:32G

	72
	model name Intel(R) Xeon(R) CPU E5-2609 0 @ 2.40GHz
	2.6.32-279.el6.x86_64 #1 SMP Fri Jun 22 12:19:21 UTC 2012 x86_64 x86_64 x86_64 GNU/Linux
	memory:32G
	
	73
	model name Intel(R) Xeon(R) CPU E5-2609 0 @ 2.40GHz
	2.6.32-279.el6.x86_64 #1 SMP Fri Jun 22 12:19:21 UTC 2012 x86_64 x86_64 x86_64 GNU/Linux
	memory:32G

	61
	model name      : Intel(R) Core(TM) i5-3470 CPU @ 3.20GHz
	2.6.32-358.el6.x86_64 #1 SMP Fri Feb 22 00:31:26 UTC 2013 x86_64 x86_64 x86_64 GNU/Linux
	memory:3G

	simple data:
	patten	connection	round trip time(microsecond)	packet per/sec	 	server	client	monitor		process packet	cpu(%)	connection per/sec	packet size(bytes)	network(MBytes)	nmon file		
	epoll 	20000		10									10				73		71,72		61			y								5				~100		 8
	
	
	more information:
		 tinynet (V1.0.0) 一个轻量级的网络库（测试报告）
		 http://blog.chinaunix.net/uid-8625039-id-4337909.html
		 
		 tinynet V1.0.1 一个轻量级的网络库（测试报告2）
		 http://blog.chinaunix.net/uid-8625039-id-4351041.html
    
    

