/*
 * mock_ccimp_os.h
 *
 *  Created on: Feb 14, 2014
 *      Author: spastor
 */

#ifndef _MOCK_CCIMP_OS_H_
#define _MOCK_CCIMP_OS_H_

enum {
    MOCK_MALLOC_DISABLED,
    MOCK_MALLOC_ENABLED	/* Mock disabled. Do malloc normally */
};

enum {
    MOCK_FREE_DISABLED,
    MOCK_FREE_ENABLED_CHECK_PARAMETER,
    MOCK_FREE_ENABLED_DONT_CHECK_PARAMETER,
    MOCK_FREE_ENABLED_NOT_EXPECTED
};

enum {
    MOCK_SYNCR_DISABLED,
    MOCK_SYNCR_ENABLED
};

typedef enum {
    MOCK_THREAD_DISABLED,                   /* Mock disabled. Create thread normally */
    MOCK_THREAD_ENABLED_NORMAL,             /* Mock enabled. Create thread normally */
    MOCK_THREAD_ENABLED_DONT_CREATE_THREAD, /* Mock enabled. Don't create thread, return FALSE */
    MOCK_THREAD_ENABLED_ARGUMENT_NULL      /* Mock enabled. Create thread setting argument to NULL */
} mock_thread_bahavior_t;

void Mock_ccimp_os_malloc_create(void);
void Mock_ccimp_os_malloc_destroy(void);
void Mock_ccimp_os_malloc_expectAndReturn(size_t expect, void * retval);

void Mock_ccimp_os_free_create(void);
void Mock_ccimp_os_free_destroy(void);
void Mock_ccimp_os_free_expectAndReturn(void * ptr, ccimp_status_t retval);
void Mock_ccimp_os_free_notExpected(void);

void Mock_ccimp_os_create_thread_create(void);
void Mock_ccimp_os_create_thread_destroy(void);
void Mock_ccimp_os_create_thread_expectAndReturn(ccimp_create_thread_info_t * const create_thread_info, mock_thread_bahavior_t behavior, ccimp_status_t retval);

void Mock_ccimp_os_get_system_time_create(void);
void Mock_ccimp_os_get_system_time_destroy(void);
void Mock_ccimp_os_get_system_time_return(unsigned long retval);

void Mock_ccimp_os_syncr_create_create(void);
void Mock_ccimp_os_syncr_create_destroy(void);
void Mock_ccimp_os_syncr_create_return(unsigned long retval);

void Mock_ccimp_os_syncr_acquire_create(void);
void Mock_ccimp_os_syncr_acquire_destroy(void);
void Mock_ccimp_os_syncr_acquire_return(unsigned long retval);

void Mock_ccimp_os_syncr_release_create(void);
void Mock_ccimp_os_syncr_release_destroy(void);
void Mock_ccimp_os_syncr_release_return(unsigned long retval);

extern "C" {
ccimp_status_t ccimp_os_malloc_real(ccimp_os_malloc_t * malloc_info);
ccimp_status_t ccimp_os_free_real(ccimp_os_free_t * free_info);
ccimp_status_t ccimp_os_realloc_real(ccimp_os_realloc_t * free_info);
ccimp_status_t ccimp_os_create_thread_real(ccimp_create_thread_info_t * const create_thread_info);
ccimp_status_t ccimp_os_get_system_time_real(ccimp_os_system_up_time_t * const system_up_time);
ccimp_status_t ccimp_os_yield_real(void);
ccimp_status_t ccimp_os_syncr_create_real(ccimp_os_syncr_create_t * const data);
ccimp_status_t ccimp_os_syncr_acquire_real(ccimp_os_syncr_acquire_t * const data);
ccimp_status_t ccimp_os_syncr_release_real(ccimp_os_syncr_release_t * const data);

}

#endif
