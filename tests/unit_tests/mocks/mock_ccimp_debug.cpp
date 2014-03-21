#include "mock_ccimp_debug.h"

#define MOCK_DEBUG_ENABLED 1    /* Mock disabled. Do printf normally */

void Mock_ccimp_debug_printf_create(void)
{
    return;
}

void Mock_ccimp_debug_printf_destroy(void)
{
    mock("ccimp_debug_vprintf").checkExpectations();
}

void Mock_ccimp_debug_vprintf_expect(debug_t const debug, char const * const buffer)
{
    /* If we are calling expectations, then override default implementation */
    mock("ccimp_debug_vprintf").setData("behavior", MOCK_DEBUG_ENABLED);

    if (strcmp(buffer, CCIMP_DEBUG_PRINTF_DOESNT_EXPECT_A_CALL) != 0)
    {
        mock("ccimp_debug_vprintf").expectOneCall("ccimp_debug_vprintf")
                    .withParameter("debug", debug).withParameter("buffer", buffer);
    }
}

extern "C" {
#if (defined CCIMP_DEBUG_ENABLED)
#include "CppUTestExt/MockSupport_c.h"
#include "ccapi_definitions.h"

void ccimp_debug_vprintf(debug_t const debug, char const * const format, va_list args)
{
    uint8_t behavior;

    behavior = mock_scope_c("ccimp_debug_vprintf")->getData("behavior").value.intValue;
    if (behavior == MOCK_DEBUG_ENABLED)
    {
        char buffer[500];
        vsnprintf(buffer, sizeof(buffer), format, args);   

        mock_scope_c("ccimp_debug_vprintf")->actualCall("ccimp_debug_vprintf")
              ->withIntParameters("debug", debug)->withStringParameters("buffer", buffer);
    }
    else
    {
        /* Skip mocking, but we won't call real implementation... not to bother. May change when we have zones */
        /* ccimp_debug_vprintf_real(debug, format, args); */
    }
    return;
}
#endif
}
