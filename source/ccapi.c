/*
 * ccapi.c
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#include "internal/ccapi_definitions.h"
#include "connector_api.h"

connector_handle_t ccapi_connector_handle;

ccapi_config_t * ccapi_config;

connector_callback_status_t ccapi_config_handler(connector_request_id_config_t config_request, void * const data)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (config_request)
    {
        case connector_request_id_config_device_id:
            {
                connector_config_pointer_data_t * device_id = data;
                memcpy(device_id->data, ccapi_config->device_id, device_id->bytes_required);
            }
            break;
        case connector_request_id_config_device_cloud_url:
            {
                connector_config_pointer_string_t * device_cloud = data;
                device_cloud->string = ccapi_config->device_cloud_url;
                device_cloud->length = strlen(ccapi_config->device_cloud_url);
            }
            break;
        case connector_request_id_config_vendor_id:
            {
                connector_config_vendor_id_t * vendor_id = data;
                vendor_id->id = ccapi_config->vendor_id;
            }
            break;
        case connector_request_id_config_device_type:
            {
                connector_config_pointer_string_t * device_type = data;
                device_type->string = ccapi_config->device_type;
                device_type->length = strlen(ccapi_config->device_type);
            }
            break;
        default:
            assert(0);
            break;
    }
    return status;
}
connector_callback_status_t ccapi_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id, void * const data)
{
    connector_callback_status_t status = connector_callback_error;

    switch (class_id)
    {
        case connector_class_id_config:
            status = ccapi_config_handler(request_id.config_request, data);
            break;
        default:
            assert(0);
            break;
    }

    return status;
}
