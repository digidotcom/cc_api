/*
 * mocks.h
 *
 *  Created on: Mar 11, 2014
 *      Author: spastor
 */

#ifndef MOCKS_H_
#define MOCKS_H_

#include "mock_ccimp_debug.h"
#include "mock_ccimp_os.h"
#include "mock_connector_api.h"

void Mock_create_all(void);
void Mock_destroy_all(void);

extern "C" {
extern char * assert_buffer;
}

#endif /* MOCKS_H_ */
