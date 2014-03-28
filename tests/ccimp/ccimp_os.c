/*
 * ccimp_os.c
 *
 *  Created on: Mar 5, 2014
 *      Author: hbujanda
 */
#include "ccimp/ccimp_os.h"

#include <malloc.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>

#ifdef UNIT_TEST
#define ccimp_malloc             ccimp_malloc_real
#define ccimp_free               ccimp_free_real
#define ccimp_realloc            ccimp_realloc_real
#define ccimp_create_thread      ccimp_create_thread_real
#define ccimp_os_get_system_time ccimp_os_get_system_time_real
#define ccimp_os_yield           ccimp_os_yield_real
#endif

#define ccapi_logging_line_info(message) /* TODO */

/******************** LINUX IMPLEMENTATION ********************/

ccimp_status_t ccimp_malloc(ccimp_malloc_t * const malloc_info)
{
    malloc_info->ptr = malloc(malloc_info->size);

    return malloc_info->ptr == NULL ? CCIMP_STATUS_ABORT : CCIMP_STATUS_OK;
}

ccimp_status_t ccimp_free(ccimp_free_t * const free_info)
{
    free(free_info->ptr);

    return CCIMP_STATUS_OK;
}

ccimp_status_t ccimp_realloc(ccimp_realloc_t * const realloc_info)
{
    ccimp_status_t status = CCIMP_STATUS_OK;

    realloc_info->ptr = realloc(realloc_info->ptr, realloc_info->new_size);
    if (realloc_info->ptr == NULL)
    {
        status = CCIMP_STATUS_ABORT;
    }

    return status;
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

ccimp_status_t ccimp_os_syncr_create(ccimp_os_syncr_create_t * const data)
{

    ccimp_status_t status = CCIMP_STATUS_OK;

    sem_t * sem = (sem_t *) malloc(sizeof(sem_t)); 

    if (sem_init(sem, 0, 0) == -1)
    {
        printf("ccimp_os_syncr_create error\n");
        return CCIMP_STATUS_ABORT;
    }

    data->syncr_object = sem;

    return status;
}

ccimp_status_t ccimp_os_syncr_acquire(ccimp_os_syncr_acquire_t * const data)
{
    struct timespec ts = { 0 };
    int s;
    sem_t * sem = data->syncr_object;

    assert(sem);

    data->acquired = CCAPI_FALSE;

    if (data->timeout_ms == OS_SYNCR_ACQUIRE_NOWAIT)
    {
        ccapi_logging_line_info("ccimp_os_syncr_acquire: about to call sem_trywait()\n");
        s = sem_trywait(sem);
    }
    else if (data->timeout_ms == OS_SYNCR_ACQUIRE_INFINITE)
    {
        ccapi_logging_line_info("ccimp_os_syncr_acquire: about to call sem_wait()\n");
        s = sem_wait(sem);
    }
    else
    {
        /* Calculate relative interval as current time plus number of milliseconds requested */
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        {
            printf("ccimp_os_syncr_acquire: clock_gettime error\n");
            return CCIMP_STATUS_ABORT;
        }

        ts.tv_sec += data->timeout_ms / 1000;
        ts.tv_nsec += (data->timeout_ms % 1000) * 1000 * 1000;

        /* Adjust if nsec rolls-over 999999999 */

        #define NSEC_ROLL_OVER (1 * 1000 * 1000 * 1000)

        if (ts.tv_nsec >= NSEC_ROLL_OVER)
        {   
            ts.tv_sec += 1;
            ts.tv_nsec %= NSEC_ROLL_OVER;
        }

        ccapi_logging_line_info("ccimp_os_syncr_acquire: about to call sem_timedwait()\n");
        s = sem_timedwait(sem, &ts);
    }

    /* Check what happened */
    if (s == -1) 
    {
        if (errno == ETIMEDOUT)
        {
            ccapi_logging_line_info("ccimp_os_syncr_acquire: timed out\n");
        }
        else if (data->timeout_ms == OS_SYNCR_ACQUIRE_NOWAIT && errno == EAGAIN)
        {
            ccapi_logging_line_info("ccimp_os_syncr_acquire: not signaled\n");
        }
        else
        {
            perror("sem_timedwait");
            return CCIMP_STATUS_ABORT;
        }
    } 
    else
    {
        ccapi_logging_line_info("ccimp_os_syncr_acquire: got it\n");
        data->acquired = CCAPI_TRUE;
    }

    return CCIMP_STATUS_OK;
}

ccimp_status_t ccimp_os_syncr_release(ccimp_os_syncr_release_t const * const data)
{
    sem_t * sem = data->syncr_object;

    assert(sem);

    if (sem_post(sem) == -1) 
    {
        printf("ccimp_os_syncr_release error\n");
        return CCIMP_STATUS_ABORT;
    }

    return CCIMP_STATUS_OK;
}

ccimp_status_t ccimp_os_syncr_destroy(ccimp_os_syncr_destroy_t const * const data)
{
    sem_t * sem = data->syncr_object;

    assert(sem);

    if (sem_destroy(sem) == -1) 
    {
        printf("ccimp_os_syncr_destroy error\n");
        return CCIMP_STATUS_ABORT;
    }

    free(sem); 

    return CCIMP_STATUS_OK;
}
