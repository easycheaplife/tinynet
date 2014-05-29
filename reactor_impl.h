class Event_Handle;

enum Mask
{
	kMaskRead = 0,
	kMaskWrite,
	kMaskAccept,
	//	for client socket
	kMaskConnect
};


class Reactor_Impl
{
public:
	virtual ~Reactor_Impl() {}
	
	/*
	 *	__connect: is coming connection
	 */
	virtual int register_handle(Event_Handle* __handle,int __fd,int __mask,int __connect = 0) = 0;
	
	virtual int remove_handle(Event_Handle* __handle,int __mask) = 0;
	
	virtual int handle_event(unsigned long __time) = 0;
	
	virtual int event_loop(unsigned long __time) = 0;

	virtual int handle_close(int __fd) = 0;

	//	__fd is the broadcaster
	virtual void broadcast(int __fd,const char* __data,unsigned int __length) = 0;
};
