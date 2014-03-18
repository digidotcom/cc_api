/*
 * test_ccapi_stop.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: spastor
 */

#include "CppUTest/CommandLineTestRunner.h"
#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
#include "ccimp/ccimp_os.h"
}

#include "test_helper_functions.h"

TEST_GROUP(ccapi_stop_test)
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

TEST(ccapi_stop_test, testCcapiNotStarted)
{
    ccapi_stop_error_t error;

    error = ccapi_stop(CCAPI_STOP_GRACEFULLY);
    CHECK(error == CCAPI_STOP_ERROR_NOT_STARTED);

    error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
    CHECK(error == CCAPI_STOP_ERROR_NOT_STARTED);
}

