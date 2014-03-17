#include "ccapi/ccapi.h"
#include "ccimp/ccimp_os.h"
#include "ccapi_definitions.h"

static ccapi_start_error_t check_params(ccapi_start_t const * const start)
{
    ccapi_start_error_t error = CCAPI_START_ERROR_NONE;
    uint8_t invalid_device_id[DEVICE_ID_LENGTH] = {0x00};

    if (start->vendor_id == 0x00)
    {
        error = CCAPI_START_ERROR_INVALID_VENDORID;
        goto done;
    }

    if (memcmp(start->device_id, invalid_device_id, sizeof invalid_device_id) == 0)
    {
        error = CCAPI_START_ERROR_INVALID_DEVICEID;
        goto done;
    }

    if (start->device_cloud_url == NULL || start->device_cloud_url[0] == '\0')
    {
        error = CCAPI_START_ERROR_INVALID_URL;
        goto done;
    }

    if (start->device_type == NULL || start->device_type[0] == '\0')
    {
        error = CCAPI_START_ERROR_INVALID_DEVICETYPE;
        goto done;
    }

done:
    return error;
}

static ccapi_start_error_t check_malloc(void * p)
{
    if (p == NULL)
        return CCAPI_START_ERROR_INSUFFICIENT_MEMORY;
    else
        return CCAPI_START_ERROR_NONE;
}

ccapi_start_error_t ccxapi_start(ccapi_data_t * ccapi_data, ccapi_start_t const * const start)
{
    ccapi_start_error_t error = CCAPI_START_ERROR_NONE;

    if (start == NULL)
    {
        error = CCAPI_START_ERROR_NULL_PARAMETER;
        goto done;
    }

    error = check_params(start);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;

#if (defined CCIMP_DEBUG_ENABLED)
    {
        ccapi_config_debug_error_t config_debug_error;
        config_debug_error = ccapi_config_debug(start->debug.init_zones, start->debug.init_level);
        if (config_debug_error != CCAPI_CONFIG_DEBUG_ERROR_NONE)
        {
            error = CCAPI_START_ERROR_INVALID_DEBUG_CONFIG;
            goto done;
        }
    }
#endif

    ccapi_data->config.vendor_id = start->vendor_id;
    memcpy(ccapi_data->config.device_id, start->device_id, sizeof ccapi_data->config.device_id);

    ccapi_data->config.device_type = ccapi_malloc(strlen(start->device_type) + 1);
    error = check_malloc(ccapi_data->config.device_type);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;
    strcpy(ccapi_data->config.device_type, start->device_type);

    ccapi_data->config.device_cloud_url = ccapi_malloc(strlen(start->device_cloud_url) + 1);
    error = check_malloc(ccapi_data->config.device_cloud_url);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;
    strcpy(ccapi_data->config.device_cloud_url, start->device_cloud_url);

    ccapi_data->config.cli_supported = start->service.cli == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_data->config.receive_supported = start->service.receive == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_data->config.firmware_supported = start->service.firmware == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_data->config.rci_supported = start->service.rci == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_data->config.filesystem_supported = start->service.file_system == NULL ? CCAPI_FALSE : CCAPI_TRUE;

    ccapi_data->connector_handle = connector_init(ccapi_connector_callback, ccapi_data);
    error = check_malloc(ccapi_data->connector_handle);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;

    ccapi_data->signature = ccapi_signature;

    {
        ccapi_data->thread.connector_run = ccapi_malloc(sizeof *ccapi_data->thread.connector_run);
        error = check_malloc(ccapi_data->thread.connector_run);
        if (error != CCAPI_START_ERROR_NONE)
            goto done;

        ccapi_data->thread.connector_run->status = CCAPI_THREAD_REQUEST_START;
        ccapi_data->thread.connector_run->ccimp_info.argument = ccapi_data;
        ccapi_data->thread.connector_run->ccimp_info.start = ccapi_connector_run_thread;
        ccapi_data->thread.connector_run->ccimp_info.type = CCIMP_THREAD_CONNECTOR_RUN;

        if (ccimp_create_thread(&ccapi_data->thread.connector_run->ccimp_info) != CCIMP_STATUS_OK)
        {
            error = CCAPI_START_ERROR_THREAD_FAILED;
            goto done;
        }

        do
        {          
            ccimp_os_yield();
        } while (ccapi_data->thread.connector_run->status == CCAPI_THREAD_REQUEST_START);
    }
done:
    /* TODO: Free when error !! */

    /* ccapi_debug_printf(ZONE_START_STOP, LEVEL_INFO, "ccapi_start ret %d\n", error); */

    return error;
}

ccapi_start_error_t ccapi_start(ccapi_start_t const * const start)
{
	ccapi_start_error_t error;

	ccapi_data_single_instance = ccapi_malloc(sizeof *ccapi_data_single_instance);
    error = check_malloc(ccapi_data_single_instance);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;
    error = ccxapi_start(ccapi_data_single_instance, start);
done:
	return error;
}
