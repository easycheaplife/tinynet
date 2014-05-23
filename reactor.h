class Event_Handle;
class Reactor_Impl;
class Reactor
{
public:
	Reactor();
	
	~Reactor();
	
	static Reactor* instance();

	static void destory();
	
	int register_handle(Event_Handle* __handle,int __fd,int __mask);
	
	int remove_handle(Event_Handle* __handle,int __mask);
	
	int handle_event(unsigned long __millisecond);
	
	int event_loop(unsigned long __millisecond);
	
	Reactor_Impl* reactor_impl() const { return reactor_impl_; }
private:
	Reactor(const Reactor&);
	
	Reactor operator = (const Reactor&);
	
	static Reactor* 				reactor_;

	Reactor_Impl*					reactor_impl_;
	
};