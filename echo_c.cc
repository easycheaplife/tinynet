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
#include <stdlib.h>			//	exit
#include <netinet/in.h>		//	sockaddr_in
#include <strings.h>		//	bzero
#include <arpa/inet.h>		//	inet_addr
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
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

static int __random_string_size = 22;
static int __buf_size = 256;
static int __sleep_time = 1000*100;

void 	_set_noblock(int __fd) {
    int __opts = fcntl(__fd,F_GETFL);
    if(0 > __opts) {
        perror("error at fcntl(sock,F_GETFL)");
        exit(1);
    }
    __opts = __opts | O_NONBLOCK;
    if( 0 > fcntl(__fd,F_SETFL,__opts) ) {
        perror("error at fcntl(sock,F_SETFL)");
        exit(1);
    }
}

void output(const char* __fmt,...) {
#ifdef __DEBUG
    static const int __output_size = 1024;
    char __buf[__output_size] = {};
    va_list __args;
    va_start(__args,__fmt);
    vsnprintf(__buf, __output_size, __fmt, __args);
    va_end(__args);
    printf("%s\n",__buf);
#endif //__DEBUG
}

void test_4_transform_monitor(int sock) {
    srand( (unsigned)time(NULL));
    int __random_index = 0;

    char __send_buf[__buf_size];
    char __recv_buf[__buf_size];
    static const int __packet_head_size = sizeof(unsigned short);
    for(int __i = 0; ; ++__i) {
        __random_index = rand()%__random_string_size;
        unsigned short __length = __random_string[__random_index].size();
        memset(__send_buf,0,__buf_size);
        memcpy(__send_buf,(void*)&__length,__packet_head_size);
        strcpy(__send_buf + __packet_head_size,__random_string[__random_index].c_str());
        int send_bytes = send(sock,(void*)__send_buf,__packet_head_size + __length,0);
        if(-1 != send_bytes) {
            output("%d bytes send: %s",send_bytes,__random_string[__random_index].c_str());
        } else {
            printf("recv error,errno = %d\n",errno);
            break;
        }
        //	receive data
        unsigned short __length2 = 0;
        unsigned long __usable_size = 0;
        if(ioctl(sock,FIONREAD,&__usable_size)) {
            perror("ioctl FIONREAD");
        }
        if(__usable_size < __packet_head_size) {
            //	not enough,continue;
            usleep(__sleep_time*10);
            output("#1,__usable_size %lu\n",__usable_size);
            continue;
        }
        int recv_bytes = recv(sock,(void*)&__length2,__packet_head_size,0);
        if(0 == recv_bytes) {
            printf("The return value will be 0 when the peer has performed an orderly shutdown \n");
            break;
        } else if(-1 == recv_bytes) {
            if(EAGAIN == errno || EWOULDBLOCK == errno) {
                usleep(__sleep_time*10);
                output("#2\n");
                continue;
            } else {
                printf("recv error,errno = %d\n",errno);
                break;
            }
        }
        if(__packet_head_size != recv_bytes) {
            printf(" __packet_head_size error! %d bytes recv,sock %d\n", recv_bytes,sock);
        }
        memset(__recv_buf,0,__buf_size);
        if(ioctl(sock,FIONREAD,&__usable_size)) {
            perror("ioctl FIONREAD");
        }
        if(__usable_size < __length2) {
            //	not enough,continue;
            usleep(__sleep_time*10);
            output("#3\n");
            continue;
        }
        recv_bytes = recv(sock,(void*)__recv_buf,__length2,0);
        if(0 == recv_bytes) {
            printf("The return value will be 0 when the peer has performed an orderly shutdown \n");
            break;
        } else if(-1 == recv_bytes) {
            if(EAGAIN == errno || EWOULDBLOCK == errno) {
                usleep(__sleep_time*10);
                output("#4\n");
                continue;
            } else {
                printf("recv error,errno = %d\n",errno);
                break;
            }
        }
        output("%d bytes recv: %s",recv_bytes + __packet_head_size,__recv_buf);
        usleep(__sleep_time);
    }
}
int main(int __arg_num, char** __args) {
    if(3 != __arg_num) {
        printf("param error,please input correct param,for example : ./echo_c 192.168.22.61 9876\n");
        exit(1);
    }
    const char* __host = __args[1];
    unsigned int __port = atoi(__args[2]);
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(-1 == sock) {
        printf("error at socket,errno = %d\n",errno);
        exit(1);
    }
    struct sockaddr_in clientaddr;
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_addr.s_addr = inet_addr(__host);
    clientaddr.sin_port = htons(__port);
    int res = connect(sock,(sockaddr*)&clientaddr,sizeof(sockaddr_in));
    if(-1 == res) {
        printf("error at connect,errno = %d\n", errno);
        exit(1);
    }
    test_4_transform_monitor(sock);
}
