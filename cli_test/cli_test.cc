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
#include "cli_test.h"
#include "reactor.h"
#include "easy_util.h"
#include "easy_byte_buffer.h"
#include "easy_time.h"

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

static std::string __random_string[] = 
{
	"[0x000085e4][T]AdvertisingIndentitifer: '', IdentifierForVendor: '', DeviceName: 'PC', ModelName: 'x86', SystemName: '', SystemVersion: '', HardwareID: '74d435046509'",
	"nice to meet you!",
	"It is the tears of the earth that keep here smiles in bloom.",
	"The mighty desert is burning for the love of a blade of grass who shakes her head and laughs and flies away.",
	"If you shed tears when you miss the sun, you also miss the stars.",
	"Her wishful face haunts my dreams like the rain at night.",
	"Once we dreamt that we were strangers.We wake up to find that we  were dear to each other.",
	"Sorrow is hushed into peace in my heart like the evening among the silent trees.",
	"Some unseen fingers, like an idle breeze, are playing upon my heart the music of the ripples.",
	"Listen, my heart, to the whispers of the world with which it makes love to you.",
	"Do not seat your love upon a precipice because it is high.",
	"I sit at my window this morning where the world like a passer-by stops for a moment, nods to me and goes.",
	"There little thoughts are the rustle of leaves; they have their whisper of joy in my mind.",
	"What you are you do not see, what you see is your shadow.",
	"My wishes are fools, they shout across thy song, my Master.Let me but listen.",
	"They throw their shadows before them who carry their lantern on their back.",
	"That I exist is  a perpetual surprise which is life.",
	"We, the rustling leaves, have a voice that answers the storms,but who are you so silent?I am a mere flower.",
	"Do not blame your food because you have no appetite.",
	"Success is not final, failure is not fatal: it is the courage to continue that counts.",
	"I cannot tell why this heart languishes in silence.It is for small needs it never asks, or knows or remembers.",
	"The bird wishes it were a cloud.The cloud wishes it were a bird."
};

static int __random_string_size = 22;

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
#ifdef __REACTOR_SINGLETON
	Reactor* __reactor = Reactor::instance();
#else
	Reactor* __reactor = new Reactor();
#endif // __REACTOR_SINGLETON
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
#ifdef __REACTOR_SINGLETON
	delete __reactor;
	__reactor = NULL;
#endif // __REACTOR_SINGLETON
	return 0;
}

