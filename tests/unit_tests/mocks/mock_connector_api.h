/*
 * Mock_connector_api.h
 *
 *  Created on: Feb 14, 2014
 *      Author: spastor
 */

#ifndef _MOCK_CONNECTOR_API_H_
#define _MOCK_CONNECTOR_API_H_

extern "C" {
#include "connector_api.h"
}

void Mock_connector_init_create(void);
void Mock_connector_init_destroy(void);
void Mock_connector_init_expectAndReturn(connector_callback_t const callback, connector_handle_t retval);


#endif /* MOCK_CONNECTOR_API_H_ */
