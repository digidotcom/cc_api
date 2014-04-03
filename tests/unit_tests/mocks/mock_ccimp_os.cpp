#include <pthread.h>

#define CCAPI_CONST_PROTECTION_UNLOCK

#include "mock_ccimp_os.h"

void Mock_ccimp_malloc_create(void)
{
    return;
}

void Mock_ccimp_malloc_destroy(void)
{
    mock("ccimp_malloc").checkExpectations();
}

void Mock_ccimp_malloc_expectAndReturn(size_t expect, void * retval)
{
    mock("ccimp_malloc").expectOneCall("ccimp_malloc")
            .withParameter("size", (int)expect)
            .andReturnValue(retval);

    /* If we are calling expectations, then override default malloc */
    mock("ccimp_malloc").setData("behavior", MOCK_MALLOC_ENABLED);
}

void Mock_ccimp_free_create(void)
{
    return;
}

void Mock_ccimp_free_destroy(void)
{
    mock("ccimp_free").checkExpectations();
}

void Mock_ccimp_free_expectAndReturn(void * ptr, ccimp_status_t retval)
{
    /* Pass a NULL pointer if you don't know beforehand the value */
    if (ptr != NULL)
    {
        mock("ccimp_free").expectOneCall("ccimp_free")
            .withParameter("ptr", (void *)ptr)
            .andReturnValue((int)retval);

        mock("ccimp_free").setData("behavior", MOCK_FREE_ENABLED_CHECK_PARAMETER);
    }
    else
    {
        mock("ccimp_free").expectOneCall("ccimp_free")
            .andReturnValue((int)retval);

        mock("ccimp_free").setData("behavior", MOCK_FREE_ENABLED_DONT_CHECK_PARAMETER);
    }
}

void Mock_ccimp_free_notExpected(void)
{
    mock("ccimp_free").setData("behavior", MOCK_FREE_ENABLED_NOT_EXPECTED);
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
    mock("ccimp_create_thread").checkExpectations();
}

void Mock_ccimp_create_thread_expectAndReturn(ccimp_create_thread_info_t * const create_thread_info, mock_thread_bahavior_t behavior, ccimp_status_t retval)
{
    mock("ccimp_create_thread").expectOneCall("ccimp_create_thread")
            .withParameterOfType("ccimp_create_thread_info_t", "parameterName", create_thread_info)
            .andReturnValue(retval);

    mock("ccimp_create_thread").setData("behavior", behavior);
}

void Mock_ccimp_os_get_system_time_create(void)
{
    return;
}

void Mock_ccimp_os_get_system_time_destroy(void)
{
    mock("ccimp_os_get_system_time").checkExpectations();
}

void Mock_ccimp_os_get_system_time_return(unsigned long retval)
{
    mock("ccimp_os_get_system_time").expectOneCall("ccimp_os_get_system_time").andReturnValue((int)retval);
}

extern "C" {
#include "CppUTestExt/MockSupport_c.h"
#include "ccapi_definitions.h"

ccimp_status_t ccimp_create_thread(ccimp_create_thread_info_t * const create_thread_info)
{
    uint8_t behavior;

    behavior = mock_scope_c("ccimp_create_thread")->getData("behavior").value.intValue;

    if (behavior == MOCK_THREAD_DISABLED)
    {
        /* Do not report actualCall */

        /* Create thread correctly */
        return ccimp_create_thread_real(create_thread_info);
    }
    else if (behavior == MOCK_THREAD_ENABLED_NORMAL)
    {
        mock_scope_c("ccimp_create_thread")->actualCall("ccimp_create_thread")->withParameterOfType("ccimp_create_thread_info_t", "parameterName", create_thread_info);

        /* Create thread correctly */
        return ccimp_create_thread_real(create_thread_info);
    }
    else if (behavior == MOCK_THREAD_ENABLED_DONT_CREATE_THREAD)
    {
        mock_scope_c("ccimp_create_thread")->actualCall("ccimp_create_thread")->withParameterOfType("ccimp_create_thread_info_t", "parameterName", create_thread_info);

        /* Don't create thread, return FALSE */
        return CCIMP_STATUS_ABORT;
    }
    else if (behavior == MOCK_THREAD_ENABLED_ARGUMENT_NULL)
    {
        mock_scope_c("ccimp_create_thread")->actualCall("ccimp_create_thread")->withParameterOfType("ccimp_create_thread_info_t", "parameterName", create_thread_info);

        /* Create thread setting argument to NULL */
        create_thread_info->argument = NULL;
        return ccimp_create_thread_real(create_thread_info);
    }

    return (ccimp_status_t)mock_scope_c("ccimp_create_thread")->returnValue().value.intValue;
}

ccimp_status_t ccimp_malloc(ccimp_malloc_t * const malloc_info)
{
    uint8_t behavior;

    behavior = mock_scope_c("ccimp_malloc")->getData("behavior").value.intValue;
    switch(behavior)
    {
        case MOCK_MALLOC_ENABLED:
        {
            mock_scope_c("ccimp_malloc")->actualCall("ccimp_malloc")->withIntParameters("size", malloc_info->size);
            malloc_info->ptr = mock_scope_c("ccimp_malloc")->returnValue().value.pointerValue;
            break;
        }
        case MOCK_MALLOC_DISABLED:
        default:
        {
            /* Skip mocking, use default malloc implementation */
            ccimp_malloc_real(malloc_info);
            memset(malloc_info->ptr, 0xFF, malloc_info->size); /* Try to catch hidden problems */
            break;
        }
    }

    return malloc_info->ptr == NULL ? CCIMP_STATUS_ABORT : CCIMP_STATUS_OK;
}

ccimp_status_t ccimp_free(ccimp_free_t * const free_info)
{
    uint8_t behavior;
    ccimp_status_t retval = CCIMP_STATUS_OK;

    behavior = mock_scope_c("ccimp_free")->getData("behavior").value.intValue;
    switch(behavior)
    {
        case MOCK_FREE_ENABLED_CHECK_PARAMETER:
        {
            mock_scope_c("ccimp_free")->actualCall("ccimp_free")->withPointerParameters("ptr", free_info->ptr);
            retval = (ccimp_status_t)mock_scope_c("ccimp_free")->returnValue().value.intValue;
            break;
        }
        case MOCK_FREE_ENABLED_DONT_CHECK_PARAMETER:
        {
            mock_scope_c("ccimp_free")->actualCall("ccimp_free");
            retval =  (ccimp_status_t)mock_scope_c("ccimp_free")->returnValue().value.intValue;
            break;
        }
        case MOCK_FREE_ENABLED_NOT_EXPECTED:
        {
            mock_scope_c("ccimp_free")->actualCall("ccimp_free");
            break;
        }
        default:
        {
            ccimp_free_real(free_info);
            break;
        }
    }

    return retval;
}

ccimp_status_t ccimp_realloc(ccimp_realloc_t * const realloc_info)
{
    return ccimp_realloc_real(realloc_info);
}

ccimp_status_t ccimp_os_get_system_time(ccimp_os_system_up_time_t * const system_up_time)
{
    mock_scope_c("ccimp_os_get_system_time")->actualCall("ccimp_os_get_system_time");
    system_up_time->sys_uptime = mock_scope_c("ccimp_os_get_system_time")->returnValue().value.intValue;
    return CCIMP_STATUS_OK;
}

ccimp_status_t ccimp_os_yield(void)
{
    pthread_testcancel();
    return ccimp_os_yield_real();
}
}
