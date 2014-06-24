#include <string>
#include "event_handle.h"

class Event_Handle_Srv : public  Event_Handle
{
public:
	Event_Handle_Srv(Reactor* __reactor,const char* __host,unsigned int __port);

	virtual ~Event_Handle_Srv();
	
	//	-1 means error happened, 0 means no error.
	virtual int handle_input(int __fd);
	
	virtual int handle_output(int __fd);
	
	virtual int handle_exception(int __fd);
	
	//	close a socket by special fd
	virtual int handle_close(int __fd);
	
	virtual int handle_timeout(int __fd);
	
	virtual int get_handle() const { return fd_;}

	void	broadcast(int __fd,const char* __data,unsigned int __length);

	//	read data from network cache
	int	read(int __fd,char* __buf, int __length); 

	//	write data to peer 
	int write(int __fd,const char* __data, int __length);

public:
	//	pure virtual function, subclass must define it.
	virtual void on_connected(int __fd) = 0;

	virtual void on_read(int __fd) = 0;

protected:
	void 	_init();
	
	void 	_set_noblock(int __fd);

	void	_set_reuse_addr(int __fd);

	void	_get_usable( int __fd, unsigned long& __usable_size);

	void	_set_no_delay(int __fd);
private:
	int  			fd_;

	std::string		host_;

	unsigned		port_;
};