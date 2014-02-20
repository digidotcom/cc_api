#include "ccapi/ccapi.h"
#include "ccimp/ccimp_os.h"
#include "ccapi_definitions.h"

static ccapi_init_error_t check_params(ccapi_start_t const * const start)
{
    ccapi_init_error_t error = CCAPI_INIT_ERROR_NONE;
    uint8_t invalid_device_id[DEVICE_ID_LENGTH] = {0x00};

    if (start->vendor_id == 0x00)
    {
        error = CCAPI_INIT_ERROR_INVALID_VENDORID;
        goto done;
    }

    if (memcmp(start->device_id, invalid_device_id, sizeof invalid_device_id) == 0)
    {
        error = CCAPI_INIT_ERROR_INVALID_DEVICEID;
        goto done;
    }

    if (start->device_cloud_url == NULL || start->device_cloud_url[0] == '\0')
    {
        error = CCAPI_INIT_ERROR_INVALID_URL;
        goto done;
    }

    if (start->device_type == NULL || start->device_type[0] == '\0')
    {
        error = CCAPI_INIT_ERROR_INVALID_DEVICETYPE;
        goto done;
    }

done:
    return error;
}

static ccapi_init_error_t check_malloc(void * p)
{
    if (p == NULL)
        return CCAPI_INIT_ERROR_INSUFFICIENT_MEMORY;
    else
        return CCAPI_INIT_ERROR_NONE;
}

ccapi_init_error_t ccapi_start(ccapi_start_t const * const start)
{
    ccapi_init_error_t error = CCAPI_INIT_ERROR_NONE;

    if (start == NULL)
    {
        error = CCAPI_INIT_ERROR_NULL_PARAMETER;
        goto done;
    }

    error = check_params(start);
    if (error != CCAPI_INIT_ERROR_NONE)
        goto done;

    ccapi_data = ccapi_malloc(sizeof *ccapi_data);
    error = check_malloc(ccapi_data);
    if (error != CCAPI_INIT_ERROR_NONE)
        goto done;

    ccapi_data->config.vendor_id = start->vendor_id;
    memcpy(ccapi_data->config.device_id, start->device_id, sizeof ccapi_data->config.device_id);

    ccapi_data->config.device_type = ccapi_malloc(strlen(start->device_type) + 1);
    error = check_malloc(ccapi_data->config.device_type);
    if (error != CCAPI_INIT_ERROR_NONE)
        goto done;
    strcpy(ccapi_data->config.device_type, start->device_type);

    ccapi_data->config.device_cloud_url = ccapi_malloc(strlen(start->device_cloud_url) + 1);
    error = check_malloc(ccapi_data->config.device_cloud_url);
    if (error != CCAPI_INIT_ERROR_NONE)
        goto done;
    strcpy(ccapi_data->config.device_cloud_url, start->device_cloud_url);

    ccapi_data->config.cli_supported = start->service.cli == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_data->config.receive_supported = start->service.receive == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_data->config.firmware_supported = start->service.firmware == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_data->config.rci_supported = start->service.rci == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_data->config.filesystem_supported = start->service.file_system == NULL ? CCAPI_FALSE : CCAPI_TRUE;

    ccapi_data->connector_handle = connector_init(ccapi_connector_callback);
    error = check_malloc(ccapi_data->connector_handle);
    if (error != CCAPI_INIT_ERROR_NONE)
        goto done;

    {
        ccapi_data->thread.connector_run = ccapi_malloc(sizeof *ccapi_data->thread.connector_run);
        error = check_malloc(ccapi_data->thread.connector_run);
        if (error != CCAPI_INIT_ERROR_NONE)
            goto done;

        ccapi_data->thread.connector_run->status = CCAPI_THREAD_NOT_STARTED;
        ccapi_data->thread.connector_run->ccimp_info.argument = ccapi_data;
        ccapi_data->thread.connector_run->ccimp_info.start = ccapi_connector_run_thread;
        ccapi_data->thread.connector_run->ccimp_info.type = CCIMP_THREAD_CONNECTOR_RUN;

        if (ccimp_create_thread(&ccapi_data->thread.connector_run->ccimp_info) != CCAPI_TRUE)
        {
            error = CCAPI_INIT_ERROR_THREAD_FAILED;
            goto done;
        }

        do 
        {
            /* TODO call ccimp_os_yield() when available */
        } while(ccapi_data->thread.connector_run->status == CCAPI_THREAD_NOT_STARTED);

    }
done:
    return error;
}
