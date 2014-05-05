#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

#ifdef CCIMP_DATA_SERVICE_ENABLED

ccapi_dp_error_t ccapi_dp_create_collection(ccapi_dp_collection_t * * const dp_collection)
{
    ccapi_dp_error_t error = CCAPI_DP_ERROR_NONE;
    ccapi_dp_collection_t * collection;

    if (dp_collection == NULL)
    {
        error = CCAPI_DP_ERROR_INVALID_ARGUMENT;
        goto done;
    }

    collection = ccapi_malloc(sizeof **dp_collection);

    if (collection == NULL)
    {
        error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
        goto done;
    }

    collection->syncr = ccapi_syncr_create_and_release();
    if (collection->syncr == NULL)
    {
        error = CCAPI_DP_ERROR_SYNCR_FAILED;
        reset_heap_ptr(&collection);
        goto done;
    }

    collection->data_points_list = NULL;

done:
    if (dp_collection != NULL)
    {
        *dp_collection = collection;
    }

    return error;
}

ccapi_dp_error_t ccapi_dp_clear_collection(ccapi_dp_collection_t * const dp_collection)
{
    ccimp_status_t ccimp_status;
    ccapi_dp_error_t error = CCAPI_DP_ERROR_NONE;

    if (dp_collection == NULL)
    {
        error = CCAPI_DP_ERROR_INVALID_ARGUMENT;
        goto done;
    }

    ccimp_status = ccapi_syncr_acquire(dp_collection->syncr);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
            error = CCAPI_DP_ERROR_SYNCR_FAILED;
            /* No break */
        case CCIMP_STATUS_BUSY:
            goto done;
    }

    if (dp_collection->data_points_list != NULL)
    {
        ccapi_data_point_t * data_point = dp_collection->data_points_list;

        do {
            ccapi_data_point_t * const next_data_point = data_point->next;
            ccapi_free(data_point);

            data_point = next_data_point;
        } while (data_point != NULL);
    }

    ccimp_status = ccapi_syncr_release(dp_collection->syncr);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
            error = CCAPI_DP_ERROR_SYNCR_FAILED;
            /* No break */
        case CCIMP_STATUS_BUSY:
            goto done;
    }

done:
    return error;
}

ccapi_dp_error_t ccapi_dp_destroy_collection(ccapi_dp_collection_t * const dp_collection)
{
    ccimp_status_t ccimp_status;
    ccapi_dp_error_t error;

    error = ccapi_dp_clear_collection(dp_collection);
    if (error != CCAPI_DP_ERROR_NONE)
    {
        goto done;
    }

    ccimp_status = ccapi_syncr_destroy(dp_collection->syncr);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            dp_collection->syncr = NULL;
            break;
        case CCIMP_STATUS_ERROR:
            error = CCAPI_DP_ERROR_SYNCR_FAILED;
            /* No break */
        case CCIMP_STATUS_BUSY:
            goto done;
    }

    ccapi_free(dp_collection);
done:
    return error;
}

#endif
