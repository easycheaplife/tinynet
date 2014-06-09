#ifndef reactor_impl_iocp_h__
#define reactor_impl_iocp_h__
/************************************************************************/
/*  
 *  bugs:
 *  #1	using acceptex, when on_accept_completed called, that means a packet have received. you must add the data to input stream. 
 *  if not, you maybe think loss of data or the data of received is not in sequence.(2014-6-6)
 *  #2	client send small packet, such as a int, when reading data from overlapped_puls, the overlapped_puls puls next one can not compose
 *  a complete packet.it will be a error.(2014-6-6 not fixed)(2014-6-9 fixed)
 */
/************************************************************************/
#include <winsock2.h>
#include <WinBase.h>
#include <mswsock.h>
#include "reactor_impl.h"
#include "easy_allocator.h"

#ifndef __USE_CRITICAL_SECTION
#define __USE_CRITICAL_SECTION
#endif // __USE_CRITICAL_SECTION

#define DATA_BUFSIZE					8192
#define POOL_SIZE						1024

enum kOPType
{
	OP_ACCEPT = kMaskAccept,		//	the flag of accept a new connect
	OP_READ = kMaskRead,			//	the flag of post a read
	OP_WRITE = kMaskWrite,			//	the flag of post a write
	OP_ZERO_READ,					//	the flag of post a zero read
};

struct Client_Context;
struct Overlapped_Puls;

class Reactor_Impl_Iocp : public Reactor_Impl
{
public:
	Reactor_Impl_Iocp();

	~Reactor_Impl_Iocp();

	int register_handle(Event_Handle* __handle,int __fd,int __mask,int __connect);

	int remove_handle(Event_Handle* __handle,int __mask);

	int handle_event(unsigned long __millisecond);

	int handle_close(int __fd);

	int event_loop(unsigned long __millisecond);

	//	__fd is the broadcaster
	void broadcast(int __fd,const char* __data,unsigned int __length);

	BOOL write(Client_Context* __client_context,const char* __data, int __length);

public:

	void set_sock_opt();

	void destoryt_net();

	void post_accept(Overlapped_Puls* __overlapped_plus);

	BOOL post_recv(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

	BOOL post_send(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

	void process_packet(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

	void send_pending_send(Client_Context* __client_context);

	int read_packet(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

	void send_2_all_client(const char* __data, int __length);

	void send_2_all_client(Overlapped_Puls* __overlapped_puls);

	BOOL send_2_client(Client_Context* __client_context,const char* __data, int __length);

	BOOL send_2_client(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

public:
	//	object allocate and release function
	Overlapped_Puls* allocate_overlapped_puls(int __buffer_len);

	void release_overlapped_puls(Overlapped_Puls* __overlapped_puls);

	Client_Context* allocate_client_context(SOCKET __sock);

	void release_client_context(Client_Context* __client_context);

	BOOL remove_pending_accept(Overlapped_Puls* __overlapped_puls);

	void free_all_client_context();

	void free_all_overlap_puls();

	void insert_pending_accept(Overlapped_Puls* __overlapped_puls);

	void insert_pending_send(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

	Overlapped_Puls* get_penging_send(Client_Context* __client_context);

	BOOL remove_pending_send(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

	Overlapped_Puls* get_next_read_overlap_puls(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

public:
	void on_connection_error(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls, int __error);

	void on_accept_completed(Overlapped_Puls* __overlapped_puls,DWORD __bytes_transferred);

	void on_read_completed(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls,DWORD __bytes_transferred);

	void on_zero_read_completed(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls,DWORD __bytes_transferred);

	void on_write_completed(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls,DWORD __bytes_transferred);

	BOOL add_connection(Client_Context* __client_context);

	void close_connection(Client_Context* __client_context);

	//	close all client connection
	void close_all_connection();

	void check_all_connection_timeout();

	virtual void on_connection_established(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

	virtual void on_connection_closing(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);
private:

	void _ready(); 

	void _create_completeion_port();

	void _associate_completeion_port(HANDLE __completion_port,HANDLE __device,ULONG_PTR __completion_key);

	int _get_cpu_number();

	void _begin_thread(unsigned (__stdcall * __start_address ) (void *),void* __pv);

	void _process_io(DWORD __per_handle,Overlapped_Puls* __overlapped_puls,DWORD __bytes_transferred,int __error);

	void _close_socket(SOCKET __socket);
private:
	static unsigned int __stdcall work_thread_function(void* __pv);

	static unsigned int __stdcall listen_thread(void* __pv);

private:
	int								fd_;

	Event_Handle* 					handle_;

	HANDLE							completeion_port_;

	WSAEVENT						event_array_[WSA_MAXIMUM_WAIT_EVENTS];

	unsigned int					event_total_;

	//	for AcceptEx
	LPFN_ACCEPTEX					lpfn_acceptex_;

	//	for GetAcceptExSockaddrs
	LPFN_GETACCEPTEXSOCKADDRS		lpfn_get_acceptex_sockaddrs_; 

private:
	//	data of memery manager
	//	list of free Overlapped_Puls,it use for memory manager,if the free list is null,allocate a  new buffer,else get from free list
	Overlapped_Puls*				free_overlap_puls_;
	LONG							free_overlap_puls_count_;
	LONG							max_free_overlap_puls_count_;

	//	list of free client connection 
	Client_Context*					free_client_context_;
	LONG							free_cleint_context_count_;
	LONG							max_free_client_context_count_;

	//	list of active client connection 
	Client_Context*					active_cleint_context_;
	//	current client connection
	LONG 							cur_connection_;
	//	max client connection
	LONG							max_connection_;

	//	list of pending accept 
	Overlapped_Puls*				penging_accept_overlap_puls_;

	//	current pending accept number
	LONG							pending_accept_count_;

	// post accept number beforehand
	int								pre_post_accept_num_;

	// post recv number beforehand
	int								pre_post_recv_num_;

private:
	//lock
	easy::mutex_lock				client_context_lock_;
	easy::mutex_lock				overlap_puls_lock_;
	easy::mutex_lock				active_clienk_context_lock_;
	easy::mutex_lock				pending_accept_lock_;
	easy::mutex_lock				waiting_sendt_lock_;
	easy::mutex_lock				out_read_overlap_puls_lock_;

	int								work_thread_cur_;
};

#endif // reactor_impl_iocp_h__