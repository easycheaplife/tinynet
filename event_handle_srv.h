#include "event_handle.h"

class Event_Handle_Srv : public  Event_Handle
{
public:
	Event_Handle_Srv(Reactor* __reactor);
	
	virtual int handle_input(int __fd);
	
	virtual int handle_output(int __fd);
	
	virtual int handle_exception(int __fd);
	
	virtual int handle_close(int __fd);
	
	virtual int handle_timeout(int __fd);
	
	virtual int get_handle() const { return fd_;}
	
private:
	void 	_init(unsigned int __port = 9876);
	
	void 	_set_noblock(int __fd);
	
private:
	int  	fd_;
};