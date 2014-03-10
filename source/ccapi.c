/*
 * ccapi.c
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#include "ccapi_definitions.h"
#include "connector_api.h"

char const ccapi_signature[] = "CCAPI_SIG"; /* TODO: CCAPI_SW_VERSION */

ccapi_data_t * ccapi_data;

void * ccapi_malloc(size_t size)
{
    ccimp_malloc_t malloc_struct;
    ccimp_status_t status;

    malloc_struct.size = size;
    status = ccimp_malloc(&malloc_struct);

    return status == CCIMP_STATUS_OK ? malloc_struct.ptr : NULL;
}

void ccapi_connector_run_thread(void * const argument)
{
    ccapi_data_t * local_ccapi_data = argument;

    assert (local_ccapi_data != NULL);

    /* local_ccapi_data is corrupted, it's likely the implementer made it wrong passing argument to the new thread */
    ASSERT_CHECK ((local_ccapi_data->signature == ccapi_signature), "Bad ccapi_signature");

    local_ccapi_data->thread.connector_run->status = CCAPI_THREAD_RUNNING;
    while (local_ccapi_data->thread.connector_run->status == CCAPI_THREAD_RUNNING)
    {
        connector_status_t const status = connector_run(local_ccapi_data->connector_handle);

        /* It's very unlikely that we get this error as we have already verified ccapi_signature */
        ASSERT_CHECK ((status != connector_init_error), "Bad connector_signature");

        switch(status)
        {
            default:
                break;
        }            
    }
    ASSERT(local_ccapi_data->thread.connector_run->status == CCAPI_THREAD_REQUEST_STOP);
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

#if (defined CCAPI_DEBUG)
/* TODO: Can we do this? It's done for RCI*/
#include <stdarg.h>

void debug_printf(ccapi_debug_zones_t zone, ccapi_debug_level_t level, va_list args, char const * const format)
{
    char formatted_string[250];

    /* Go to the safe side, if we can't reach the zones/level info, just print it */
    if (ccapi_data != NULL) 
    {

        /* Evaluate level */
        if (level < ccapi_data->dbg_level)
            return;

        /* zone */
        if (!(zone & ccapi_data->dbg_zones))
            return;
    }

    /* TODO: Call connector_snprintf so we check buffer size */
    vsprintf(formatted_string, format, args);
    va_end(args);

    ccimp_debug_printf(formatted_string);
}

void connector_debug_printf(char const * const format, ...)
{
    va_list args;
    
    va_start(args, format);
    debug_printf(ZONE_LAYER1, LEVEL_ERROR, args, format);
    va_end(args);
}


void ccapi_debug_printf(ccapi_debug_zones_t zone, ccapi_debug_level_t level, char const * const format, ...)
{
    va_list args;
    
    va_start(args, format);
    debug_printf(zone, level, args, format);
    va_end(args);
}
#else
void ccapi_debug_printf(ccapi_debug_zones_t zone, ccapi_debug_level_t level, char const * const format, ...)
{  
    (void)zone;
    (void)level;
    (void)format;
}
#endif /* (defined CCAPI_DEBUG) */