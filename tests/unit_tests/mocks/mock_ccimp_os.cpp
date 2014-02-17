#include "CppUTestExt/MockSupport.h"

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

extern "C" {
#include "CppUTestExt/MockSupport_c.h"
#include "internal/ccapi_definitions.h"

void * ccimp_malloc(size_t size)
{
    mock_c()->actualCall("ccimp_malloc")->withIntParameters("size", size);
    return mock_c()->returnValue().value.pointerValue;
}

}
