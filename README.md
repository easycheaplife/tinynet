    tinynet��һ���������Ŀ�ƽ̨������⣬֧��select,poll,epoll,iocp��������ģ�ͣ�����ͨ���귽����л�����ģ�͡�
    ʹ�ã�
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
	
	��linuxΪ����-D__HAVE_SELECT�����ʹ��select,-D__HAVE_EPOLL��ʹ��epoll.��Ȼ������Ŀ¼����cmakelist,����ͨ��cmake�������Լ�����Ŀ��������Ҫ��epollΪ��������linux��������ߵ�IO��·����ģ�͡�
	����Ϊechoģ�ͣ��ͻ���ÿ�뷢10�����ݰ���ÿ�����ݰ�Լ100bytes,���ݰ�����ͷ�Լ����ݣ���ͷ12bytes,���������Լ�����8bytes��Ϣ���ͻ�������Ϊÿ��5�����˴β����ܼ�����15000������Ҫ�����������Լ��ȶ��ԡ��������������ݰ��Ľ����Լ����ͣ�
��ʱ�������������߼����������ݻ��棬ʹ�û��λ��棬��д�������̣߳����������������ݲ����棬ֱ�ӷ��͡�
	���Ի������ã�
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

	�����ݣ�
	patten	connection	round trip time(microsecond)	packet per/sec	 	server	client	monitor		process packet	cpu(%)	connection per/sec	packet size(bytes)	network(Kbytes)	nmon file		
	epoll 	15000		150000~170000						10				72		71		61			y								5				~100				12000~13000
	
	
	round trip time Ϊ�ͻ��˷��������ݰ�����ʱ�䣬ԼΪ150ms,����������12Mbytes���ң�����ǧ��(1000Mbits = 125Mbytes)��˵��Ӧ�û��кܴ�Ŀռ�,��ϸ���ݼ�nmon�ĵ���report/epoll15000/kakao2_140703_1122.xlsx����
	�������ݽ��Ǹ������۵����ݣ���һ���ǳ�׼ȷ�������ο� ^_^
	
	
	����ʱ��ȷ��easy�ļ�����tinynet��ͬһĿ¼�£� easy�����ַ��https://github.com/yuyunliuhen/easy��
    
    

