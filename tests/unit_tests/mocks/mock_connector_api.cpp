/*
 * mock_connector_api.cpp
 *
 *  Created on: Feb 14, 2014
 *      Author: spastor
 */

#include "mock_connector_api.h"

char * assert_buffer = NULL;

union vp2fp 
{
    void* vp;
    connector_callback_status_t (*fp)(connector_class_id_t const class_id, connector_request_id_t const request_id, void * const data);
};

void Mock_connector_init_create(void)
{
    return;
}

void Mock_connector_init_destroy(void)
{
    return;
}

void Mock_connector_init_expectAndReturn(connector_callback_t const callback, connector_handle_t retval)
{
    vp2fp u;
    u.fp = callback;
    mock().expectOneCall("connector_init")
             .withParameter("callback", u.vp)
             .andReturnValue(retval);
}

connector_handle_t connector_init(connector_callback_t const callback)
{
    vp2fp u;
    u.fp = callback;

    mock().actualCall("connector_init").withParameter("callback", u.vp); 
    return mock().returnValue().getPointerValue();
}

void Mock_connector_run_create(void)
{
    return;
}

void Mock_connector_run_destroy(void)
{
    return;
}

void Mock_connector_run_expectAndReturn(connector_handle_t const handle, connector_status_t retval)
{
    mock().expectOneCall("connector_run")
             .withParameter("handle", handle)
             .andReturnValue(retval);
}

connector_status_t connector_run(connector_handle_t const handle)
{
    connector_status_t ret_value;

    mock().actualCall("connector_run").withParameter("handle", handle);

    ret_value = (connector_status_t)mock().returnValue().getIntValue();

    if ((connector_status_t)ret_value == connector_init_error)
        return connector_init_error;

    for(;;)
    {

    }
    return ret_value;
}
