/*
 * mocks.h
 *
 *  Created on: Mar 11, 2014
 *      Author: spastor
 */

#ifndef MOCKS_H_
#define MOCKS_H_

#include "mock_ccimp_logging.h"
#include "mock_ccimp_os.h"
#include "mock_connector_api.h"
#include "mock_ccimp_network_tcp.h"

void Mock_create_all(void);
void Mock_destroy_all(void);

extern MockFunctionComparator ccimp_network_open_t_comparator;
extern MockFunctionComparator ccimp_network_send_t_comparator;
extern MockFunctionComparator ccimp_network_receive_t_comparator;
extern MockFunctionComparator ccimp_network_close_t_comparator;
extern MockFunctionComparator ccimp_create_thread_info_t_comparator;
extern MockFunctionComparator connector_transport_t_comparator;
extern MockFunctionComparator connector_initiate_stop_request_t_comparator;

extern "C" {
extern char * assert_buffer;
extern char * assert_file;
}

#endif /* MOCKS_H_ */
