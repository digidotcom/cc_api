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
#include <stdarg.h>

#if (defined UNIT_TEST)
#define ccimp_os_malloc             ccimp_os_malloc_real
#define ccimp_os_free               ccimp_os_free_real
#define ccimp_os_realloc            ccimp_os_realloc_real
#define ccimp_os_create_thread      ccimp_os_create_thread_real
#define ccimp_os_get_system_time    ccimp_os_get_system_time_real
#define ccimp_os_yield              ccimp_os_yield_real
#define ccimp_os_syncr_create       ccimp_os_syncr_create_real
#define ccimp_os_syncr_acquire      ccimp_os_syncr_acquire_real
#define ccimp_os_syncr_release      ccimp_os_syncr_release_real
#endif

#define ccapi_logging_line_info(message) /* TODO */

/******************** LINUX IMPLEMENTATION ********************/

int connector_snprintf(char * const str, size_t const size, char const * const format, ...)
{
    va_list args;
    int result;

    va_start(args, format);

#if __STDC_VERSION__ >= 199901L
    result = vsnprintf(str, size, format, args);
#else
    /**************************************************************************************
    * NOTE: This is an example snprintf() implementation for environments                 *
    * that do not provide one.                                                            *
    * The following buffer should hold the longest possible string that can be generated. *
    * Consider the common case to be only one format specifier at once.                   *
    * WARNING: If an application is using double Data Point types, this value can be as   *
    * big as:                                                                             *
    *                 max_digits = 3 + DBL_MANT_DIG - DBL_MIN_EXP                         *
    * The above expression evaluates to 1077 in a 64-bit Linux machine for GCC 4.8.1.     *
    **************************************************************************************/
    #define SAFE_BUFFER_BYTES 64

    if (size >= SAFE_BUFFER_BYTES)
    {
        result = vsprintf(str, format, args);
    }
    else
    {
        char local_buffer[SAFE_BUFFER_BYTES];
        ssize_t const bytes_needed = vsprintf(local_buffer, format, args);

        if (bytes_needed < (ssize_t)size)
        {
            memcpy(str, local_buffer, bytes_needed + 1); /* Don't forget the \0 */
        }
        result = bytes_needed;
    }
    #undef SAFE_BUFFER_BYTES
#endif
    va_end(args);

    return result;
}

ccimp_status_t ccimp_os_malloc(ccimp_os_malloc_t * const malloc_info)
{
    malloc_info->ptr = malloc(malloc_info->size);

    return malloc_info->ptr == NULL ? CCIMP_STATUS_ERROR : CCIMP_STATUS_OK;
}

ccimp_status_t ccimp_os_free(ccimp_os_free_t * const free_info)
{
    free((void *)free_info->ptr);

    return CCIMP_STATUS_OK;
}

ccimp_status_t ccimp_os_realloc(ccimp_os_realloc_t * const realloc_info)
{
    ccimp_status_t status = CCIMP_STATUS_OK;

    realloc_info->ptr = realloc(realloc_info->ptr, realloc_info->new_size);
    if (realloc_info->ptr == NULL)
    {
        status = CCIMP_STATUS_ERROR;
    }

    return status;
}

static void * thread_wrapper(void * argument)
{
    ccimp_create_thread_info_t * create_thread_info = (ccimp_create_thread_info_t *)argument;

    create_thread_info->start(create_thread_info->argument);

    return NULL;
}

ccimp_status_t ccimp_os_create_thread(ccimp_create_thread_info_t * const create_thread_info)
{
    pthread_t pthread;
    int ccode = pthread_create(&pthread, NULL, thread_wrapper, create_thread_info);

    if (ccode != 0)
    {
        printf("ccimp_create_thread() error %d\n", ccode);
        return (CCIMP_STATUS_ERROR);
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
    sem_t * const sem = (sem_t *) malloc(sizeof *sem);

    if (sem == NULL)
    {
        printf("ccimp_os_syncr_create insufficent memory\n");
        status = CCIMP_STATUS_ERROR;
        goto done;
    }

    if (sem_init(sem, 0, 0) == -1)
    {
        printf("ccimp_os_syncr_create error\n");
        free(sem);
        status = CCIMP_STATUS_ERROR;
        goto done;
    }

    data->syncr_object = sem;
done:
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
            return CCIMP_STATUS_ERROR;
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
            return CCIMP_STATUS_ERROR;
        }
    } 
    else
    {
        ccapi_logging_line_info("ccimp_os_syncr_acquire: got it\n");
        data->acquired = CCAPI_TRUE;
    }

    return CCIMP_STATUS_OK;
}

ccimp_status_t ccimp_os_syncr_release(ccimp_os_syncr_release_t * const data)
{
    sem_t * const sem = data->syncr_object;

    assert(sem);

    if (sem_post(sem) == -1) 
    {
        printf("ccimp_os_syncr_release error\n");
        return CCIMP_STATUS_ERROR;
    }

    return CCIMP_STATUS_OK;
}

ccimp_status_t ccimp_os_syncr_destroy(ccimp_os_syncr_destroy_t * const data)
{
    sem_t * const sem = data->syncr_object;

    assert(sem);

    if (sem_destroy(sem) == -1) 
    {
        printf("ccimp_os_syncr_destroy error\n");
        return CCIMP_STATUS_ERROR;
    }

    free(sem); 

    return CCIMP_STATUS_OK;
}
