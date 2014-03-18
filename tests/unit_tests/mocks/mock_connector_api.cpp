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


static connector_status_t connector_run_retval = connector_idle;

void Mock_connector_run_create(void)
{
    connector_run_retval = connector_idle;
    return;
}

void Mock_connector_run_destroy(void)
{
    return;
}

void Mock_connector_run_returnInNextLoop(connector_status_t retval)
{
    connector_run_retval = retval;
}

connector_status_t connector_run(connector_handle_t const handle)
{
    UNUSED_ARGUMENT(handle);

    do {
        if (connector_run_retval == connector_idle || connector_run_retval == connector_working || connector_run_retval == connector_pending || connector_run_retval == connector_active || connector_run_retval == connector_success)
        {
            ccimp_os_yield();
        }
    } while (connector_run_retval == connector_idle || connector_run_retval == connector_working || connector_run_retval == connector_pending || connector_run_retval == connector_active || connector_run_retval == connector_success);

    return connector_run_retval;
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
             .withParameter("request_data", request_data)
             .andReturnValue(retval);
}

connector_status_t connector_initiate_action(connector_handle_t const handle, connector_initiate_request_t const request, void const * const request_data)
{
    mock("connector_initiate_action").actualCall("connector_initiate_action")
            .withParameter("handle", handle)
            .withParameter("request", request)
            .withParameter("request_data", (void *)request_data);

    if (request == connector_initiate_terminate)
    {
        ccapi_data_single_instance->thread.connector_run->status = CCAPI_THREAD_REQUEST_STOP;
        Mock_connector_run_returnInNextLoop(connector_device_terminated);
    }

    return (connector_status_t)mock("connector_initiate_action").returnValue().getIntValue();

}
