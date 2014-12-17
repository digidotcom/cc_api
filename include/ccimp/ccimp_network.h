/*
* Copyright (c) 2014 Etherios, a Division of Digi International, Inc.
* All rights not expressly granted are reserved.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Etherios 11001 Bren Road East, Minnetonka, MN 55343
* =======================================================================
*/

#ifndef _CCIMP_NETWORK_H_
#define _CCIMP_NETWORK_H_

#include "ccimp/ccimp_types.h"

#if (defined CCAPI_CONST_PROTECTION_UNLOCK)
#define CONST
#else
#define CONST   const
#endif

#define CCIMP_TCP_PORT      3197
#define CCIMP_SSL_PORT      3199
#define CCIMP_UDP_PORT      3297

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

ccimp_status_t ccimp_network_udp_open(ccimp_network_open_t * const data);
ccimp_status_t ccimp_network_udp_send(ccimp_network_send_t * const data);
ccimp_status_t ccimp_network_udp_receive(ccimp_network_receive_t * const data);
ccimp_status_t ccimp_network_udp_close(ccimp_network_close_t * const data);

ccimp_status_t ccimp_network_sms_open(ccimp_network_open_t * const data);
ccimp_status_t ccimp_network_sms_send(ccimp_network_send_t * const data);
ccimp_status_t ccimp_network_sms_receive(ccimp_network_receive_t * const data);
ccimp_status_t ccimp_network_sms_close(ccimp_network_close_t * const data);

#endif /* CCIMP_NETWORK_H_ */
