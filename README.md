tinynet
===
    tinynet is light-weighted network library specially for game server. it supports select/poll/epoll/iocp multiplexing model. you can switch any one by define different macro. 
	
compiler:
---
	make sure easy project at the same directory,easey git address:https://github.com/yuyunliuhen/easy.
##### linux
	use gcc,version 4.8.1 or later
	for example:
		g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_SELECT -o ./bin/srv_test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_select.h reactor_impl_select.cc server_impl.h server_impl.cc ./srv_test/srv_test.h ./srv_test/srv_test.cc  -I../easy/src/base -I.
		g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_EPOLL -o ./bin/srv_test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_epoll.h reactor_impl_epoll.cc server_impl.h server_impl.cc ./srv_test/srv_test.h ./srv_test/srv_test.cc -I../easy/src/base -I.
		g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_POLL -o ./bin/srv_test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_poll.h reactor_impl_poll.cc server_impl.h server_impl.cc ./srv_test/srv_test.h ./srv_test/srv_test.cc -I../easy/src/base -I.
	you alse can use camke,just execute
	$cmake . & make 
##### windows
	cmake -G "Visual Studio 11 Win64" . 
	or call build_x64/x86.bat
	
usage:
---
#### for example:
##### server:
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
		if(3 != __arg_num)
		{
			printf("param error,please input correct param! for example: nohup ./srv_test 192.168.22.63 9876 & \n");
			exit(1);
		}
		easy_char* __host = args[1];
		easy_uint32 __port = atoi(args[2]);
		Reactor* __reactor = new Reactor();
		Srv_Test __srv_test(__reactor,__host,__port);
		static const easy_int32 __max_time_out = 5000*1000;
		__reactor->event_loop(__max_time_out);
		return 0;
	}
##### client:
	class Cli_Test : public Client_Impl
	{
	public:
		Cli_Test(Reactor* __reactor,const easy_char* __host,easy_uint32 __port = 9876);

		easy_int32 handle_packet(const easy_char* __packet,easy_int32 __length);

		~Cli_Test();
	};
	
	Cli_Test::Cli_Test( Reactor* __reactor,const easy_char* __host,easy_uint32 __port /*= 9876*/ )
		: Client_Impl(__reactor,__host,__port)
	{

	}

	easy_int32 Cli_Test::handle_packet( const easy_char* __packet,easy_int32 __length )
	{
		Event_Handle_Cli::write(__packet,__length);
		return -1;
	}

	Cli_Test::~Cli_Test()
	{

	}

	int main(int argc, char* argv[])
	{
		/*
			g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_SELECT -o ./bin/cli_test  reactor.h reactor.cc event_handle.h event_handle_cli.h event_handle_cli.cc reactor_impl.h reactor_impl_select.h reactor_impl_select.cc client_impl.h client_impl.cc cli_test/cli_test.h cli_test/cli_test.cc  -I../easy/src/base -I.
		*/
		if(3 != argc)
		{
			printf("param error,please input correct param! for example: ./tinynet_cli 192.168.22.63 9876 \n");
			exit(1);
		}
		char* __host = argv[1];
		unsigned int __port = atoi(argv[2]);
		Reactor* __reactor = new Reactor();
		Cli_Test* __cli_test = new Cli_Test(__reactor,__host,__port);
		
		srand(easy::EasyTime::get_cur_sys_time());
		int __random_index = rand()%__random_string_size;
		std::string __context = __random_string[__random_index];
		unsigned short __length = __context.size();
		static const easy_int32 __head_size = sizeof(easy_uint16);
	#if 1
		static const int __data_length = 256;
		unsigned char __data[__data_length] = {};
		memcpy(__data,&__length,__head_size);
		memcpy(__data + __head_size,__context.c_str(),__length);
		__cli_test->write((char*)__data,__length + __head_size);
	#else
		easy::EasyByteBuffer	__byte_buffer;
		__byte_buffer << __length;
		__byte_buffer << __context;
		__cli_test->write((char*)__byte_buffer.contents(),__byte_buffer.size());
	#endif
		static const int __sleep_time = 100*1000;
		while (true)
		{
			easy::Util::sleep(__sleep_time);
		}
		return 0;
	}

press test
---
	(1)use echo c/s model,sends ten packets per/second,100 bytes per/packet,include 12 bytes head size and content;
	(2)test under linux,about 5 connection per/second,20000 connections in all.to test program' s stability and the number of connections,just include data sending and parsing,do nothing others.
	
##### machine configuration:
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

##### simple data:
	patten	connection	round trip time(microsecond)	packet per/sec	 	server	client	monitor		process packet	cpu(%)	connection per/sec	packet size(bytes)	network(MBytes)	nmon file		
	epoll 	20000		10									10				73		71,72		61			y								5				~100		 8
	
more information:
---
[tinynet (V1.0.0) a light-weighted network library test repoet](http://blog.chinaunix.net/uid-8625039-id-4337909.html)
		 
[tinynet V1.0.1 a light-weighted network library test repoet 2](http://blog.chinaunix.net/uid-8625039-id-4351041.html)
    
    

