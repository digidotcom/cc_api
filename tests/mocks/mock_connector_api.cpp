/*
 * mock_connector_api.cpp
 *
 *  Created on: Feb 14, 2014
 *      Author: spastor
 */

#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#include "mocks.h"

static mock_connector_api_info_t mock_info[MAX_INFO];
static sem_t sem;

mock_connector_api_info_t * mock_connector_api_info_alloc(connector_handle_t connector_handle, ccapi_handle_t ccapi_handle)
{
    size_t i;

    sem_wait(&sem);

    for (i = 0; i < MAX_INFO; i++)
    {
        if (mock_info[i].used == connector_false)
        {
            mock_info[i].used = connector_true;
            mock_info[i].connector_handle = connector_handle;
            mock_info[i].ccapi_handle = ccapi_handle;	
            mock_info[i].connector_run_retval = connector_idle;
            mock_info[i].connector_initiate_transport_start_info.init_transport = CCAPI_TRUE;
            mock_info[i].connector_initiate_transport_stop_info.stop_transport = CCAPI_TRUE;

            sem_post(&sem);

            return &mock_info[i];
        }
    }

    sem_post(&sem);

    return NULL;
}

mock_connector_api_info_t * mock_connector_api_info_get_nosem(connector_handle_t connector_handle)
{
    size_t i;

    for (i = 0; i < MAX_INFO; i++)
    {
        if (mock_info[i].used == connector_true && mock_info[i].connector_handle == connector_handle)
            return &mock_info[i];
    }

    assert (0);

    return NULL;
}

mock_connector_api_info_t * mock_connector_api_info_get(connector_handle_t connector_handle)
{
    mock_connector_api_info_t * mock_info;

    sem_wait(&sem);

    mock_info = mock_connector_api_info_get_nosem(connector_handle);

    sem_post(&sem);

    return mock_info;
}

void mock_connector_api_info_free(connector_handle_t connector_handle)
{
    mock_connector_api_info_t * mock_info;

    sem_wait(&sem);

    mock_info = mock_connector_api_info_get_nosem(connector_handle);
    mock_info->used = connector_false;
    mock_info->ccapi_handle = NULL;

    sem_post(&sem);
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
    connector_handle_t connector_handle;

    behavior = mock("connector_init").getData("behavior").getIntValue();

    if (behavior == MOCK_CONNECTOR_INIT_ENABLED)
    {
        mock("connector_init").actualCall("connector_init").withParameter("callback", u.vp);
        connector_handle = mock("connector_init").returnValue().getPointerValue();
    }
    else
    {
        connector_handle = malloc(sizeof (int)); /* Return a different pointer each time. */
    } 
    
    if (connector_handle != NULL)
        mock_connector_api_info_alloc(connector_handle, (ccapi_handle_t)context);

    return connector_handle;
}

void Mock_connector_run_create(void)
{
    if (sem_init(&sem, 0, 1) == -1)
    {
        printf("Mock_connector_run_create: sem_init error\n");
    }

    return;
}

void Mock_connector_run_destroy(void)
{
    size_t i;
    connector_bool_t finished;

    do
    {
        finished = connector_true;

        sem_wait(&sem);

        for (i = 0; i < MAX_INFO; i++)
        {
            if (mock_info[i].used == connector_true)
            {
                mock_info[i].connector_run_retval = connector_device_terminated;

                sem_post(&sem);

                usleep(100);

                finished = connector_false;
                break;
            }
        }
    } while (finished == connector_false);

    if (sem_destroy(&sem) == -1) printf("ccimp_os_syncr_destroy error\n");

    return;
}

connector_status_t connector_run(connector_handle_t const handle)
{
    connector_status_t connector_run_retval;

    /* printf("+connector_run: handle=%p\n",(void*)handle);	*/
    do {
        mock_connector_api_info_t * mock_info = mock_connector_api_info_get(handle);
        connector_run_retval = mock_info != NULL? mock_info->connector_run_retval:connector_idle;

        if (connector_run_retval == connector_idle || connector_run_retval == connector_working || connector_run_retval == connector_pending || connector_run_retval == connector_active || connector_run_retval == connector_success)
        {
            ccimp_os_yield();
        }
    } while (connector_run_retval == connector_idle || connector_run_retval == connector_working || connector_run_retval == connector_pending || connector_run_retval == connector_active || connector_run_retval == connector_success);
    /* printf("-connector_run: handle=%p\n",(void*)handle); */

    switch (connector_run_retval)
    {
        case connector_device_terminated:
        case connector_init_error:
            mock_connector_api_info_free(handle);
            break;
        default:
            break;
    }

    return connector_run_retval;
}

/* * * * * * * * * * connector_initiate_action() * * * * * * * * * */
void Mock_connector_initiate_action_create(void)
{
    mock().installComparator("connector_transport_t", connector_transport_t_comparator);
    mock().installComparator("connector_initiate_stop_request_t", connector_initiate_stop_request_t_comparator);
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
        case connector_initiate_transport_stop:
            mock("connector_initiate_action").expectOneCall("connector_initiate_action")
                     .withParameter("handle", handle)
                     .withParameter("request", request)
                     .withParameterOfType("connector_initiate_stop_request_t", "request_data", request_data)
                     .andReturnValue(retval);
            break;
        case connector_initiate_transport_start:
            mock("connector_initiate_action").expectOneCall("connector_initiate_action")
                     .withParameter("handle", handle)
                     .withParameter("request", request)
                     .withParameterOfType("connector_transport_t", "request_data", request_data)
                     .andReturnValue(retval);
            break;
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
    mock_connector_api_info_t * mock_info = mock_connector_api_info_get(handle);
    ccapi_data_t * ccapi_data = (ccapi_data_t *)mock_info->ccapi_handle;

    assert(ccapi_data != NULL);

    switch (request)
    {
        case connector_initiate_transport_stop:
        {
            mock("connector_initiate_action").actualCall("connector_initiate_action")
                    .withParameter("handle", handle)
                    .withParameter("request", request)
                    .withParameterOfType("connector_initiate_stop_request_t", "request_data", (void *)request_data);
            if (ccapi_data->transport_tcp.connected && mock_info->connector_initiate_transport_stop_info.stop_transport)
            {
                 connector_request_id_t request_id;
                 connector_initiate_stop_request_t stop_status = {connector_transport_tcp, connector_wait_sessions_complete, NULL};

                 request_id.status_request = connector_request_id_status_stop_completed;
                 ccapi_connector_callback(connector_class_id_status, request_id, &stop_status, (void *)ccapi_data);
            }
            break;
        }
        case connector_initiate_transport_start:
        {
            mock("connector_initiate_action").actualCall("connector_initiate_action")
                    .withParameter("handle", handle)
                    .withParameter("request", request)
                    .withParameterOfType("connector_transport_t", "request_data", (void *)request_data);
            if (!ccapi_data->transport_tcp.connected && mock_info->connector_initiate_transport_start_info.init_transport)
            {
                 connector_request_id_t request_id;
                 connector_status_tcp_event_t tcp_status = {connector_tcp_communication_started};

                 request_id.status_request = connector_request_id_status_tcp;
                 ccapi_connector_callback(connector_class_id_status, request_id, &tcp_status, (void *)ccapi_data);
            }
            break;
        }
        case connector_initiate_terminate:
        {
            mock("connector_initiate_action").actualCall("connector_initiate_action")
                    .withParameter("handle", handle)
                    .withParameter("request", request)
                    .withParameter("request_data", (void *)request_data);

            if (mock_info)
            {
                ccapi_data->thread.connector_run->status = CCAPI_THREAD_REQUEST_STOP;
                mock_info->connector_run_retval = connector_device_terminated;
            }

            break;
        }
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
