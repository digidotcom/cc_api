/*
 * mock_ccimp_os.h
 *
 *  Created on: Feb 14, 2014
 *      Author: spastor
 */

#ifndef _MOCK_CCIMP_OS_H_
#define _MOCK_CCIMP_OS_H_

#include "CppUTestExt/MockSupport.h"

extern "C" {
#include "ccapi_definitions.h"
#include "ccimp/ccimp_os.h"
}

void Mock_ccimp_malloc_create(void);
void Mock_ccimp_malloc_destroy(void);
void Mock_ccimp_malloc_expectAndReturn(size_t expect, void * retval);

void Mock_ccimp_create_thread_create(void);
void Mock_ccimp_create_thread_destroy(void);
void Mock_ccimp_create_thread_expectAndReturn(ccimp_create_thread_info_t * const create_thread_info, ccapi_bool_t retval);

#endif
