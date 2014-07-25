// tinynet_cli.cpp : Defines the entry point for the console application.
//

#include "reactor.h"
#include "easy_util.h"
#include "client_impl.h"
#include "easy_byte_buffer.h"

int main(int argc, char* argv[])
{
	if(3 != argc)
	{
		printf("param error,please input correct param! for example: ./tinynet_cli 192.168.22.63 9876 \n");
		exit(1);
	}
	char* __host = argv[1];
	unsigned int __port = atoi(argv[2]);
	Reactor* __reactor = Reactor::instance();
	Client_Impl* client_impl_ = new Client_Impl(__reactor,__host,__port);
	
	int __log_level = 1;
	int __frame_number = 7;
	int __guid = 15;
	int __res_frane_number = 0;
	int __res_log_level = 0;
	int __head = 0;
	//	set head
	__head |= (__frame_number << 8);
	__head |= (__log_level);

	std::string __context = "[0x000085e4][T]AdvertisingIndentitifer: '', IdentifierForVendor: '', DeviceName: 'King-PC', ModelName: 'x86', SystemName: '', SystemVersion: '', HardwareID: '74d435046509'";
	int __length = __context.size();
#if 0
	static const int __data_length = 256;
	unsigned char __data[__data_length] = {};
	memcpy(__data,&__length,4);
	memcpy(__data + 4,&__head,4);
	memcpy(__data + 8,&__guid,4);
	memcpy(__data + 12,__context.c_str(),__length);
	client_impl_->write((char*)__data,__length + 12);
#else
	easy::EasyByteBuffer	__byte_buffer;
	__byte_buffer << __length;
	__byte_buffer << __head;
	__byte_buffer << __guid;
	__byte_buffer << __context;
	client_impl_->write((char*)__byte_buffer.contents(),__byte_buffer.size());
#endif
	static const int __sleep_time = 100*1000;
	while (true)
	{
		easy::Util::sleep(__sleep_time);
	}
	return 0;
}

