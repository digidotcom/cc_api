/*
 * ccapi_definitions.h
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#ifndef _CCAPI_DEFINITIONS_H_
#define _CCAPI_DEFINITIONS_H_

#include "ccimp/ccimp_types.h"
#include "connector_api.h"

#define UNUSED_ARGUMENT(a)  (void)(a)

extern ccapi_data_t * ccapi_data;

extern void ccapi_connector_thread(void * const argument);
extern connector_callback_status_t ccapi_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id, void * const data);
extern connector_handle_t ccapi_connector_handle;

#endif
