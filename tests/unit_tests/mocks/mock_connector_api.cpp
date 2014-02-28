/*
 * mock_connector_api.cpp
 *
 *  Created on: Feb 14, 2014
 *      Author: spastor
 */

#include "mock_connector_api.h"
#include <sched.h>

char * assert_buffer = NULL;

uint8_t thread_wait;

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
    mock("connector_init").expectOneCall("connector_init")
             .withParameter("callback", u.vp)
             .andReturnValue(retval);
}

connector_handle_t connector_init(connector_callback_t const callback)
{
    vp2fp u;
    u.fp = callback;

    mock("connector_init").actualCall("connector_init").withParameter("callback", u.vp); 
    return mock("connector_init").returnValue().getPointerValue();
}

void Mock_connector_run_create(void)
{
    thread_wait = 0;
    return;
}

void Mock_connector_run_destroy(void)
{
    /* Make sure that if the thread has been scheduled, the mock signalization is done */
    while(thread_wait)
    {
        sched_yield();
    }
    return;
}

void Mock_connector_run_expectAndReturn(connector_handle_t const handle, connector_status_t retval)
{
    mock("connector_run").expectOneCall("connector_run")
             .withParameter("handle", handle)
             .andReturnValue(retval);

    mock("connector_run").setData("behavior", MOCK_CONNECTOR_RUN_ENABLED);
}

connector_status_t connector_run(connector_handle_t const handle)
{
    uint8_t behavior;
    connector_status_t ret_value;
    
    thread_wait = 1;

    behavior = mock("connector_run").getData("behavior").getIntValue();


    if (behavior == MOCK_CONNECTOR_RUN_ENABLED)
    {
        mock("connector_run").actualCall("connector_run").withParameter("handle", handle);

        ret_value = (connector_status_t)mock("connector_run").returnValue().getIntValue();
        if ((connector_status_t)ret_value == connector_init_error)
        {
            thread_wait = 0;
            return connector_init_error;
        }
    }
    else
    {
        ret_value = connector_success;
    }

    thread_wait = 0;

    for(;;)
    {

    }
    return ret_value;
}
