#include "mock_ccimp_debug.h"

void Mock_ccimp_debug_create(void)
{
    return;
}

void Mock_ccimp_debug_destroy(void)
{
}

void Mock_ccimp_debug_expect(char const * const message)
{
    mock("ccimp_debug").expectOneCall("ccimp_debug")
            .withParameter("message", message);

    /* If we are calling expectations, then override default implementation */
    mock("ccimp_debug").setData("behavior", MOCK_DEBUG_ENABLED);
}

extern "C" {
#if (defined CCAPI_DEBUG)
#include "CppUTestExt/MockSupport_c.h"
#include "ccapi_definitions.h"

void ccimp_debug_printf(char const * const message)
{
    uint8_t behavior;

    behavior = mock_scope_c("ccimp_debug")->getData("behavior").value.intValue;
    if (behavior == MOCK_DEBUG_ENABLED)
    {
        mock_scope_c("ccimp_debug")->actualCall("ccimp_debug")->withStringParameters("message", message);
    }
    else
    {
        /* Skip mocking, use default implementation */
        ccimp_debug_printf_real(message);
    }
    return;
}
#endif
}
