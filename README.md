    tinynet是一个轻量级的跨平台的网络库，支持select,poll,epoll,iocp多种网络模型，可以通过宏方便的切换各种模型。
    使用：
	/*	
	g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_SELECT -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_select.h reactor_impl_select.cc server_impl.h server_impl.cc test.cc 
	g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_EPOLL -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_epoll.h reactor_impl_epoll.cc server_impl.h server_impl.cc test.cc
	g++ -g -Wl,--no-as-needed -std=c++11 -pthread -D__LINUX -D__HAVE_POLL -o test reactor.h reactor.cc event_handle.h event_handle_srv.h event_handle_srv.cc reactor_impl.h reactor_impl_poll.h reactor_impl_poll.cc server_impl.h server_impl.cc test.cc
	*/
	if(3 != __arg_num)
	{
		printf("param error,please input correct param! for example: nohup ./transform 192.168.22.63 9876 & \n");
		exit(1);
	}
	char* __host = args[1];
	unsigned int __port = atoi(args[2]);
	Reactor* __reactor = Reactor::instance();
	Server_Impl __event_handle_srv(__reactor,__host,__port);
	__reactor->event_loop(5000*1000);
	
	以linux为例，-D__HAVE_SELECT则代表使用select,-D__HAVE_EPOLL则使用epoll.当然，工程目录中有cmakelist,可以通过cmake来构建自己的项目。测试主要以epoll为例，它是linux下性能最高的IO多路复用模型。
	测试为echo模型，客户端每秒发10个数据包，每个数据包约100bytes,数据包括包头以及数据，包头12bytes,包括长度以及其他8bytes信息。客户端连接为每秒5个，此次测试总计连接15000个，主要测试连接数以及稳定性。服务器包含数据包的解析以及发送，
暂时不包含其他的逻辑。接收数据缓存，使用环形缓存，读写分两个线程，不加锁。发送数据不缓存，直接发送。
	测试机器配置：
	71
	model name      : Intel(R) Xeon(R) CPU E5-2609 0 @ 2.40GHz
	2.6.32-279.el6.x86_64 #1 SMP Fri Jun 22 12:19:21 UTC 2012 x86_64 x86_64 x86_64 GNU/Linux	
	memory:32G

	72
	model name Intel(R) Xeon(R) CPU E5-2609 0 @ 2.40GHz
	2.6.32-279.el6.x86_64 #1 SMP Fri Jun 22 12:19:21 UTC 2012 x86_64 x86_64 x86_64 GNU/Linux
	memory:32G

	61
	model name      : Intel(R) Core(TM) i5-3470 CPU @ 3.20GHz
	2.6.32-358.el6.x86_64 #1 SMP Fri Feb 22 00:31:26 UTC 2013 x86_64 x86_64 x86_64 GNU/Linux
	memory:3G

	简单数据：
	patten	connection	round trip time(microsecond)	packet per/sec	 	server	client	monitor		process packet	cpu(%)	connection per/sec	packet size(bytes)	network(Kbytes)	nmon file		
	epoll 	15000		150000~170000						10				72		71		61			y								5				~100				12000~13000
	
	
	round trip time 为客户端服务器数据包往返时间，约为150ms,网络流量在12Mbytes左右，对于千兆(1000Mbits = 125Mbytes)来说，应该还有很大的空间,详细数据见nmon文档（report/epoll15000/kakao2_140703_1122.xlsx），
	测试数据仅是个人主观的数据，不一定非常准确，仅供参考 ^_^
	
	
	编译时，确保easy文件夹与tinynet在同一目录下， easy代码地址：https://github.com/yuyunliuhen/easy。
    
    

