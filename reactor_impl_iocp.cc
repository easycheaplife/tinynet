#include <process.h>
#include <WinSock2.h>
#include "reactor_impl_iocp.h"
#include "event_handle.h"

//	implement SOCKET pool instead of PER_HANDLE_DATA
struct Client_Context
{
	//	accept socket
	SOCKET fd_;
};

Reactor_Impl_Iocp::Reactor_Impl_Iocp()
{
	handle_ = NULL;
	fd_ = -1;
	max_fd_ = -1;
	_create_completeion_port();
}

int Reactor_Impl_Iocp::register_handle(Event_Handle* __handle,int __fd,int __mask,int __connect)
{
	if(kMaskAccept ==__mask)
	{
		fd_ = __fd;
		handle_ = __handle;
		_ready();
	}
	else
	{
		if(1 == __connect)
		{
			events_.insert(std::map<int,Event_Handle*>::value_type(__fd,__handle));
		}
	}
	if(max_fd_ < __fd)
	{
		max_fd_ = __fd;
	}
	return -1;
}

int Reactor_Impl_Iocp::remove_handle(Event_Handle* __handle,int __mask)
{
	return -1;
}

int Reactor_Impl_Iocp::handle_event(unsigned long __millisecond)
{
	return -1;
}

int Reactor_Impl_Iocp::event_loop(unsigned long __millisecond)
{
	while(true)
	{
		SOCKET __fd_accept = WSAAccept(fd_,NULL,NULL,NULL,0);
		Client_Context* __client_context = new Client_Context();
		__client_context->fd_ = __fd_accept;
		_associate_completeion_port(__fd_accept,(DWORD)__client_context);
	}
	return -1;
}

void Reactor_Impl_Iocp::_create_completeion_port()
{
	//	if the NumberOfConcurrentThreads is 0,that means io completion port will use default value,the number of cpu ' thread.
	completeion_port_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if(NULL == completeion_port_)
	{
		printf("_create_completeion_port failed with error: %d\n", GetLastError());
	}
}

void Reactor_Impl_Iocp::_associate_completeion_port( int __fd,DWORD __completion_key)
{
	HANDLE __completeion_port = CreateIoCompletionPort((HANDLE)__fd, completeion_port_, __completion_key, 0);
	if(NULL == __completeion_port)
	{
		printf("_associate_completeion_port failed with error: %d\n", GetLastError());
	}
}

void Reactor_Impl_Iocp::_ready()
{
	_associate_completeion_port(fd_,0);
	int __number_work_thread = _get_cpu_number();
	for(int __i = 0; __i < __number_work_thread; ++__i)
	{
		_begin_thread(&work_thread_function,this);
	}
}

int Reactor_Impl_Iocp::_get_cpu_number()
{
	SYSTEM_INFO __systemInfo;
	ZeroMemory(&__systemInfo,sizeof(__systemInfo));
	//	determine how many processors are on the system.
	GetSystemInfo(&__systemInfo);

	return __systemInfo.dwNumberOfProcessors;
}

void Reactor_Impl_Iocp::_begin_thread(unsigned (__stdcall * __start_address) (void *),void* __pv)
{
	unsigned int __thread_id = 0;
	uintptr_t __res = _beginthreadex( NULL, 0, __start_address, __pv, 0, &__thread_id );
	if (0 == __res)
	{
		printf("_beginthreadex exception!");
		return;
	}
	HANDLE __work_thread = (HANDLE)__res;
	try
	{ 
		if (__work_thread) 
		{ 
			CloseHandle(__work_thread);
			(__work_thread) = NULL; 
		} 
	} 
	catch(...) 
	{
		printf("CloseHandle error\n");
	}
}

unsigned int __stdcall Reactor_Impl_Iocp::work_thread_function( void* __pv )
{
	Reactor_Impl_Iocp* __this = (Reactor_Impl_Iocp*)__pv;
	DWORD __bytes_transferred = 0;
	DWORD __per_handle = 0;
	LPOVERLAPPED __overlapped;
	while (true)
	{
		BOOL __res = GetQueuedCompletionStatus(__this->completeion_port_, &__bytes_transferred,(LPDWORD)&__per_handle,(LPOVERLAPPED*)&__overlapped, INFINITE);
		DWORD __io_error = ::WSAGetLastError();
		if(!__res)
		{
			
		}
	}
	return 0;
}

int Reactor_Impl_Iocp::handle_close( int __fd )
{
	return -1;
}
