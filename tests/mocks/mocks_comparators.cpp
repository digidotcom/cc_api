#include "mocks.h"

#define ValueToStringFunction(type)     static SimpleString type##_ValueToString(void * object) {(void)object; return #type;}
#define compare_pointers(object1, object2)  do {if (object1 == NULL || object2 == NULL) return false; else if (object1 == object2) return true;} while(0)
#define compare_strings(string1, string2)   do {if (string1 != string2 && (string1 == NULL || string2 == NULL)) return false;\
                                                else if (strcmp(string1, string2) != 0) return false;                          } while(0)

ValueToStringFunction(ccimp_network_open_t)
ValueToStringFunction(ccimp_network_send_t)
ValueToStringFunction(ccimp_network_receive_t)
ValueToStringFunction(ccimp_network_close_t)
ValueToStringFunction(connector_transport_t)
ValueToStringFunction(connector_initiate_stop_request_t)

static SimpleString ccimp_create_thread_info_t_ValueToString(void* object)
{
    return StringFrom(((ccimp_create_thread_info_t*)object)->type);
}

static bool ccimp_network_open_t_IsEqual(void * object1, void * object2)
{
    ccimp_network_open_t * open_data_1 = (ccimp_network_open_t*)object1;
    ccimp_network_open_t * open_data_2 = (ccimp_network_open_t*)object2;

    compare_pointers(object1, object2);
    compare_strings(open_data_1->device_cloud.url, open_data_2->device_cloud.url);

    if (open_data_1->handle != open_data_2->handle)
        return false;

    return true;
}

static bool ccimp_network_send_t_IsEqual(void * object1, void * object2)
{
    ccimp_network_send_t * send_data_1 = (ccimp_network_send_t*)object1;
    ccimp_network_send_t * send_data_2 = (ccimp_network_send_t*)object2;

    compare_pointers(object1, object2);

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

static bool ccimp_network_receive_t_IsEqual(void * object1, void * object2)
{
    ccimp_network_receive_t * receive_data_1 = (ccimp_network_receive_t*)object1;
    ccimp_network_receive_t * receive_data_2 = (ccimp_network_receive_t*)object2;

    compare_pointers(object1, object2);

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

static bool ccimp_network_close_t_IsEqual(void * object1, void * object2)
{
    ccimp_network_close_t * close_data_1 = (ccimp_network_close_t*)object1;
    ccimp_network_close_t * close_data_2 = (ccimp_network_close_t*)object2;

    compare_pointers(object1, object2);
    if (close_data_1->handle != close_data_2->handle)
        return false;

    return true;
}

static bool ccimp_create_thread_info_t_IsEqual(void* object1, void* object2)
{
    ccimp_create_thread_info_t * create_thread_info1 = (ccimp_create_thread_info_t*)object1;
    ccimp_create_thread_info_t * create_thread_info2 = (ccimp_create_thread_info_t*)object2;

    compare_pointers(object1, object2);
    if (create_thread_info1->argument != create_thread_info2->argument)
        return false;
    if (create_thread_info1->type != create_thread_info2->type)
        return false;

    return true;
}

static bool connector_transport_t_IsEqual(void * object1, void * object2)
{
    connector_transport_t * connector_transport_1 = (connector_transport_t *)object1;
    connector_transport_t * connector_transport_2 = (connector_transport_t *)object2;

    compare_pointers(object1, object2);

    if (*connector_transport_1 != *connector_transport_2)
        return false;

    return true;
}

static bool connector_initiate_stop_request_t_IsEqual(void * object1, void * object2)
{
    connector_initiate_stop_request_t * connector_initiate_stop_request_1 = (connector_initiate_stop_request_t *)object1;
    connector_initiate_stop_request_t * connector_initiate_stop_request_2 = (connector_initiate_stop_request_t *)object2;

    compare_pointers(object1, object2);
    if (connector_initiate_stop_request_1->condition != connector_initiate_stop_request_2->condition)
        return false;

    if (connector_initiate_stop_request_1->transport != connector_initiate_stop_request_2->transport)
        return false;

    if (connector_initiate_stop_request_1->user_context != connector_initiate_stop_request_2->user_context)
        return false;

    return true;
}

MockFunctionComparator ccimp_network_open_t_comparator(ccimp_network_open_t_IsEqual, ccimp_network_open_t_ValueToString);
MockFunctionComparator ccimp_network_send_t_comparator(ccimp_network_send_t_IsEqual, ccimp_network_send_t_ValueToString);
MockFunctionComparator ccimp_network_receive_t_comparator(ccimp_network_receive_t_IsEqual, ccimp_network_receive_t_ValueToString);
MockFunctionComparator ccimp_network_close_t_comparator(ccimp_network_close_t_IsEqual, ccimp_network_close_t_ValueToString);
MockFunctionComparator ccimp_create_thread_info_t_comparator(ccimp_create_thread_info_t_IsEqual, ccimp_create_thread_info_t_ValueToString);
MockFunctionComparator connector_transport_t_comparator(connector_transport_t_IsEqual, connector_transport_t_ValueToString);
MockFunctionComparator connector_initiate_stop_request_t_comparator(connector_initiate_stop_request_t_IsEqual, connector_initiate_stop_request_t_ValueToString);
