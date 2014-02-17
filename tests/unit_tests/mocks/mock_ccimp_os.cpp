#include "CppUTestExt/MockSupport.h"
#include "ccimp/ccimp_types.h"

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
    mock().expectOneCall("ccimp_create_thread")
            .withParameter("create_thread_info", (void *)create_thread_info)
            .andReturnValue(retval);
}

extern "C" {
#include "CppUTestExt/MockSupport_c.h"
#include "internal/ccapi_definitions.h"

void * ccimp_malloc(size_t size)
{
    mock_c()->actualCall("ccimp_malloc")->withIntParameters("size", size);
    return mock_c()->returnValue().value.pointerValue;
}

ccapi_bool_t ccimp_create_thread(ccimp_create_thread_info_t * create_thread_info)
{
    mock_c()->actualCall("ccimp_create_thread")->withPointerParameters("create_thread_info", create_thread_info);
    return (ccapi_bool_t)mock_c()->returnValue().value.intValue;
}

}

