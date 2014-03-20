/*
 * mocks.cpp
 *
 *  Created on: Mar 11, 2014
 *      Author: spastor
 */


#include "mocks.h"

void Mock_create_all()
{
    Mock_ccimp_debug_printf_create();
    Mock_ccimp_malloc_create();
    Mock_ccimp_free_create();
    Mock_ccimp_create_thread_create();
    Mock_connector_init_create();
    Mock_connector_run_create();
    Mock_connector_init_create();
}

void Mock_destroy_all()
{
    if (ccapi_data_single_instance != NULL)
    {
        /* Dear developer: If you want to call free on this pointer,
         * know that it leads to race conditions making TEST(ccapi_init_test, testDeviceTypeNoMemory) fail
         * once in ~100 iterations. Don't say I didn't tell you! Good luck! =)
         */
        ccapi_data_single_instance = NULL;
    }
    Mock_ccimp_create_thread_destroy();
    Mock_ccimp_debug_printf_destroy();
    Mock_ccimp_malloc_destroy();
    Mock_ccimp_free_destroy();
    Mock_ccimp_create_thread_destroy();
    Mock_connector_init_destroy();
    Mock_connector_run_destroy();
    Mock_connector_init_destroy();
    ASSERT_CLEAN();
    mock().removeAllComparators();
    mock().checkExpectations();
    mock().clear();
}

