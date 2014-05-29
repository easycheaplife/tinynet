#ifndef __LINUX
#include <WinSock.h>
#else
#include <sys/select.h>
 #include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#endif //__LINUX

#include <map>
#include <vector>
#include "reactor_impl.h"

#ifndef __USE_STD_MAP
//#define __USE_STD_MAP
#endif // !__USE_STD_MAP

//	struct forward declaration 
struct Event_Handle_Data;

class Reactor_Impl_Select : public Reactor_Impl
{
public:
	Reactor_Impl_Select();
	
	~Reactor_Impl_Select() {}
	
	int register_handle(Event_Handle* __handle,int __fd,int __mask,int __connect);
	
	int remove_handle(Event_Handle* __handle,int __mask);
	
	int handle_event(unsigned long __millisecond);

	int handle_close(int __fd);
	
	int event_loop(unsigned long __millisecond);

	void broadcast(int __fd,const char* __data,unsigned int __length);
	
	void write(int __fd,const char* __data, int __length);
private:
	fd_set 							read_set_;
	
	fd_set 							write_set_;
	
	fd_set 							excepion_set_;
	
	int								fd_;
	
	int								max_fd_;
	
	Event_Handle* 					handle_;
#ifdef __USE_STD_MAP
	//	key is fd
	std::map<int,Event_Handle*> 	events_;
#else
	std::vector<Event_Handle_Data*>	events_;
#endif // __USE_STD_MAP
};
