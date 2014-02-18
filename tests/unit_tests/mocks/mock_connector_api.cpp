/*
 * mock_connector_api.cpp
 *
 *  Created on: Feb 14, 2014
 *      Author: spastor
 */

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

connector_status_t connector_run(connector_handle_t const handle)
{
    assert(handle != NULL);
    for(;;)
    {

    }
    return connector_open_error;
}
