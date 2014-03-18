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

TEST(ccapi_stop_test, testCcapiStopped)
{
    ccapi_stop_error_t stop_error;
    ccapi_start_error_t start_error;
    ccapi_start_t start = {0};

    fill_start_structure_with_good_parameters(&start);

    start_error = ccapi_start(&start);

    CHECK_EQUAL(start_error, CCAPI_START_ERROR_NONE);

    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_terminate, NULL, connector_success);

    stop_error = ccapi_stop(CCAPI_STOP_GRACEFULLY);
    CHECK(stop_error == CCAPI_STOP_ERROR_NONE);
}
