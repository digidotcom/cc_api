#ifndef _CCIMP_NETWORK_H_
#define _CCIMP_NETWORK_H_

#include "ccimp/ccimp_types.h"

#if (defined CCAPI_CONST_PROTECTION_UNLOCK)
#define CONST
#else
#define CONST   const
#endif

typedef void * ccimp_network_handle_t;

typedef struct  {
    union
    {
        char const * CONST url;
        char const * CONST phone;
    } device_cloud;
    ccimp_network_handle_t handle;
} ccimp_network_open_t;

typedef struct  {
    ccimp_network_handle_t CONST handle;
    void const * CONST buffer;
    size_t CONST bytes_available;
    size_t bytes_used;
} ccimp_network_send_t;

typedef struct  {
    ccimp_network_handle_t CONST handle;
    void * CONST buffer;
    size_t CONST bytes_available;
    size_t bytes_used;
} ccimp_network_receive_t;

typedef struct  {
    ccimp_network_handle_t CONST handle;
} ccimp_network_close_t;

ccimp_status_t ccimp_network_tcp_open(ccimp_network_open_t * const data);
ccimp_status_t ccimp_network_tcp_send(ccimp_network_send_t * const data);
ccimp_status_t ccimp_network_tcp_receive(ccimp_network_receive_t * const data);
ccimp_status_t ccimp_network_tcp_close(ccimp_network_close_t * const data);

#endif /* CCIMP_NETWORK_H_ */
