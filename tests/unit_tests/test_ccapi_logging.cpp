#include "CppUTest/CommandLineTestRunner.h"
#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
}

#include "test_helper_functions.h"

using namespace std;

TEST_GROUP(ccapi_logging_test)
{
    void setup()
    {
        Mock_create_all();
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

#if (defined CCIMP_LOGGING_ENABLED)

TEST(ccapi_logging_test, layer2_args0)
{
	debug_t debug;

    debug = debug_all;

	Mock_ccimp_hal_logging_vprintf_expect(debug, "layer2_args0");

    ccapi_logging_line("layer2_args0");
}

TEST(ccapi_logging_test, layer2_args1)
{
	debug_t debug;

    debug = debug_all;

	Mock_ccimp_hal_logging_vprintf_expect(debug, "layer2_args1 33");

    ccapi_logging_line("layer2_args1 %d", 33);
}

TEST(ccapi_logging_test, layer2_args2)
{
	debug_t debug;

    debug = debug_all;

	Mock_ccimp_hal_logging_vprintf_expect(debug, "layer2_args2 34 0.25");

    ccapi_logging_line("layer2_args2 %d %.2f", 34, 0.25);
}

#define CALL_DEBUG_VPRINTF(type, format) \
    do \
    { \
        va_list args; \
 \
        va_start(args, (format)); \
        connector_debug_vprintf((type), (format), args); \
        va_end(args); \
    } \
    while (0)

static void connector_debug_line(char const * const format, ...)
{
    CALL_DEBUG_VPRINTF(debug_all, format);
}

TEST(ccapi_logging_test, layer1_args0)
{
	debug_t debug;

    debug = debug_all;

	Mock_ccimp_hal_logging_vprintf_expect(debug, "layer1_args0");

    connector_debug_line("layer1_args0");
}

TEST(ccapi_logging_test, layer1_args1)
{
	debug_t debug;

    debug = debug_all;

	Mock_ccimp_hal_logging_vprintf_expect(debug, "layer1_args1 33");

    connector_debug_line("layer1_args1 %d", 33);
}

TEST(ccapi_logging_test, layer1_args2)
{
	debug_t debug;

    debug = debug_all;

	Mock_ccimp_hal_logging_vprintf_expect(debug, "layer1_args2 34 0.25");

    connector_debug_line("layer1_args2 %d %.2f", 34, 0.25);
}

#else
/* CCAPI_LOGGING is not defined. Check that nothing happens */
TEST(ccapi_logging_test, testDbgDefineDisabled)
{
    ccapi_start_error_t error;
    ccapi_start_t start = {0};
    fill_start_structure_with_good_parameters(&start);

    /* Manually enable the mock to check that nothing is called */
    mock("ccimp_hal_logging_vprintf").setData("behavior", CCIMP_LOGGING_PRINTF_DOESNT_EXPECT_A_CALL);

    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_NONE);
}
#endif