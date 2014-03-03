#include "mocks/mock_ccimp_os.h"

void Mock_ccimp_malloc_create(void)
{
    return;
}

void Mock_ccimp_malloc_destroy(void)
{
}

void Mock_ccimp_malloc_expectAndReturn(size_t expect, void * retval)
{
    mock("ccimp_malloc").expectOneCall("ccimp_malloc")
            .withParameter("size", (int)expect)
            .andReturnValue(retval);

    /* If we are calling expectations, then override default malloc */
    mock("ccimp_malloc").setData("behavior", MOCK_MALLOC_ENABLED);
}

bool ccimp_create_thread_info_t_IsEqual(void* object1, void* object2)
{
    ccimp_create_thread_info_t * o1 = (ccimp_create_thread_info_t*)object1;
    ccimp_create_thread_info_t * o2 = (ccimp_create_thread_info_t*)object2;
    bool ret = 0;

    if (o1 != NULL && o2 != NULL) 
    {  
        ret = ((o1->argument == o2->argument) && (o1->type == o2->type));
        /* Not checking 'start' parameter */
    }

    return ret;
}

SimpleString ccimp_create_thread_info_t_ValueToString(void* object)
{
	return StringFrom(((ccimp_create_thread_info_t*)object)->type);
}

void Mock_ccimp_create_thread_create(void)
{

    static MockFunctionComparator comparator(ccimp_create_thread_info_t_IsEqual, ccimp_create_thread_info_t_ValueToString);
    mock().installComparator("ccimp_create_thread_info_t", comparator);

    return;
}

void Mock_ccimp_create_thread_destroy(void)
{
}

void Mock_ccimp_create_thread_expectAndReturn(ccimp_create_thread_info_t * const create_thread_info, mock_thread_bahavior_t behavior, ccapi_bool_t retval)
{
    mock("ccimp_create_thread").expectOneCall("ccimp_create_thread")
            .withParameterOfType("ccimp_create_thread_info_t", "parameterName", create_thread_info)
            .andReturnValue(retval);

    mock("ccimp_create_thread").setData("behavior", behavior);
}

extern "C" {
#include "CppUTestExt/MockSupport_c.h"
#include "ccapi_definitions.h"
#include <malloc.h>
#include <pthread.h>
#include <unistd.h>

/******************** LINUX IMPLEMENTATION ********************/

static void * thread_wrapper(void * argument)
{
    ccimp_create_thread_info_t * create_thread_info = (ccimp_create_thread_info_t *)argument;

    /* TODO: Introduce a random delay on thread start? */
    /* usleep(300000); */

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
    uint8_t behavior;

    behavior = mock_scope_c("ccimp_create_thread")->getData("behavior").value.intValue;

    if (behavior == MOCK_THREAD_DISABLED)
    {
        /* Do not report actualCall */

        /* Create thread correctly */
        ccimp_create_thread_real(create_thread_info);
        return CCAPI_TRUE;
    }
    else if (behavior == MOCK_THREAD_ENABLED0)
    {
        mock_scope_c("ccimp_create_thread")->actualCall("ccimp_create_thread")->withParameterOfType("ccimp_create_thread_info_t", "parameterName", create_thread_info);

        /* Create thread correctly */
        ccimp_create_thread_real(create_thread_info);
        return CCAPI_TRUE;
    }
    else if (behavior == MOCK_THREAD_ENABLED1)
    {
        mock_scope_c("ccimp_create_thread")->actualCall("ccimp_create_thread")->withParameterOfType("ccimp_create_thread_info_t", "parameterName", create_thread_info);

        /* Don't create thread, return FALSE */
        return CCAPI_FALSE;
    }
    else if (behavior == MOCK_THREAD_ENABLED2)
    {
        mock_scope_c("ccimp_create_thread")->actualCall("ccimp_create_thread")->withParameterOfType("ccimp_create_thread_info_t", "parameterName", create_thread_info);

        /* Create thread but corrupting argument */
        void * wrong_argument = &wrong_argument; /* Not NULL */
        create_thread_info->argument = wrong_argument;
        ccimp_create_thread_real(create_thread_info);
        return CCAPI_TRUE;
    }

    return (ccapi_bool_t)mock_scope_c("ccimp_create_thread")->returnValue().value.intValue;
}

ccimp_status_t ccimp_malloc(ccimp_malloc_t * malloc)
{
    uint8_t behavior;

    behavior = mock_scope_c("ccimp_malloc")->getData("behavior").value.intValue;
    if (behavior == MOCK_MALLOC_ENABLED)
    {
        mock_scope_c("ccimp_malloc")->actualCall("ccimp_malloc")->withIntParameters("size", malloc->size);
        malloc->ptr = mock_scope_c("ccimp_malloc")->returnValue().value.pointerValue;
    }
    else
    {
        /* Skip mocking, use default malloc implementation */
        malloc->ptr = calloc(1, malloc->size);
        memset(malloc->ptr, 0xFF, malloc->size); /* Try to catch hidden problems */
    }
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
