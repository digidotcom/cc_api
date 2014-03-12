#include "mock_ccimp_debug.h"

#define MOCK_DEBUG_ENABLED 1    /* Mock disabled. Do printf normally */

void Mock_ccimp_debug_printf_create(void)
{
    return;
}

void Mock_ccimp_debug_printf_destroy(void)
{
    mock("ccimp_debug_printf").checkExpectations();
}

void Mock_ccimp_debug_printf_expect(char const * const message)
{
    /* If we are calling expectations, then override default implementation */
    mock("ccimp_debug_printf").setData("behavior", MOCK_DEBUG_ENABLED);

    if (strcmp(message, CCIMP_DEBUG_PRINTF_DOESNT_EXPECT_A_CALL) != 0)
    {
        mock("ccimp_debug_printf").expectOneCall("ccimp_debug_printf")
                    .withParameter("message", message);
    }
}

extern "C" {
#if (defined CCIMP_DEBUG_ENABLED)
#include "CppUTestExt/MockSupport_c.h"
#include "ccapi_definitions.h"

void ccimp_debug_printf(char const * const message)
{
    uint8_t behavior;

    behavior = mock_scope_c("ccimp_debug_printf")->getData("behavior").value.intValue;
    if (behavior == MOCK_DEBUG_ENABLED)
    {
        mock_scope_c("ccimp_debug_printf")->actualCall("ccimp_debug_printf")->withStringParameters("message", message);
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
