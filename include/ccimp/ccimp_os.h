/*
 * ccimp_os.h
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#ifndef CCIMP_OS_H_
#define CCIMP_OS_H_

#include "ccimp/ccimp_types.h"

#if (defined CCAPI_CONST_PROTECTION_UNLOCK)
#define CONST
#else
#define CONST   const
#endif

typedef void (* ccimp_thread_start_t) (void * const argument);

typedef enum {
    CCIMP_THREAD_CONNECTOR_RUN
} ccimp_thread_type_t;

typedef struct
{
    ccimp_thread_type_t type;
    ccimp_thread_start_t start;
    void * argument;
} ccimp_create_thread_info_t;

typedef struct {
    size_t CONST size;
    void * ptr;
} ccimp_malloc_t;

typedef struct {
    void * ptr;
} ccimp_free_t;

typedef struct {
    size_t CONST old_size;
    size_t CONST new_size;
    void * ptr;
} ccimp_realloc_t;

typedef struct {
    unsigned long sys_uptime;
} ccimp_os_system_up_time_t;

#define OS_SYNCR_ADQUIRE_NOWAIT             ((unsigned long) 0)
#define OS_SYNCR_ADQUIRE_INFINITE           ((unsigned long)-1)

typedef struct {
    void * syncr_object;
} ccimp_os_syncr_create_t;

typedef struct {
    void * CONST syncr_object;
    unsigned long CONST timeout_ms;
    ccapi_bool_t acquired;
} ccimp_os_syncr_adquire_t;

typedef struct {
    void * CONST syncr_object;
} ccimp_os_syncr_release_t;

typedef struct {
    void * CONST syncr_object;
} ccimp_os_syncr_destroy_t;

ccimp_status_t ccimp_malloc(ccimp_malloc_t * malloc_info);
ccimp_status_t ccimp_free(ccimp_free_t * free_info);
ccimp_status_t ccimp_realloc(ccimp_realloc_t * realloc_info);

ccimp_status_t ccimp_create_thread(ccimp_create_thread_info_t * const create_thread_info);

ccimp_status_t ccimp_os_get_system_time(ccimp_os_system_up_time_t * const system_up_time);
ccimp_status_t ccimp_os_yield(void);

ccimp_status_t ccimp_os_syncr_create(ccimp_os_syncr_create_t * const data);
ccimp_status_t ccimp_os_syncr_adquire(ccimp_os_syncr_adquire_t * const data);
ccimp_status_t ccimp_os_syncr_release(ccimp_os_syncr_release_t const * const data);
ccimp_status_t ccimp_os_syncr_destroy(ccimp_os_syncr_destroy_t const * const data);


#endif
