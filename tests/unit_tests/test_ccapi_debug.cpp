#include "CppUTest/CommandLineTestRunner.h"
#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"

#include "test_helper_functions.h"

using namespace std;

TEST_GROUP(ccapi_debug_test)
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

#if (defined CCIMP_DEBUG_ENABLED)

TEST(ccapi_debug_test, testDbg_args0)
{
	debug_t debug;

    debug = debug_all;

	Mock_ccimp_debug_vprintf_expect(debug, "testDbg_args0");

    ccapi_debug_line("testDbg_args0");
}

TEST(ccapi_debug_test, testDbg_args1)
{
	debug_t debug;

    debug = debug_all;

	Mock_ccimp_debug_vprintf_expect(debug, "testDbg_args1 33");

    ccapi_debug_line("testDbg_args1 %d", 33);
}

TEST(ccapi_debug_test, testDbg_args2)
{
	debug_t debug;

    debug = debug_all;

	Mock_ccimp_debug_vprintf_expect(debug, "testDbg_args2 34 0.25");

    ccapi_debug_line("testDbg_args2 %d %.2f", 34, 0.25);
}


#else
/* CCAPI_DEBUG is not defined. Check that nothing happens */
TEST(ccapi_debug_test, testDbgDefineDisabled)
{
    ccapi_start_error_t error;
    ccapi_start_t start = {0};
    fill_start_structure_with_good_parameters(&start);

    /* Manually enable the mock to check that nothing is called */
    mock("ccimp_debug_vprintf").setData("behavior", CCIMP_DEBUG_PRINTF_DOESNT_EXPECT_A_CALL);

    error = ccapi_start(&start);
    CHECK(error == CCAPI_START_ERROR_NONE);
}
#endif