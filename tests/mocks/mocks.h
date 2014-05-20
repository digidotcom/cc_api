#ifndef _MOCKS_H_
#define _MOCKS_H_

#include "CppUTestExt/MockSupport.h"
#include "CppUTest/CommandLineTestRunner.h"

extern "C" {
#include "ccapi_definitions.h"
#include "ccimp/ccimp_logging.h"

extern char * assert_buffer;
extern char * assert_file;
}

extern "C" {
#include "ccapi/ccxapi.h"
}

#include "mock_ccimp_filesystem.h"
#include "mock_ccimp_logging.h"
#include "mock_ccimp_os.h"
#include "mock_connector_api.h"
#include "mock_ccimp_network_tcp.h"
#include "mock_ccimp_network_udp.h"
#include "mock_ccimp_network_sms.h"
#include "mocks_comparators.h"

void Mock_create_all(void);
void Mock_destroy_all(void);

#endif
