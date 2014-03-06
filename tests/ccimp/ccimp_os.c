/*
 * ccimp_os.c
 *
 *  Created on: Mar 5, 2014
 *      Author: hbujanda
 */

#include "ccapi_definitions.h"
#include "ccimp/ccimp_os.h"

#include <malloc.h>
#include <pthread.h>
#include <unistd.h>

#ifdef UNIT_TEST
#define ccimp_malloc             ccimp_malloc_real
#define ccimp_create_thread      ccimp_create_thread_real
#define ccimp_os_get_system_time ccimp_os_get_system_time_real
#define ccimp_os_yield           ccimp_os_yield_real
#endif

/******************** LINUX IMPLEMENTATION ********************/

ccimp_status_t ccimp_malloc(ccimp_malloc_t * malloc)
{
    malloc->ptr = calloc(1, malloc->size);

    return malloc->ptr == NULL ? CCIMP_STATUS_ABORT : CCIMP_STATUS_OK;
}

static void * thread_wrapper(void * argument)
{
    ccimp_create_thread_info_t * create_thread_info = (ccimp_create_thread_info_t *)argument;

    create_thread_info->start(create_thread_info->argument);

    return NULL;
}

ccimp_status_t ccimp_create_thread(ccimp_create_thread_info_t * const create_thread_info)
{
    pthread_t pthread;
    int ccode = pthread_create(&pthread, NULL, thread_wrapper, create_thread_info);

    if (ccode != 0)
    {
        printf("ccimp_create_thread() error %d\n", ccode);
        return (CCIMP_STATUS_ABORT);
    }

    return CCIMP_STATUS_OK;
}

ccimp_status_t ccimp_os_get_system_time(ccimp_os_system_up_time_t * const system_up_time)
{
    static time_t start_system_up_time;
    time_t present_time;

    time(&present_time);

    if (start_system_up_time == 0)
       start_system_up_time = present_time;

    present_time -= start_system_up_time;
    system_up_time->sys_uptime = (unsigned long) present_time;

    return CCIMP_STATUS_OK;
}

ccimp_status_t ccimp_os_yield(void)
{
    int error;

    error = sched_yield();
    if (error)
    {
        /* In the Linux implementation this function always succeeds */
        printf("app_os_yield: sched_yield failed with %d\n", error);
    }

    return CCIMP_STATUS_OK;
}
