/****************************************************************************
 Copyright (c) 2013-2014 King Lee

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#ifndef reactor_impl_iocp_h__
#define reactor_impl_iocp_h__
/************************************************************************/
/*  
 *  bugs:
 *  #1	2014-6-6 
 *  using acceptex, when on_accept_completed called, that means a packet have received. you must add the data to input stream. 
 *  if not, you maybe think loss of data or the data of received is not in sequence.
 *  #2	2014-6-10 
 *  client send small packet, such as a int, when reading data from overlapped_puls, the overlapped_puls puls next one can not compose
 *  a complete packet.it will be a error.
 *  #3	2014-6-11 
 *  memory leaks when process packet.
 *  #4	2014-6-13
 *  client send data frequently, about thousand packets per second, more than two client,GetQueuedCompletionStatus will return timeout.
 *  up-regulation the value of PRE_POST_RECV_NUM will remission the problem. concurrent ability will be improved.
 *  #5	2014-11-21
 *  client can not recv any message but there is some data not be sent.
 *  #6	2015-1-20
 *  error code 10038 will happed when getsockopt called at listen_thread.
 */
/************************************************************************/
#ifdef WIN32

#include <winsock2.h>
#include <WinBase.h>
#include <mswsock.h>
#include "reactor_impl.h"
#include "easy_allocator.h"
#include "easy_lock.h"


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
	//	constructor function
	Reactor_Impl_Iocp();

	//	destructor function
	~Reactor_Impl_Iocp();

	//	register handle
	easy_int32 register_handle(Event_Handle* __handle,easy_int32 __fd,easy_int32 __mask,easy_int32 __connect);

	//	remove handle
	easy_int32 remove_handle(Event_Handle* __handle,easy_int32 __mask);

	//	handle event
	easy_int32 handle_event(easy_ulong __millisecond);

	//	handle connection close
	easy_int32 handle_close(easy_int32 __fd);

	//	handle packet
	easy_int32 handle_packet(easy_int32 __fd,const easy_char* __packet,easy_uint32 __length);

	//	process event loop
	easy_int32 event_loop(easy_ulong __millisecond);

	//	__fd is the broadcaster
	void broadcast(easy_int32 __fd,const easy_char* __data,easy_uint32 __length);
	//	write data to client
	BOOL write(Client_Context* __client_context,const easy_char* __data, easy_int32 __length);

public:

	//	set socket option
	void set_sock_opt();

	//	release resource
	void destoryt_net();

	//	post a accept
	void post_accept(Overlapped_Puls* __overlapped_plus);

	//post a recv
	BOOL post_recv(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

	//	post a send
	BOOL post_send(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

	//	process packet
	void process_packet(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

	//	send a client pending
	void send_pending_send(Client_Context* __client_context);

	//	send all client pending 
	void send_all_pending_send();

	//	read packet from client 
	easy_int32 read_packet(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

	//	send data to all client
	void send_2_all_client(Client_Context* __client_context,const easy_char* __data, easy_int32 __length);

	//	send data to all client
	void send_2_all_client(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

	//	send data to special client
	BOOL send_2_client(Client_Context* __client_context,const easy_char* __data, easy_int32 __length);

	//	send data to special client
	BOOL send_2_client(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

public:
	//	object allocate a function
	Overlapped_Puls* allocate_overlapped_puls(easy_int32 __buffer_len);

	//	object release function
	void release_overlapped_puls(Overlapped_Puls* __overlapped_puls);

	//	allocate a client context
	Client_Context* allocate_client_context(SOCKET __sock);

	//	release client context
	void release_client_context(Client_Context* __client_context);

	//	remove a accept from pending
	BOOL remove_pending_accept(Overlapped_Puls* __overlapped_puls);

	//	free all client context
	void free_all_client_context();

	//	free all buffer
	void free_all_overlap_puls();

	//	insert a accept to pending
	void insert_pending_accept(Overlapped_Puls* __overlapped_puls);

	//	insert a buffer to pending
	void insert_pending_send(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

	//	get the next buffer from pending
	Overlapped_Puls* get_penging_send(Client_Context* __client_context);

	//	remove a buffer from pending
	BOOL remove_pending_send(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

	//	get the next completed buffer
	Overlapped_Puls* get_next_read_overlap_puls(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

public:
	//	a error happened at connecting
	void on_connection_error(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls, easy_int32 __error);

	//	new connection accept completed
	void on_accept_completed(Overlapped_Puls* __overlapped_puls,DWORD __bytes_transferred);

	//	read operator completed
	void on_read_completed(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls,DWORD __bytes_transferred);

	//	zero byte read operator completed
	void on_zero_read_completed(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls,DWORD __bytes_transferred);

	//	write operator completed
	void on_write_completed(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls,DWORD __bytes_transferred);

	//	add a new connection
	BOOL add_connection(Client_Context* __client_context);

	//	close a connection
	void close_connection(Client_Context* __client_context);

	//	close all client connection
	void close_all_connection();

	void check_all_connection_timeout();

	//	callback function
	virtual void on_connection_established(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);

	virtual void on_connection_closing(Client_Context* __client_context,Overlapped_Puls* __overlapped_puls);
private:
	//	read for server start
	void _ready(); 

	//	create completion port
	void _create_completeion_port();

	//	associate completion port
	void _associate_completeion_port(HANDLE __completion_port,HANDLE __device,ULONG_PTR __completion_key);

	//	get the number of cpu
	easy_int32 _get_cpu_number();

	//	begin a new thread
	void _begin_thread(unsigned (__stdcall * __start_address ) (void *),void* __pv);

	//	process i/o
	void _process_io(DWORD __per_handle,Overlapped_Puls* __overlapped_puls,DWORD __bytes_transferred,easy_int32 __error);

	//	close socket
	void _close_socket(SOCKET __socket);

	//	get client context by fd
	Client_Context* _get_client_context(easy_int32 __fd);

private:
	//	thread function
	static easy_uint32 __stdcall work_thread_function(void* __pv);

	static easy_uint32 __stdcall listen_thread(void* __pv);

private:
	easy_int32						fd_;

	Event_Handle* 					handle_;

	HANDLE							completeion_port_;

	WSAEVENT						event_array_[WSA_MAXIMUM_WAIT_EVENTS];

	easy_uint32						event_total_;

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
	easy_int32						pre_post_accept_num_;

	// post recv number beforehand
	easy_int32						pre_post_recv_num_;

private:
	//lock
	easy::mutex_lock				client_context_lock_;
	easy::mutex_lock				overlap_puls_lock_;
	easy::mutex_lock				active_clienk_context_lock_;
	easy::mutex_lock				pending_accept_lock_;
	easy::mutex_lock				waiting_sendt_lock_;
	easy::mutex_lock				out_read_overlap_puls_lock_;

	easy_int32						work_thread_cur_;
};

#endif // WIN32

#endif // reactor_impl_iocp_h__