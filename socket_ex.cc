#include "socket_ex.h"
#include <stdio.h>
#include <stdlib.h>
#if defined __WINDOWS || defined WIN32
#include <WinSock2.h>
#elif defined __LINUX || defined __MACX
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h> 		//	gethostname
#include <netdb.h>			//	gethostbyname
#include <sys/ioctl.h>		//	ioctl
#include <execinfo.h>
#endif //   __WINDOWS

void Socket_Ex::set_noblock(easy_int32 __fd)
{
#if defined __WINDOWS || defined WIN32
	unsigned long __non_block = 1;
	if (SOCKET_ERROR == ioctlsocket(__fd, FIONBIO, &__non_block))
	{
		printf("_set_noblock() error at ioctlsocket,error code = %d\n", WSAGetLastError());
	}
#elif defined __LINUX || defined __MACX
	easy_int32 __opts = fcntl(__fd,F_GETFL);  
	if(0 > __opts)  
	{  
		perror("error at fcntl(sock,F_GETFL)");  
		exit(1);  
	}  
	__opts = __opts | O_NONBLOCK;  
	if( 0 > fcntl(__fd,F_SETFL,__opts) )  
	{  
		perror("error at fcntl(sock,F_SETFL)");  
		exit(1);  
	}  
#endif //__WINDOWS
}

void Socket_Ex::set_reuse_addr( easy_int32 __fd )
{
	easy_int32 __option_name = 1;
	if(setsockopt(__fd, SOL_SOCKET, SO_REUSEADDR, (easy_char*)&__option_name, sizeof(easy_int32)) == -1)  
	{  
		perror("setsockopt SO_REUSEADDR");  
		exit(1);  
	}  
}

void Socket_Ex::set_no_delay( easy_int32 __fd )
{
#if defined __WINDOWS || defined WIN32
	//	The Nagle algorithm is disabled if the TCP_NODELAY option is enabled 
	easy_int32 __no_delay = TRUE;
	if(SOCKET_ERROR == setsockopt( __fd, IPPROTO_TCP, TCP_NODELAY, (easy_char*)&__no_delay, sizeof(int)))
	{
		perror("setsockopt TCP_NODELAY ");  
		exit(1);  
	}
#endif //__WINDOWS
}

void Socket_Ex::get_usable( easy_int32 __fd, easy_ulong& __usable_size )
{
#if defined __WINDOWS || defined WIN32
	if(SOCKET_ERROR == ioctlsocket(__fd, FIONREAD, &__usable_size))
	{
		printf("ioctlsocket failed with error %d\n", WSAGetLastError());
	}
#elif defined __LINUX || defined __MACX
	if(ioctl(__fd,FIONREAD,&__usable_size))
	{
		perror("ioctl FIONREAD");
	}
#endif //__LINUX
}

