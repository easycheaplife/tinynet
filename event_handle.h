class Reactor;
class Event_Handle
{
public:

	Event_Handle(){}

	virtual ~Event_Handle() { }

	virtual int handle_input(int __fd){ return -1; }
	
	virtual int handle_output(int __fd){ return -1; }
	
	virtual int handle_exception(int __fd){ return -1; }
	
	virtual int handle_close(int __fd){ return -1; }
	
	virtual int handle_timeout(int __fd){ return -1; }
	
	Reactor* reactor() const { return reactor_; }
	
protected:
	Event_Handle(Reactor* __reactor){ reactor_ = __reactor;}
	
private:
	Reactor* reactor_;
};