/*
 * ccapi.c
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#include "ccapi_definitions.h"
#include "connector_api.h"

ccapi_data_t * ccapi_data;


void ccapi_connector_run_thread(void * const argument)
{
    ccapi_data_t * local_ccapi_data = argument;

    local_ccapi_data->thread.connector_run->status = CCAPI_THREAD_RUNNING;
    while (local_ccapi_data->thread.connector_run->status == CCAPI_THREAD_RUNNING)
    {
        connector_status_t const status = connector_run(local_ccapi_data->connector_handle);
        assert(status != connector_open_error);
    }
    assert(local_ccapi_data->thread.connector_run->status == CCAPI_THREAD_REQUEST_STOP);
}

connector_callback_status_t ccapi_config_handler(connector_request_id_config_t config_request, void * const data)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (config_request)
    {
        case connector_request_id_config_device_id:
            {
                connector_config_pointer_data_t * device_id = data;
                memcpy(device_id->data, ccapi_data->config.device_id, device_id->bytes_required);
            }
            break;
        case connector_request_id_config_device_cloud_url:
            {
                connector_config_pointer_string_t * device_cloud = data;
                device_cloud->string = ccapi_data->config.device_cloud_url;
                device_cloud->length = strlen(ccapi_data->config.device_cloud_url);
            }
            break;
        case connector_request_id_config_vendor_id:
            {
                connector_config_vendor_id_t * vendor_id = data;
                vendor_id->id = ccapi_data->config.vendor_id;
            }
            break;
        case connector_request_id_config_device_type:
            {
                connector_config_pointer_string_t * device_type = data;
                device_type->string = ccapi_data->config.device_type;
                device_type->length = strlen(ccapi_data->config.device_type);
            }
            break;
        case connector_request_id_config_firmware_facility:
            {
                connector_config_supported_t * firmware_supported = data;
                firmware_supported->supported = ccapi_data->config.firmware_supported;
            }
            break;
        case connector_request_id_config_file_system:
            {
                connector_config_supported_t * filesystem_supported = data;
                filesystem_supported->supported = ccapi_data->config.filesystem_supported;
            }
            break;
        case connector_request_id_config_remote_configuration:
            {
                connector_config_supported_t * rci_supported = data;
                rci_supported->supported = ccapi_data->config.rci_supported;
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
