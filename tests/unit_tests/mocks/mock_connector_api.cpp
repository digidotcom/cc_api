/*
 * mock_connector_api.cpp
 *
 *  Created on: Feb 14, 2014
 *      Author: spastor
 */

#include "mock_connector_api.h"
#include "mock_ccimp_os.h"
#include <pthread.h>

mock_connector_api_info_t mock_info[MAX_INFO];

uint8_t thread_wait;

mock_connector_api_info_t * alloc_mock_connector_api_info(void)
{
    size_t i;

    for (i = 0; i < MAX_INFO; i++)
    {
        if (mock_info[i].used == connector_false)
        {
            mock_info[i].used = connector_true;
            mock_info[i].connector_run_retval = connector_idle;

            return &mock_info[i];
        }
    }

    return NULL;
}

mock_connector_api_info_t * get_mock_connector_api_info(connector_handle_t connector_handle)
{
    size_t i;

    for (i = 0; i < MAX_INFO; i++)
    {
        if (mock_info[i].used == connector_true && mock_info[i].connector_handle == connector_handle)
            return &mock_info[i];
    }

    return NULL;
}

void free_mock_connector_api_info(mock_connector_api_info_t * mock_info)
{
    mock_info->ccapi_handle = NULL;
    mock_info->used = connector_false;
}

union vp2fp 
{
    void* vp;
    connector_callback_status_t (*fp)(connector_class_id_t const class_id, connector_request_id_t const request_id, void * const data, void * const context);
};

void Mock_connector_init_create(void)
{
    memset(mock_info, 0, sizeof(mock_info));

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
        return mock("connector_init").returnValue().getPointerValue();
    }
    else
    {
        return (connector_handle_t)rand();
    } 
}

static connector_bool_t kill_ccapi_thread = connector_false;

void Mock_connector_run_create(void)
{
    kill_ccapi_thread = connector_false;

    return;
}

void Mock_connector_run_destroy(void)
{
    kill_ccapi_thread = connector_true;

    return;
}

connector_status_t connector_run(connector_handle_t const handle)
{
    connector_status_t connector_run_retval;

    /* printf("+connector_run: handle=%p\n",(void*)handle);	*/
    do {
        mock_connector_api_info_t * mock_info = get_mock_connector_api_info(handle);
        connector_run_retval = mock_info != NULL? mock_info->connector_run_retval:connector_idle;

        if (connector_run_retval == connector_idle || connector_run_retval == connector_working || connector_run_retval == connector_pending || connector_run_retval == connector_active || connector_run_retval == connector_success)
        {
            ccimp_os_yield();
        }
        if (kill_ccapi_thread == connector_true)
        {
            pthread_exit(NULL);
        }
    } while (connector_run_retval == connector_idle || connector_run_retval == connector_working || connector_run_retval == connector_pending || connector_run_retval == connector_active || connector_run_retval == connector_success);
    /* printf("-connector_run: handle=%p\n",(void*)handle); */

    return connector_run_retval;
}

/* * * * * * * * * * connector_initiate_action() * * * * * * * * * */
SimpleString connector_transport_t_ValueToString(void* object)
{
    UNUSED_ARGUMENT(object);
    return "connector_transport_t";
}

bool connector_transport_t_IsEqual(void * object1, void * object2)
{
    connector_transport_t * connector_transport_1 = (connector_transport_t *)object1;
    connector_transport_t * connector_transport_2 = (connector_transport_t *)object2;

    if (connector_transport_1 == connector_transport_2)
        return true;

    if (connector_transport_1 == NULL || connector_transport_2 == NULL)
        return false;

    if (*connector_transport_1 != *connector_transport_2)
        return false;

    return true;
}

void Mock_connector_initiate_action_create(void)
{
    static MockFunctionComparator connector_transport_t_comparator(connector_transport_t_IsEqual, connector_transport_t_ValueToString);
    mock().installComparator("connector_transport_t", connector_transport_t_comparator);

    return;
}

void Mock_connector_initiate_action_destroy(void)
{
    mock("connector_initiate_action").checkExpectations();
}

void Mock_connector_initiate_action_expectAndReturn(connector_handle_t handle, connector_initiate_request_t request, void * request_data, connector_status_t retval)
{
    switch (request)
    {
        case connector_initiate_transport_start:
            mock("connector_initiate_action").expectOneCall("connector_initiate_action")
                     .withParameter("handle", handle)
                     .withParameter("request", request)
                     .withParameterOfType("connector_transport_t", "request_data", request_data)
                     .andReturnValue(retval);
            break;
        case connector_initiate_transport_stop:
        case connector_initiate_send_data:
#ifdef CONNECTOR_SHORT_MESSAGE
        case connector_initiate_ping_request:
        case connector_initiate_session_cancel:
        case connector_initiate_session_cancel_all:
#endif
        case connector_initiate_terminate:
            mock("connector_initiate_action").expectOneCall("connector_initiate_action")
                     .withParameter("handle", handle)
                     .withParameter("request", request)
                     .withParameter("request_data", request_data)
                     .andReturnValue(retval);
            break;
    }

}

connector_status_t connector_initiate_action(connector_handle_t const handle, connector_initiate_request_t const request, void const * const request_data)
{
    switch (request)
    {
        case connector_initiate_transport_start:
        {
            mock("connector_initiate_action").actualCall("connector_initiate_action")
                    .withParameter("handle", handle)
                    .withParameter("request", request)
                    .withParameterOfType("connector_transport_t", "request_data", (void *)request_data);
            break;
        }
        case connector_initiate_terminate:
        {
            mock("connector_initiate_action").actualCall("connector_initiate_action")
                    .withParameter("handle", handle)
                    .withParameter("request", request)
                    .withParameter("request_data", (void *)request_data);

            mock_connector_api_info_t * mock_info = get_mock_connector_api_info(handle);

            if (mock_info && mock_info->ccapi_handle != NULL)
            {
                /* printf("terminate: handle=%p\n",(void*)handle); */
                ((ccapi_data_t *)mock_info->ccapi_handle)->thread.connector_run->status = CCAPI_THREAD_REQUEST_STOP;
                mock_info->connector_run_retval = connector_device_terminated;
            }

            break;
        }
        case connector_initiate_transport_stop:
        case connector_initiate_send_data:
#ifdef CONNECTOR_SHORT_MESSAGE
        case connector_initiate_ping_request:
        case connector_initiate_session_cancel:
        case connector_initiate_session_cancel_all:
#endif
            break;

    }


    return (connector_status_t)mock("connector_initiate_action").returnValue().getIntValue();
}
