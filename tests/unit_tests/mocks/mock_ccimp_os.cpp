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

extern "C" {
#include "CppUTestExt/MockSupport_c.h"
#include "ccapi_definitions.h"

ccimp_status_t ccimp_create_thread(ccimp_create_thread_info_t * create_thread_info)
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
    else if (behavior == MOCK_THREAD_ENABLED2_ARGUMENT_CORRUPT)
    {
        mock_scope_c("ccimp_create_thread")->actualCall("ccimp_create_thread")->withParameterOfType("ccimp_create_thread_info_t", "parameterName", create_thread_info);

        /* Create thread but corrupting argument */
        void * wrong_argument = &wrong_argument; /* Not NULL */
        create_thread_info->argument = wrong_argument;
        return ccimp_create_thread_real(create_thread_info);
    }
    else if (behavior == MOCK_THREAD_ENABLED3_ARGUMENT_NULL)
    {
        mock_scope_c("ccimp_create_thread")->actualCall("ccimp_create_thread")->withParameterOfType("ccimp_create_thread_info_t", "parameterName", create_thread_info);

        /* Create thread setting argument to NULL */
        create_thread_info->argument = NULL;
        return ccimp_create_thread_real(create_thread_info);
    }

    return (ccimp_status_t)mock_scope_c("ccimp_create_thread")->returnValue().value.intValue;
}

ccimp_status_t ccimp_malloc(ccimp_malloc_t * malloc_info)
{
    uint8_t behavior;

    behavior = mock_scope_c("ccimp_malloc")->getData("behavior").value.intValue;
    if (behavior == MOCK_MALLOC_ENABLED)
    {
        mock_scope_c("ccimp_malloc")->actualCall("ccimp_malloc")->withIntParameters("size", malloc_info->size);
        malloc_info->ptr = mock_scope_c("ccimp_malloc")->returnValue().value.pointerValue;
    }
    else
    {
        /* Skip mocking, use default malloc implementation */
        ccimp_malloc_real(malloc_info);
        memset(malloc_info->ptr, 0xFF, malloc_info->size); /* Try to catch hidden problems */
    }
    return malloc_info->ptr == NULL ? CCIMP_STATUS_ABORT : CCIMP_STATUS_OK;
}

ccimp_status_t ccimp_os_get_system_time(ccimp_os_system_up_time_t * const system_up_time)
{
    return ccimp_os_get_system_time_real(system_up_time);
}

ccimp_status_t ccimp_os_yield(void)
{
    return ccimp_os_yield_real();
}
}
