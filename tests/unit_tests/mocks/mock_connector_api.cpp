/*
 * mock_connector_api.cpp
 *
 *  Created on: Feb 14, 2014
 *      Author: spastor
 */

#include "mock_connector_api.h"

static connector_handle_t connector_init_retval = NULL;
static connector_callback_t connector_init_expected = NULL;

char * assert_buffer = NULL;

void Mock_connector_init_create(void)
{
    /* TODO, mock this */
    return;
}

void Mock_connector_init_destroy(void)
{
    mock().checkExpectations();
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

static connector_status_t connector_run_retval = connector_init_error;
static connector_handle_t connector_run_expected = NULL;

void Mock_connector_run_create(void)
{
    /* TODO, mock this */
    return;
}

void Mock_connector_run_destroy(void)
{
    mock().checkExpectations();
    return;
}

void Mock_connector_run_expectAndReturn(connector_handle_t const handle, connector_status_t retval)
{
    connector_run_expected = handle;
    connector_run_retval = retval;
}

connector_status_t connector_run(connector_handle_t const handle)
{
    CHECK(handle == connector_run_expected);

    if (connector_run_retval == connector_init_error)
        return connector_init_error;

    for(;;)
    {

    }
    return connector_run_retval;
}
