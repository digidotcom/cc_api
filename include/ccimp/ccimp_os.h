/*
 * ccimp_os.h
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#ifndef CCIMP_OS_H_
#define CCIMP_OS_H_

#include "ccimp/ccimp_types.h"

typedef void (* ccimp_thread_start_t) (void * const argument);

typedef enum {
    CCIMP_THREAD_CONNECTOR_RUN
} ccimp_thread_type_t;

typedef struct
{
    ccimp_thread_type_t thread_type;
    ccimp_thread_start_t thread_start;
    void * argument;
} ccimp_create_thread_info_t;


void * ccimp_malloc(size_t size);
ccapi_bool_t ccimp_create_thread(ccimp_create_thread_info_t * const create_thread_info);

#endif
