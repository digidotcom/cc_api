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


#endif