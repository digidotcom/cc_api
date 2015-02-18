/*
* Copyright (c) 2014 Etherios, a Division of Digi International, Inc.
* All rights not expressly granted are reserved.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Etherios 11001 Bren Road East, Minnetonka, MN 55343
* =======================================================================
*/

#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_sm_support)
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

TEST(test_ccapi_sm_support, testSmNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;

    th_fill_start_structure_with_good_parameters(&start);

    error = ccapi_start(&start);

    CHECK_EQUAL(CCAPI_START_ERROR_NONE, error);
    CHECK_EQUAL(CCAPI_FALSE, ccapi_data_single_instance->config.sm_supported);
}

TEST(test_ccapi_sm_support, testSmSupported)
{
    ccapi_start_t start = {0};
    ccapi_start_error_t error;
    ccapi_sm_service_t sm_service = {NULL, NULL, NULL, NULL, NULL};

    th_fill_start_structure_with_good_parameters(&start);
    start.service.sm = &sm_service;

    error = ccapi_start(&start);

    CHECK_EQUAL(CCAPI_START_ERROR_NONE, error);
    CHECK_EQUAL(CCAPI_TRUE, ccapi_data_single_instance->config.sm_supported);
}
