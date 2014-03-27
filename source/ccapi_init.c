#include "ccapi/ccapi.h"
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

static void free_ccapi_data_internal_resources(ccapi_data_t * const ccapi_data)
{
    ASSERT_MSG_GOTO(ccapi_data != NULL, done);

    reset_heap_ptr(&ccapi_data->config.device_type);
    reset_heap_ptr(&ccapi_data->config.device_cloud_url);
    reset_heap_ptr(&ccapi_data->thread.connector_run);
done:
    return; 
}

static ccapi_start_error_t check_malloc(void const * const p)
{
    if (p == NULL)
        return CCAPI_START_ERROR_INSUFFICIENT_MEMORY;
    else
        return CCAPI_START_ERROR_NONE;
}

/* This function allocates ccapi_data_t so other ccXapi_* functions can use it as a handler */
ccapi_start_error_t ccxapi_start(ccapi_handle_t * const ccapi_handle, ccapi_start_t const * const start)
{
    ccapi_start_error_t error = CCAPI_START_ERROR_NONE;
    ccapi_data_t * ccapi_data;

    ccapi_data = ccapi_malloc(sizeof *ccapi_data);
    *ccapi_handle = (ccapi_handle_t)ccapi_data;

    error = check_malloc(ccapi_data);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;

    /* Initialize one single time for all connector instances the logging syncr object */
    if (logging_syncr == NULL)
    {
        ccimp_os_syncr_create_t create_data;
    
        if (ccimp_os_syncr_create(&create_data) == CCIMP_STATUS_OK &&
                         ccapi_syncr_release(create_data.syncr_object) == CCIMP_STATUS_OK)
            logging_syncr = create_data.syncr_object;
    }

    ccapi_data->config.device_type = NULL;
    ccapi_data->config.device_cloud_url = NULL;
    ccapi_data->thread.connector_run = NULL;

    if (start == NULL)
    {
        error = CCAPI_START_ERROR_NULL_PARAMETER;
        goto done;
    }

    error = check_params(start);
    if (error != CCAPI_START_ERROR_NONE)
        goto done;

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
    if (error != CCAPI_START_ERROR_NONE && ccapi_data != NULL)
    {
        free_ccapi_data_internal_resources(ccapi_data);
        ccapi_free(ccapi_data);
    }

    ccapi_logging_line("ccapi_start ret %d", error);

    return error;
}

ccapi_stop_error_t ccxapi_stop(ccapi_handle_t ccapi_handle, ccapi_stop_t behavior)
{
    ccapi_data_t * ccapi_data = (ccapi_data_t *) ccapi_handle;
    ccapi_stop_error_t error = CCAPI_STOP_ERROR_NOT_STARTED;

    UNUSED_ARGUMENT(behavior);
    if (ccapi_data == NULL || ccapi_data->thread.connector_run->status == CCAPI_THREAD_NOT_STARTED)
        goto done;
    {
        connector_status_t connector_status = connector_initiate_action(ccapi_data->connector_handle, connector_initiate_terminate, NULL);
        switch(connector_status)
        {
        case connector_success:
            error = CCAPI_STOP_ERROR_NONE;
            break;
        case connector_init_error:
            break;
        case connector_invalid_data_size:
            break;
        case connector_invalid_data_range:
            break;
        case connector_invalid_data:
            break;
        case connector_keepalive_error:
            break;
        case connector_bad_version:
            break;
        case connector_device_terminated:
            break;
        case connector_service_busy:
            break;
        case connector_invalid_response:
            break;
        case connector_no_resource:
            break;
        case connector_unavailable:
            break;
        case connector_idle:
            break;
        case connector_working:
            break;
        case connector_pending:
            break;
        case connector_active:
            break;
        case connector_abort:
            break;
        case connector_device_error:
            break;
        case connector_exceed_timeout:
            break;
        case connector_invalid_payload_packet:
            break;
        case connector_open_error:
            break;
        }
    }

    do {
        ccimp_os_yield();
    } while (ccapi_data->thread.connector_run->status != CCAPI_THREAD_NOT_STARTED);

done:
    if (error == CCAPI_STOP_ERROR_NONE)
    {
        free_ccapi_data_internal_resources(ccapi_data);
        ccapi_free(ccapi_data);
    }

    return error;
}

ccapi_start_error_t ccapi_start(ccapi_start_t const * const start)
{
	ccapi_start_error_t error;

	if (ccapi_data_single_instance != NULL)
	{
	    error = CCAPI_START_ERROR_ALREADY_STARTED;
	    goto done;
	}
    error = ccxapi_start(&ccapi_data_single_instance, start);

    if (error != CCAPI_START_ERROR_NONE)
    {
        ccapi_data_single_instance = NULL;
    }

done:
	return error;
}

ccapi_stop_error_t ccapi_stop(ccapi_stop_t behavior)
{
    ccapi_stop_error_t error;
    error = ccxapi_stop(ccapi_data_single_instance, behavior);
    if (error != CCAPI_STOP_ERROR_NONE)
    {
        ccapi_data_single_instance = NULL;
    }

    return error;
}
