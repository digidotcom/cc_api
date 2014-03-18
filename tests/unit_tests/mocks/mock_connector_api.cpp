/*
 * mock_connector_api.cpp
 *
 *  Created on: Feb 14, 2014
 *      Author: spastor
 */

#include "mock_connector_api.h"
#include "mock_ccimp_os.h"

char * assert_buffer;

static connector_handle_t global_handle;

uint8_t thread_wait;

union vp2fp 
{
    void* vp;
    connector_callback_status_t (*fp)(connector_class_id_t const class_id, connector_request_id_t const request_id, void * const data, void * const context);
};

void Mock_connector_init_create(void)
{
    global_handle = NULL;

    return;
}

void Mock_connector_init_destroy(void)
{
    mock("connector_init").checkExpectations();
    return;
}

/* context is ignored because its value is allocated by ccapi_start() and is not available from test context */
void Mock_connector_init_expectAndReturn(connector_callback_t const callback, connector_handle_t retval, void * const context)
{
    vp2fp u;
    u.fp = callback;
    UNUSED_ARGUMENT(context);

    mock("connector_init").expectOneCall("connector_init")
             .withParameter("callback", u.vp)
             .andReturnValue(retval);

    mock("connector_init").setData("behavior", MOCK_CONNECTOR_INIT_ENABLED);
}

connector_handle_t connector_init(connector_callback_t const callback, void * context)
{
    vp2fp u;
    u.fp = callback;
    uint8_t behavior;

    UNUSED_ARGUMENT(context);

    behavior = mock("connector_init").getData("behavior").getIntValue();

    if (behavior == MOCK_CONNECTOR_INIT_ENABLED)
    {
        mock("connector_init").actualCall("connector_init").withParameter("callback", u.vp);
        global_handle = mock("connector_init").returnValue().getPointerValue();
    }
    else
    {
		global_handle = &global_handle; /* Not-NULL */
    }

    return global_handle;
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
        ccimp_os_yield_real();
    }
    mock("connector_run").checkExpectations();
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

void Mock_connector_initiate_action_create(void)
{
    return;
}

void Mock_connector_initiate_action_destroy(void)
{
    mock("connector_initiate_action").checkExpectations();
}

void Mock_connector_initiate_action_expectAndReturn(connector_handle_t handle, connector_initiate_request_t request, void * request_data, connector_status_t retval)
{
    mock("connector_initiate_action").expectOneCall("connector_initiate_action")
             .withParameter("handle", handle)
             .withParameter("request", request)
             .withParameter("data", request_data)
             .andReturnValue(retval);
}

connector_status_t connector_initiate_action(connector_handle_t const handle, connector_initiate_request_t const request, void const * const request_data)
{
    mock("connector_initiate_action").actualCall("connector_initiate_action")
            .withParameter("handle", handle)
            .withParameter("request", request)
            .withParameter("request_data", (void *)request_data);

    return (connector_status_t)mock("connector_initiate_action").returnValue().getIntValue();

}
