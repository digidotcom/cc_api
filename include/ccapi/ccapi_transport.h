#ifndef _CCAPI_TRANSPORT_H_
#define _CCAPI_TRANSPORT_H_

typedef enum {
    CCAPI_TRANSPORT_TCP,
    CCAPI_TRANSPORT_UDP,
    CCAPI_TRANSPORT_SMS
} ccapi_transport_t;

#include "ccapi/ccapi_transport_tcp.h"
#include "ccapi/ccapi_transport_udp.h"
#include "ccapi/ccapi_transport_sms.h"

#endif
