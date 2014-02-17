/*
 * mock_connector_api.cpp
 *
 *  Created on: Feb 14, 2014
 *      Author: spastor
 */

#include "CppUTestExt/MockSupport.h"
#include "CppUTest/CommandLineTestRunner.h"

#include "mock_connector_api.h"

static connector_handle_t connector_init_retval = NULL;
static connector_callback_t connector_init_expected = NULL;

void Mock_connector_init_create(void)
{
    /* TODO, mock this */
    return;
}

void Mock_connector_init_destroy(void)
{
    return;
}

void Mock_connector_init_expectAndReturn(connector_callback_t const callback, connector_handle_t retval)
{
    connector_init_expected = callback;
    connector_init_retval = retval;
}

connector_handle_t connector_init(connector_callback_t const callback)
{
    CHECK(callback == connector_init_expected);
    return connector_init_retval;
}

extern "C" {
#include "CppUTestExt/MockSupport_c.h"
#include "internal/ccapi_definitions.h"
#include "connector_api.h"

}

#if 0
connector_handle_t connector_init(connector_callback_t const callback)
{
    mock_c()->actualCall("connector_init")->withIntParameters("callback", callback);
    return mock_c()->returnValue().value.pointerValue;
}
#endif
