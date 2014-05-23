#include <Windows.h>
#include <map>
#include "reactor_impl.h"

class Reactor_Impl_Iocp : public Reactor_Impl
{
public:
	Reactor_Impl_Iocp();
	
	~Reactor_Impl_Iocp() {}
	
	int register_handle(Event_Handle* __handle,int __fd,int __mask,int __connect);
	
	int remove_handle(Event_Handle* __handle,int __mask);
	
	int handle_event(unsigned long __millisecond);
	
	int event_loop(unsigned long __millisecond);

	//	__fd is the broadcaster
	void broadcast(int __fd,const char* __data,unsigned int __length) {}

private:

	void _ready(); 

	void _create_completeion_port();

	void _associate_completeion_port(int __fd,ULONG_PTR __completion_key);

	int _get_cpu_number();

	void _begin_thread(unsigned (__stdcall * __start_address ) (void *),void* __pv);

private:
	static unsigned int __stdcall work_thread_function(void* __pv);

private:
	int								fd_;
	
	int								max_fd_;
	
	Event_Handle* 					handle_;
	
	std::map<int,Event_Handle*> 	events_;

	HANDLE							completeion_port_;
};
