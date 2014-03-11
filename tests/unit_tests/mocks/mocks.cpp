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
    Mock_ccimp_create_thread_create();
    Mock_connector_init_create();
    Mock_connector_run_create();
}

void Mock_destroy_all()
{
    Mock_ccimp_create_thread_destroy();
    Mock_ccimp_debug_printf_destroy();
    Mock_ccimp_malloc_destroy();
    Mock_ccimp_create_thread_destroy();
    Mock_connector_init_destroy();
    Mock_connector_run_destroy();
    ASSERT_CLEAN();
    mock().removeAllComparators();
    mock().clear();
}

