
#include "mock_ccimp_network_tcp.h"

/* * * * * * * * * * OPEN * * * * * * * * * */
SimpleString ccimp_network_open_t_ValueToString(void* object)
{
    UNUSED_ARGUMENT(object);
    return "ccimp_network_open_t";
}

bool ccimp_network_open_t_IsEqual(void * object1, void * object2)
{
    ccimp_network_open_t * open_data_1 = (ccimp_network_open_t*)object1;
    ccimp_network_open_t * open_data_2 = (ccimp_network_open_t*)object2;

    if (open_data_1 == open_data_2)
        return true;

    if (open_data_1 == NULL || open_data_2 == NULL)
        return false;

    if (strcmp(open_data_1->device_cloud.url, open_data_2->device_cloud.url) != 0)
        return false;

    if (open_data_1->handle != open_data_2->handle)
        return false;

    return true;
}


void Mock_ccimp_network_tcp_open_create(void)
{
    static MockFunctionComparator ccimp_network_open_t_comparator(ccimp_network_open_t_IsEqual, ccimp_network_open_t_ValueToString);
    mock().installComparator("ccimp_network_open_t", ccimp_network_open_t_comparator);

    return;
}

void Mock_ccimp_network_tcp_open_destroy(void)
{
    mock("ccimp_network_tcp_open").checkExpectations();
}

void Mock_ccimp_network_tcp_open_expectAndReturn(ccimp_network_open_t * expect, ccimp_status_t retval)
{
    mock("ccimp_network_tcp_open").expectOneCall("ccimp_network_tcp_open")
            .withParameterOfType("ccimp_network_open_t", "open_data", expect)
            .andReturnValue(retval);
}

/* * * * * * * * * * SEND * * * * * * * * * */
SimpleString ccimp_network_send_t_ValueToString(void* object)
{
    UNUSED_ARGUMENT(object);
    return "ccimp_network_send_t";
}

bool ccimp_network_send_t_IsEqual(void * object1, void * object2)
{
    ccimp_network_send_t * send_data_1 = (ccimp_network_send_t*)object1;
    ccimp_network_send_t * send_data_2 = (ccimp_network_send_t*)object2;

    if (send_data_1 == send_data_2)
        return true;

    if (send_data_1 == NULL || send_data_2 == NULL)
        return false;

    if (send_data_1->bytes_available != send_data_2->bytes_available)
        return false;

    if (send_data_1->bytes_used != send_data_2->bytes_used)
        return false;

    if (send_data_1->handle != send_data_2->handle)
        return false;

    if (memcmp(send_data_1->buffer, send_data_2->buffer, send_data_1->bytes_available) != 0)
        return false;

    return true;
}

void Mock_ccimp_network_tcp_send_create(void)
{
    static MockFunctionComparator ccimp_network_send_t_comparator(ccimp_network_send_t_IsEqual, ccimp_network_send_t_ValueToString);
    mock().installComparator("ccimp_network_send_t", ccimp_network_send_t_comparator);

    return;
}

void Mock_ccimp_network_tcp_send_destroy(void)
{
    mock("ccimp_network_tcp_send").checkExpectations();
}

void Mock_ccimp_network_tcp_send_expectAndReturn(ccimp_network_send_t * expect, ccimp_status_t retval)
{
    mock("ccimp_network_tcp_send").expectOneCall("ccimp_network_tcp_send")
            .withParameterOfType("ccimp_network_send_t", "send_data", expect)
            .andReturnValue(retval);
}

/* * * * * * * * * * RECEIVE * * * * * * * * * */
SimpleString ccimp_network_receive_t_ValueToString(void* object)
{
    UNUSED_ARGUMENT(object);
    return "ccimp_network_receive_t";
}

bool ccimp_network_receive_t_IsEqual(void * object1, void * object2)
{
    ccimp_network_receive_t * receive_data_1 = (ccimp_network_receive_t*)object1;
    ccimp_network_receive_t * receive_data_2 = (ccimp_network_receive_t*)object2;

    if (receive_data_1 == receive_data_2)
        return true;

    if (receive_data_1 == NULL || receive_data_2 == NULL)
        return false;

    if (receive_data_1->bytes_available != receive_data_2->bytes_available)
        return false;

    if (receive_data_1->bytes_used != receive_data_2->bytes_used)
        return false;

    if (receive_data_1->handle != receive_data_2->handle)
        return false;

    if (memcmp(receive_data_1->buffer, receive_data_2->buffer, receive_data_1->bytes_available) != 0)
        return false;

    return true;
}

void Mock_ccimp_network_tcp_receive_create(void)
{
    static MockFunctionComparator ccimp_network_receive_t_comparator(ccimp_network_receive_t_IsEqual, ccimp_network_receive_t_ValueToString);
    mock().installComparator("ccimp_network_receive_t", ccimp_network_receive_t_comparator);

    return;
}

void Mock_ccimp_network_tcp_receive_destroy(void)
{
    mock("ccimp_network_tcp_receive").checkExpectations();
}

void Mock_ccimp_network_tcp_receive_expectAndReturn(ccimp_network_receive_t * expect, ccimp_status_t retval)
{
    mock("ccimp_network_tcp_receive").expectOneCall("ccimp_network_tcp_receive")
            .withParameterOfType("ccimp_network_receive_t", "receive_data", expect)
            .andReturnValue(retval);
}

/* * * * * * * * * * CLOSE * * * * * * * * * */
SimpleString ccimp_network_close_t_ValueToString(void* object)
{
    UNUSED_ARGUMENT(object);
    return "ccimp_network_close_t";
}

bool ccimp_network_close_t_IsEqual(void * object1, void * object2)
{
    ccimp_network_close_t * close_data_1 = (ccimp_network_close_t*)object1;
    ccimp_network_close_t * close_data_2 = (ccimp_network_close_t*)object2;

    if (close_data_1 == close_data_2)
        return true;

    if (close_data_1 == NULL || close_data_2 == NULL)
        return false;

    if (close_data_1->handle != close_data_2->handle)
        return false;

    return true;
}

void Mock_ccimp_network_tcp_close_create(void)
{
    static MockFunctionComparator ccimp_network_close_t_comparator(ccimp_network_close_t_IsEqual, ccimp_network_close_t_ValueToString);
    mock().installComparator("ccimp_network_close_t", ccimp_network_close_t_comparator);

    return;
}

void Mock_ccimp_network_tcp_close_destroy(void)
{
    mock("ccimp_network_tcp_close").checkExpectations();
}

void Mock_ccimp_network_tcp_close_expectAndReturn(ccimp_network_close_t * expect, ccimp_status_t retval)
{
    mock("ccimp_network_tcp_close").expectOneCall("ccimp_network_tcp_close")
            .withParameterOfType("ccimp_network_close_t", "close_data", expect)
            .andReturnValue(retval);
}

extern "C" {
#include "CppUTestExt/MockSupport_c.h"
#include "ccapi_definitions.h"

ccimp_status_t ccimp_network_tcp_open(ccimp_network_open_t * const open_data)
{
    mock_scope_c("ccimp_network_tcp_open")->actualCall("ccimp_network_tcp_open")->withParameterOfType("ccimp_network_open_t", "open_data", (void *)open_data);

    return (ccimp_status_t)mock_scope_c("ccimp_network_tcp_open")->returnValue().value.intValue;
}

ccimp_status_t ccimp_network_tcp_send(ccimp_network_send_t * const send_data)
{
    mock_scope_c("ccimp_network_tcp_send")->actualCall("ccimp_network_tcp_send")->withParameterOfType("ccimp_network_send_t", "send_data", (void *)send_data);

    return (ccimp_status_t)mock_scope_c("ccimp_network_tcp_send")->returnValue().value.intValue;
}

ccimp_status_t ccimp_network_tcp_receive(ccimp_network_receive_t * const receive_data)
{
    mock_scope_c("ccimp_network_tcp_receive")->actualCall("ccimp_network_tcp_receive")->withParameterOfType("ccimp_network_receive_t", "receive_data", (void *)receive_data);

    return (ccimp_status_t)mock_scope_c("ccimp_network_tcp_receive")->returnValue().value.intValue;
}

ccimp_status_t ccimp_network_tcp_close(ccimp_network_close_t * const close_data)
{
    mock_scope_c("ccimp_network_tcp_close")->actualCall("ccimp_network_tcp_close")->withParameterOfType("ccimp_network_close_t", "close_data", (void *)close_data);

    return (ccimp_status_t)mock_scope_c("ccimp_network_tcp_close")->returnValue().value.intValue;
}
}
