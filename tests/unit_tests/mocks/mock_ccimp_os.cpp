#include "mocks/mock_ccimp_os.h"

void Mock_ccimp_malloc_create(void)
{
    return;
}

void Mock_ccimp_malloc_destroy(void)
{
    mock().clear();
}

void Mock_ccimp_malloc_expectAndReturn(size_t expect, void * retval)
{
    mock().expectOneCall("ccimp_malloc")
            .withParameter("size", (int)expect)
            .andReturnValue(retval);
}

void Mock_ccimp_create_thread_create(void)
{
    return;
}

void Mock_ccimp_create_thread_destroy(void)
{
    mock().clear();
}

void Mock_ccimp_create_thread_expectAndReturn(ccimp_create_thread_info_t * const create_thread_info, ccapi_bool_t retval)
{
    UNUSED_ARGUMENT(create_thread_info);
    mock().expectOneCall("ccimp_create_thread")
            .andReturnValue(retval);
}

extern "C" {
#include "CppUTestExt/MockSupport_c.h"
#include "ccapi_definitions.h"
#include <pthread.h>

/******************** LINUX IMPLEMENTATION ********************/

static void * thread_wrapper(void * argument)
{
    ccimp_create_thread_info_t * create_thread_info = (ccimp_create_thread_info_t *)argument;

    create_thread_info->start(create_thread_info->argument);

    return NULL;
}

ccapi_bool_t ccimp_create_thread_real(ccimp_create_thread_info_t * const create_thread_info)
{
    pthread_t pthread;
    int ccode = pthread_create(&pthread, NULL, thread_wrapper, create_thread_info);

    if (ccode != 0)
    {
        printf("ccimp_create_thread() error %d\n", ccode);
        return (CCAPI_FALSE);
    }

    return CCAPI_TRUE;
}
/***********************************************************/

ccapi_bool_t ccimp_create_thread(ccimp_create_thread_info_t * create_thread_info)
{
    mock_c()->actualCall("ccimp_create_thread");
    if ((ccapi_bool_t)mock_c()->returnValue().value.intValue == 0)
    {
        /* Don't create thread, return FALSE */
        return CCAPI_FALSE;
    }
    else if ((ccapi_bool_t)mock_c()->returnValue().value.intValue == 1)
    {
        /* Create thread correctly */
        ccimp_create_thread_real(create_thread_info);
        return CCAPI_TRUE;
    }
    else if ((ccapi_bool_t)mock_c()->returnValue().value.intValue == 2)
    {
        /* Create thread but corrupting argument */
        void * wrong_argument = &wrong_argument; /* Not NULL */
        create_thread_info->argument = wrong_argument;
        ccimp_create_thread_real(create_thread_info);
        return CCAPI_TRUE;
    }

    return (ccapi_bool_t)mock_c()->returnValue().value.intValue;
}

ccimp_status_t ccimp_malloc(ccimp_malloc_t * malloc)
{
    mock_c()->actualCall("ccimp_malloc")->withIntParameters("size", malloc->size);
    malloc->ptr = mock_c()->returnValue().value.pointerValue;
    return malloc->ptr == NULL ? CCIMP_STATUS_ABORT : CCIMP_STATUS_OK;
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

ccimp_status_t ccimp_os_yield(/*connector_status_t const * const status*/ void)
{
    int error;

/*
    if (*status == connector_idle)
    {
        unsigned int const timeout_in_microseconds =  100000;
        usleep(timeout_in_microseconds);
    }
*/

    error = sched_yield();
    if (error)
    {
        /* In the Linux implementation this function always succeeds */
        printf("app_os_yield: sched_yield failed with %d\n", error);
    }

    return CCIMP_STATUS_OK;
}
}
