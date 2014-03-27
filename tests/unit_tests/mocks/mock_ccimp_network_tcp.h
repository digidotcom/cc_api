#ifndef _MOCK_CCIMP_NETWORK_TCP_H_
#define _MOCK_CCIMP_NETWORK_TCP_H_

#include "CppUTestExt/MockSupport.h"

extern "C" {
#include "ccapi_definitions.h"
#include "ccimp/ccimp_os.h"
}

void Mock_ccimp_network_tcp_open_create(void);
void Mock_ccimp_network_tcp_open_destroy(void);
void Mock_ccimp_network_tcp_open_expectAndReturn(ccimp_network_open_t * expect, ccimp_status_t retval);

void Mock_ccimp_network_tcp_send_create(void);
void Mock_ccimp_network_tcp_send_destroy(void);
void Mock_ccimp_network_tcp_send_expectAndReturn(ccimp_network_send_t * expect, ccimp_status_t retval);

void Mock_ccimp_network_tcp_receive_create(void);
void Mock_ccimp_network_tcp_receive_destroy(void);
void Mock_ccimp_network_tcp_receive_expectAndReturn(ccimp_network_receive_t * expect, ccimp_status_t retval);

void Mock_ccimp_network_tcp_close_create(void);
void Mock_ccimp_network_tcp_close_destroy(void);
void Mock_ccimp_network_tcp_close_expectAndReturn(ccimp_network_close_t * expect, ccimp_status_t retval);

#endif
