#ifndef socket_ex_h__
#define socket_ex_h__
#include "easy_base_type.h"
class Socket_Ex {
  public:
    static void set_noblock(easy_int32 __fd);

    static void set_reuse_addr(easy_int32 __fd);

    static void set_no_delay(easy_int32 __fd);

    static void get_usable( easy_int32 __fd, easy_ulong& __usable_size);
};
#endif // socket_ex_h__
