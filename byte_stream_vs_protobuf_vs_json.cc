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
   general:
	$export LD_LIBRARY_PATH=$LD_LIBRARY_PATH../easy/dep/protobuf/src/.libs:../easy/dep/jansson/src/.libs
		$../easy/dep/protobuf/src/.libs/protoc -I./ --cpp_out=. transfer.proto

	compiler:
		$g++ -g -o byte_stream_vs_protobuf_vs_json transfer.pb.h transfer.pb.cc byte_stream_vs_protobuf_vs_json.cc -I../easy/dep/protobuf/src/ -I../easy/dep/jansson/src/ -L../easy/dep/protobuf/src/.libs -L../easy/dep/jansson/src/.libs -lprotobuf -ljansson
 ****************************************************************************/
#include <stdlib.h>			//	exit
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <sys/time.h>
#include <string.h>
#include "transfer.pb.h"
#include <jansson.h>
static std::string __random_string[] = {
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

static const int __loop_count = 10000;
static int __buf_size = 256;
void byte_stream_test() {
    int __guid = 15;
    int __head = 567;
    int __length2 = 0;
    int __head2 = 0;
    int __guid2 = 0;
    char __buf[__buf_size];
    int __packet_head_size = 12;
    int __random_index = 0;
    unsigned char __packet_head[__packet_head_size];
    int __length = __random_string[__random_index].size();

    struct timeval __start_timeval;
    gettimeofday(&__start_timeval, NULL);
    long __start_time = __start_timeval.tv_usec;
    for(int __i = 0; __i < __loop_count; ++__i) {
        memset(__buf,0,__buf_size);
        memset(__packet_head,0,__packet_head_size);
        //	pack
        memcpy(__buf,(void*)&__length,4);
        memcpy(__buf + 4,(void*)&__head,4);
        memcpy(__buf + 8,(void*)&__guid,4);
        strcpy(__buf + __packet_head_size,__random_string[__random_index].c_str());
        //	unpack
        memcpy(&__length2,(void*)__packet_head,4);
        memcpy(&__head2,(void*)(__packet_head + 4),4);
        memcpy(&__guid2,(void*)(__packet_head + 8),4);
    }
    struct timeval __end_timeval;
    gettimeofday(&__end_timeval, NULL);
    long __end_time = __end_timeval.tv_usec;
    long __time_total = __end_time - __start_time;
    printf("start time = %ld, end time = %ld,byte stream total time = %ld\n",__start_time,__end_time,__time_total);
}

void protobuf_test() {
    transfer::Packet __packet_protobuf;
    std::string __string_packet;
    int __guid = 15;
    int __head = 567;
    int __length2 = 0;
    int __head2 = 0;
    int __guid2 = 0;
    char __buf[__buf_size];
    int __packet_head_size = 4;
    int __random_index = 0;
    unsigned char __packet_head[__packet_head_size];

    struct timeval __start_timeval;
    gettimeofday(&__start_timeval, NULL);
    long __start_time = __start_timeval.tv_usec;
    for(int __i = 0; __i < __loop_count; ++__i) {
        memset(__buf,0,__buf_size);
        //	pack
        __packet_protobuf.Clear();
        __string_packet.clear();
        __packet_protobuf.set_head(__head);
        __packet_protobuf.set_guid(__guid);
        __packet_protobuf.set_content(__random_string[__random_index]);
        __packet_protobuf.SerializeToString(&__string_packet);
        int __length = __string_packet.length();
        memcpy(__buf,(void*)&__length,4);

        //	unpack
        __packet_protobuf.Clear();
        __packet_protobuf.ParseFromString(__string_packet);
        __head2 = __packet_protobuf.head();
        __guid2 = __packet_protobuf.guid();
    }
    struct timeval __end_timeval;
    gettimeofday(&__end_timeval, NULL);
    long __end_time = __end_timeval.tv_usec;
    long __time_total = __end_time - __start_time;
    printf("start time = %ld, end time = %ld,protobuf total time = %ld\n",__start_time,__end_time,__time_total);
}

void json_test() {
    int __guid = 15;
    int __head = 567;
    int __length2 = 0;
    int __head2 = 0;
    int __guid2 = 0;
    char __buf[__buf_size];
    int __random_index = 0;

    struct timeval __start_timeval;
    gettimeofday(&__start_timeval, NULL);
    long __start_time = __start_timeval.tv_usec;
    for(int __i = 0; __i < __loop_count; ++__i) {
        //	pack
        json_t* __json_msg = json_object();
        json_t* __json_head = json_integer(__head);
        json_t* __json_guid = json_integer(__guid);
        json_t* __json_content = json_string(__random_string[__random_index].c_str());
        json_object_set(__json_msg, "head", __json_head);
        json_object_set(__json_msg, "guid", __json_guid);
        json_object_set(__json_msg, "content", __json_content);
        char* __json_dumps_string = json_dumps(__json_msg,0);
        int __length = strlen(__json_dumps_string);
        json_decref(__json_head);
        json_decref(__json_guid);
        json_decref(__json_content);
        json_decref(__json_msg);

        //	unpack
        json_error_t* __json_error = NULL;
        json_t* __json_loads = json_loads(__json_dumps_string,JSON_DECODE_ANY,__json_error);
        json_t* __json_loads_head = json_object_get(__json_loads,"head");
        json_t* __json_loads_guid = json_object_get(__json_loads,"guid");
        __head2 = json_integer_value(__json_loads_head);
        __guid2 = json_integer_value(__json_loads_guid);
        json_decref(__json_loads);
        free(__json_dumps_string);
    }
    struct timeval __end_timeval;
    gettimeofday(&__end_timeval, NULL);
    long __end_time = __end_timeval.tv_usec;
    long __time_total = __end_time - __start_time;
    printf("start time = %ld, end time = %ld,json total time = %ld\n",__start_time,__end_time,__time_total);
}

int main(int __arg_num, char** __args) {
    byte_stream_test();
    protobuf_test();
    json_test();
}
