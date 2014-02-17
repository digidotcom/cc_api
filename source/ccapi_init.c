#include "ccapi/ccapi.h"
#include "ccimp/ccimp_os.h"
#include "internal/ccapi_definitions.h"

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

    ccapi_config = ccimp_malloc(sizeof *ccapi_config);
    error = check_malloc(ccapi_config);
    if (error != CCAPI_INIT_ERROR_NONE)
        goto done;

    ccapi_config->vendor_id = start->vendor_id;
    memcpy(ccapi_config->device_id, start->device_id, sizeof ccapi_config->device_id);

    ccapi_config->device_type = ccimp_malloc(strlen(start->device_type) + 1);
    error = check_malloc(ccapi_config->device_type);
    if (error != CCAPI_INIT_ERROR_NONE)
        goto done;
    strcpy(ccapi_config->device_type, start->device_type);

    ccapi_config->device_cloud_url = ccimp_malloc(strlen(start->device_cloud_url) + 1);
    error = check_malloc(ccapi_config->device_cloud_url);
    if (error != CCAPI_INIT_ERROR_NONE)
        goto done;
    strcpy(ccapi_config->device_cloud_url, start->device_cloud_url);

    ccapi_config->cli_supported = start->service.cli == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_config->receive_supported = start->service.receive == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_config->firmware_supported = start->service.firmware == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_config->rci_supported = start->service.rci == NULL ? CCAPI_FALSE : CCAPI_TRUE;
    ccapi_config->filesystem_supported = start->service.file_system == NULL ? CCAPI_FALSE : CCAPI_TRUE;

    ccapi_connector_handle = connector_init(ccapi_connector_callback);
    error = check_malloc(ccapi_connector_handle);
    if (error != CCAPI_INIT_ERROR_NONE)
        goto done;

done:
    return error;
}
