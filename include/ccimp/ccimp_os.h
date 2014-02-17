/*
 * ccimp_os.h
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#ifndef CCIMP_OS_H_
#define CCIMP_OS_H_

#include "ccimp/ccimp_types.h"

void * ccimp_malloc(size_t size);
ccapi_bool_t ccimp_create_thread(ccimp_create_thread_info_t * const create_thread_info);

#endif
