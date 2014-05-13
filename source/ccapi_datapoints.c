#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

#if (defined CCIMP_DATA_SERVICE_ENABLED) && (defined CCIMP_DATA_POINTS_ENABLED)

ccapi_dp_error_t ccapi_dp_create_collection(ccapi_dp_collection_t * * const dp_collection)
{
    ccapi_dp_error_t error = CCAPI_DP_ERROR_NONE;
    ccapi_dp_collection_t * collection;

    if (dp_collection == NULL)
    {
        error = CCAPI_DP_ERROR_INVALID_ARGUMENT;
        goto done;
    }

    collection = ccapi_malloc(sizeof *collection);

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

    collection->ccapi_data_stream_list = NULL;

done:
    if (dp_collection != NULL)
    {
        *dp_collection = collection;
    }

    return error;
}

static void free_data_points_in_data_stream(connector_data_stream_t * data_stream)
{
    connector_data_point_t * data_point = data_stream->point;

    while (data_point != NULL)
    {
        connector_data_point_t * const next_point = data_point->next;
        ccapi_free(data_point);

        data_point = next_point;
    }
}

static void free_ccfsm_stream(connector_data_stream_t * const ccfsm_stream_info)
{
    if (ccfsm_stream_info->stream_id != NULL)
    {
        ccapi_free(ccfsm_stream_info->stream_id);
    }
    if (ccfsm_stream_info->unit != NULL)
    {
        ccapi_free(ccfsm_stream_info->unit);
    }
    if (ccfsm_stream_info->forward_to != NULL)
    {
        ccapi_free(ccfsm_stream_info->forward_to);
    }
    ccapi_free(ccfsm_stream_info);
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
        case CCIMP_STATUS_BUSY:
            error = CCAPI_DP_ERROR_SYNCR_FAILED;
            goto done;
    }

    if (dp_collection->ccapi_data_stream_list != NULL)
    {
        ccapi_dp_data_stream_t * ccapi_data_stream = dp_collection->ccapi_data_stream_list;

        while (ccapi_data_stream != NULL) {
            ccapi_dp_data_stream_t * const next_data_stream = ccapi_data_stream->next;

            ASSERT(ccapi_data_stream->ccfsm_data_stream != NULL);

            free_data_points_in_data_stream(ccapi_data_stream->ccfsm_data_stream);
            free_ccfsm_stream(ccapi_data_stream->ccfsm_data_stream);

            ccapi_free(ccapi_data_stream->arguments.list);
            ccapi_free(ccapi_data_stream);
            ccapi_data_stream = next_data_stream;
        }
    }

    ccimp_status = ccapi_syncr_release(dp_collection->syncr);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            error = CCAPI_DP_ERROR_SYNCR_FAILED;
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

static ccapi_bool_t valid_stream_id(char const * const stream_id)
{
    ccapi_bool_t is_valid = CCAPI_FALSE;
    int i = 0;

    if (stream_id == NULL || stream_id[0] == '\0')
    {
        goto done;
    }

    /*  [_\-\[\]:a-zA-Z0-9.!/]+ */
    while (stream_id[i] != '\0') {
        if (!isalnum(stream_id[i]) && stream_id[i] != '_' && stream_id[i] != '-' && stream_id[i] != '.' && stream_id[i] != '/' && stream_id[i] != '[' && stream_id[i] != ']' && stream_id[i] != '!' && stream_id[i] != '+')
        {
            goto done;
        }
        i++;
    }

    is_valid = CCAPI_TRUE;

done:
    return is_valid;
}

static ccapi_bool_t valid_format_string(char const * const format_string)
{
    ccapi_bool_t is_valid = CCAPI_FALSE;
    int i = 0;

    if (format_string == NULL || format_string[0] == '\0' || format_string[0] == ' ')
    {
        goto done;
    }

    while (format_string[i] != '\0') {
        if (!isalnum(format_string[i]) && format_string[i] != ' ' && format_string[i] != '_')
        {
            goto done;
        }
        i++;
    }

    is_valid = CCAPI_TRUE;

done:
    return is_valid;
}

static ccapi_bool_t valid_units(char const * const units)
{
    ccapi_bool_t is_valid = CCAPI_FALSE;

    if (units == NULL || units[0] == '\0')
    {
        goto done;
    }

    is_valid = CCAPI_TRUE;

done:
    return is_valid;
}

static ccapi_bool_t valid_arg_list(ccapi_dp_argument_t const * const list, size_t const count)
{
    ccapi_bool_t is_valid = CCAPI_TRUE;
    ccapi_bool_t type_found = CCAPI_FALSE;
    ccapi_bool_t timestamp_found = CCAPI_FALSE;
    ccapi_bool_t location_found = CCAPI_FALSE;
    ccapi_bool_t quality_found = CCAPI_FALSE;
    size_t i;

    if (count < 1 || count > 4)
    {
        is_valid = CCAPI_FALSE;
        goto done;
    }

    for (i = 0; i < count; i++)
    {
        switch (list[i])
        {
            case CCAPI_DP_ARG_DATA_INT32:
            case CCAPI_DP_ARG_DATA_INT64:
            case CCAPI_DP_ARG_DATA_FLOAT:
            case CCAPI_DP_ARG_DATA_DOUBLE:
            case CCAPI_DP_ARG_DATA_STRING:
                if (!type_found)
                {
                    ccapi_logging_line("ccapi_data_stream: ambiguous 'type' keyword");
                    type_found = CCAPI_TRUE;
                }
                else
                {
                    is_valid = CCAPI_FALSE;
                    goto done;
                }
                break;
            case CCAPI_DP_ARG_TIME_EPOCH:
            case CCAPI_DP_ARG_TIME_EPOCH_MSEC:
            case CCAPI_DP_ARG_TIME_ISO8601:
                if (!timestamp_found)
                {
                    ccapi_logging_line("ccapi_data_stream: ambiguous 'timestamp' keyword");
                    timestamp_found = CCAPI_TRUE;
                }
                else
                {
                    is_valid = CCAPI_FALSE;
                    goto done;
                }
                break;
            case CCAPI_DP_ARG_LOC:
                if (!location_found)
                {
                    ccapi_logging_line("ccapi_data_stream: ambiguous '" CCAPI_DP_KEY_LOCATION "' order");
                    location_found = CCAPI_TRUE;
                }
                else
                {
                    is_valid = CCAPI_FALSE;
                    goto done;
                }
                break;
            case CCAPI_DP_ARG_QUAL:
                if (!quality_found)
                {
                    ccapi_logging_line("ccapi_data_stream: ambiguous '" CCAPI_DP_KEY_QUALITY "' order");
                    quality_found = CCAPI_TRUE;
                }
                else
                {
                    is_valid = CCAPI_FALSE;
                    goto done;
                }
                break;
            case CCAPI_DP_ARG_INVALID:
                is_valid = CCAPI_FALSE;
                goto done;
        }
    }
done:
    return is_valid;
}

static ccapi_dp_argument_t * arg_list_dup(ccapi_dp_argument_t const * const original_arg_list, size_t const count)
{
    ccapi_dp_argument_t * const arg_list = ccapi_malloc(count * sizeof *arg_list);
    size_t i;

    if (arg_list == NULL)
    {
        goto done;
    }

    for (i = 0; i < count; i++)
    {
        arg_list[i] = original_arg_list[i];
    }

done:
    return arg_list;
}

static char const * get_next_keyword(char * * const next_token, char * const string, char const * const delim)
{
    char * token_start = string != NULL ? string : *next_token;

    if (*token_start == '\0')
    {
        token_start = NULL;
        goto done;
    }

    *next_token = strchr(token_start, delim[0]);
    if (*next_token != NULL)
    {
        **next_token = '\0';
        (*next_token)++;
    }
    else
    {
        *next_token = token_start + strlen(token_start);
    }

done:
    return token_start;
}

static ccapi_dp_error_t get_arg_list_from_format_string(char const * const format_string, ccapi_dp_argument_t * * const arg_list, size_t * const arg_list_count)
{
    ccapi_dp_error_t error = CCAPI_DP_ERROR_NONE;
    char const * keyword;
    char const * const keyword_separator = " ";
    char * format_string_copy;
    char * next_keyword = NULL;
    ccapi_dp_argument_t temp_arg_list[4] = {CCAPI_DP_ARG_INVALID, CCAPI_DP_ARG_INVALID, CCAPI_DP_ARG_INVALID, CCAPI_DP_ARG_INVALID};
    size_t temp_arg_list_count = 0;

    format_string_copy = ccapi_strdup(format_string);
    if (format_string_copy == NULL)
    {
        error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
        goto done;
    }

    keyword = get_next_keyword(&next_keyword, format_string_copy, keyword_separator);
    while (keyword != NULL)
    {
        if (temp_arg_list_count > 3)
        {
            ccapi_logging_line("ccapi_data_stream: too many arguments '%s'", format_string);
            error = CCAPI_DP_ERROR_INVALID_FORMAT;
            goto done;
        }
        if (strcmp(keyword, CCAPI_DP_KEY_DATA_INT32) == 0)
        {
            temp_arg_list[temp_arg_list_count++] = CCAPI_DP_ARG_DATA_INT32;
        }
        else if (strcmp(keyword, CCAPI_DP_KEY_DATA_INT64) == 0)
        {
            temp_arg_list[temp_arg_list_count++] = CCAPI_DP_ARG_DATA_INT64;
        }
        else if (strcmp(keyword, CCAPI_DP_KEY_DATA_FLOAT) == 0)
        {
            temp_arg_list[temp_arg_list_count++] = CCAPI_DP_ARG_DATA_FLOAT;
        }
        else if (strcmp(keyword, CCAPI_DP_KEY_DATA_DOUBLE) == 0)
        {
            temp_arg_list[temp_arg_list_count++] = CCAPI_DP_ARG_DATA_DOUBLE;
        }
        else if (strcmp(keyword, CCAPI_DP_KEY_DATA_STRING) == 0)
        {
            temp_arg_list[temp_arg_list_count++] = CCAPI_DP_ARG_DATA_STRING;
        }
        else if (strcmp(keyword, CCAPI_DP_KEY_TS_EPOCH) == 0)
        {
            temp_arg_list[temp_arg_list_count++] = CCAPI_DP_ARG_TIME_EPOCH;
        }
        else if (strcmp(keyword, CCAPI_DP_KEY_TS_EPOCHMS) == 0)
        {
            temp_arg_list[temp_arg_list_count++] =  CCAPI_DP_ARG_TIME_EPOCH_MSEC;
        }
        else if (strcmp(keyword, CCAPI_DP_KEY_TS_ISO8601) == 0)
        {
            temp_arg_list[temp_arg_list_count++] =  CCAPI_DP_ARG_TIME_ISO8601;
        }
        else if (strcmp(keyword, CCAPI_DP_KEY_LOCATION) == 0)
        {
            temp_arg_list[temp_arg_list_count++] =  CCAPI_DP_ARG_LOC;
        }
        else if (strcmp(keyword, CCAPI_DP_KEY_QUALITY) == 0)
        {
            temp_arg_list[temp_arg_list_count++] =  CCAPI_DP_ARG_QUAL;
        }
        else
        {
            ccapi_logging_line("ccapi_data_stream: invalid keyword '%s'", keyword);
            temp_arg_list[temp_arg_list_count++] = CCAPI_DP_ARG_INVALID;
        }

        keyword = get_next_keyword(&next_keyword, NULL, keyword_separator);
    }

    if (!valid_arg_list(temp_arg_list, temp_arg_list_count))
    {
        ccapi_logging_line("ccapi_data_stream: invalid format string '%s'", format_string);
        error = CCAPI_DP_ERROR_INVALID_FORMAT;
        goto done;
    }

    *arg_list = arg_list_dup(temp_arg_list, temp_arg_list_count);
    if (*arg_list == NULL)
    {
        error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
        goto done;
    }

    *arg_list_count = temp_arg_list_count;


done:
    if (format_string_copy != NULL)
    {
        ccapi_free(format_string_copy);
    }

    return error;
}

static connector_data_point_type_t get_data_stream_type_from_arg_list(ccapi_dp_argument_t * list, size_t count)
{
    connector_data_point_type_t type;
    ccapi_bool_t found = CCAPI_FALSE;
    size_t i;

    for (i = 0; i < count && !found; i++)
    {
        switch (list[i])
        {
            case CCAPI_DP_ARG_DATA_INT32:
                found = CCAPI_TRUE;
                type = connector_data_point_type_integer;
                break;
            case CCAPI_DP_ARG_DATA_INT64:
                found = CCAPI_TRUE;
                type = connector_data_point_type_long;
                break;
            case CCAPI_DP_ARG_DATA_FLOAT:
                found = CCAPI_TRUE;
                type = connector_data_point_type_float;
                break;
            case CCAPI_DP_ARG_DATA_DOUBLE:
                found = CCAPI_TRUE;
                type = connector_data_point_type_double;
                break;
            case CCAPI_DP_ARG_DATA_STRING:
                found = CCAPI_TRUE;
                type = connector_data_point_type_string;
                break;
            case CCAPI_DP_ARG_TIME_EPOCH:
            case CCAPI_DP_ARG_TIME_EPOCH_MSEC:
            case CCAPI_DP_ARG_TIME_ISO8601:
            case CCAPI_DP_ARG_LOC:
            case CCAPI_DP_ARG_QUAL:
            case CCAPI_DP_ARG_INVALID:
                break;
        }
    }
    ASSERT(found == CCAPI_TRUE);

    return type;
}

static void free_ccapi_data_stream(ccapi_dp_data_stream_t * const ccapi_data_stream)
{
    ASSERT(ccapi_data_stream != NULL);
    ASSERT(ccapi_data_stream->ccfsm_data_stream != NULL);
    ccapi_free(ccapi_data_stream->arguments.list);
    ccapi_free(ccapi_data_stream->ccfsm_data_stream->stream_id);

    if (ccapi_data_stream->ccfsm_data_stream->unit != NULL)
    {
        ccapi_free(ccapi_data_stream->ccfsm_data_stream->unit);
    }
    if (ccapi_data_stream->ccfsm_data_stream->forward_to)
    {
        ccapi_free(ccapi_data_stream->ccfsm_data_stream->forward_to);
    }
    ccapi_free(ccapi_data_stream->ccfsm_data_stream);
    ccapi_free(ccapi_data_stream);
}

static void free_ccfsm_data_point(connector_data_point_t * const ccfsm_datapoint)
{
    if (ccfsm_datapoint->data.element.native.string_value != NULL)
    {
        ccapi_free(ccfsm_datapoint->data.element.native.string_value);
    }
    if (ccfsm_datapoint->time.value.iso8601_string != NULL)
    {
        ccapi_free(ccfsm_datapoint->time.value.iso8601_string);
    }

    ccapi_free(ccfsm_datapoint);
}

static ccapi_dp_data_stream_t * find_stream_id_in_collection(ccapi_dp_collection_t * const dp_collection, char const * const stream_id)
{
    ccapi_dp_data_stream_t * current_data_stream = dp_collection->ccapi_data_stream_list;
    ccapi_dp_data_stream_t * data_stream = NULL;

    while (current_data_stream != NULL)
    {
        if (strcmp(stream_id, current_data_stream->ccfsm_data_stream->stream_id) == 0)
        {
            data_stream = current_data_stream;
            goto done;
        }
        current_data_stream = current_data_stream->next;
    }

done:
    return data_stream;
}

ccapi_dp_error_t ccapi_dp_add_data_stream_to_collection_extra(ccapi_dp_collection_t * const dp_collection, char const * const stream_id, char const * const format_string, char const * const units, char const * const forward_to)
{
    ccapi_dp_error_t error = CCAPI_DP_ERROR_NONE;
    ccapi_dp_argument_t * arg_list = NULL;
    size_t arg_count;
    ccapi_dp_data_stream_t * ccapi_stream_info = NULL;
    connector_data_stream_t * ccfsm_stream_info = NULL;
    ccapi_bool_t syncr_acquired = CCAPI_FALSE;
    ccimp_status_t ccimp_status;

    if (dp_collection == NULL)
    {
        error = CCAPI_DP_ERROR_INVALID_ARGUMENT;
        goto done;
    }

    if (!valid_stream_id(stream_id))
    {
        error = CCAPI_DP_ERROR_INVALID_STREAM_ID;
        goto done;
    }

    if (!valid_format_string(format_string))
    {
        error = CCAPI_DP_ERROR_INVALID_FORMAT;
        goto done;
    }

    if (forward_to != NULL && !valid_stream_id(forward_to))
    {
        error = CCAPI_DP_ERROR_INVALID_FORWARD_TO;
        goto done;
    }

    if (units != NULL && !valid_units(units))
    {
        error = CCAPI_DP_ERROR_INVALID_UNITS;
        goto done;
    }

    error = get_arg_list_from_format_string(format_string, &arg_list, &arg_count);
    if (error != CCAPI_DP_ERROR_NONE)
    {
        goto done;
    }

    ccimp_status = ccapi_syncr_acquire(dp_collection->syncr);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            syncr_acquired = CCAPI_TRUE;
            break;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            error = CCAPI_DP_ERROR_SYNCR_FAILED;
            goto done;
    }

    if (find_stream_id_in_collection(dp_collection, stream_id) != NULL)
    {
        error = CCAPI_DP_ERROR_INVALID_STREAM_ID;
        goto done;
    }

    ccapi_stream_info = ccapi_malloc(sizeof *ccapi_stream_info);
    if (ccapi_stream_info == NULL)
    {
        error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
        goto done;
    }

    ccfsm_stream_info = ccapi_malloc(sizeof *ccfsm_stream_info);
    if (ccfsm_stream_info == NULL)
    {
        error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
        goto done;
    }

    ccapi_stream_info->arguments.list = arg_list;
    ccapi_stream_info->arguments.count = arg_count;
    ccapi_stream_info->ccfsm_data_stream = ccfsm_stream_info;

    ccfsm_stream_info->forward_to = NULL;
    ccfsm_stream_info->unit = NULL;

    ccfsm_stream_info->stream_id = ccapi_strdup(stream_id); /* TODO */
    if (ccfsm_stream_info->stream_id == NULL)
    {
        error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
        goto done;
    }

    if (units != NULL)
    {
        ccfsm_stream_info->unit = ccapi_strdup(units);
        if (ccfsm_stream_info->unit == NULL)
        {
            error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
            goto done;
        }
    }

    if (forward_to != NULL)
    {
        ccfsm_stream_info->forward_to = ccapi_strdup(forward_to);
        if (ccfsm_stream_info->forward_to == NULL)
        {
            error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
            goto done;
        }
    }

    ccfsm_stream_info->point = NULL;
    ccfsm_stream_info->type = get_data_stream_type_from_arg_list(arg_list, arg_count);
    ccfsm_stream_info->next = NULL;

    ccapi_stream_info->next = dp_collection->ccapi_data_stream_list;
    dp_collection->ccapi_data_stream_list = ccapi_stream_info;

done:
    if (error != CCAPI_DP_ERROR_NONE)
    {
        if (arg_list != NULL)
        {
            ccapi_free(arg_list);
        }
        if (ccapi_stream_info != NULL)
        {
            ccapi_free(ccapi_stream_info);
        }
        if (ccfsm_stream_info != NULL)
        {
            free_ccfsm_stream(ccfsm_stream_info);
        }
    }

    if (syncr_acquired)
    {
        ccimp_status = ccapi_syncr_release(dp_collection->syncr);
        switch (ccimp_status)
        {
            case CCIMP_STATUS_OK:
                break;
            case CCIMP_STATUS_ERROR:
            case CCIMP_STATUS_BUSY:
                error = CCAPI_DP_ERROR_SYNCR_FAILED;
        }
    }

    return error;
}

ccapi_dp_error_t ccapi_dp_remove_data_stream_from_collection(ccapi_dp_collection_handle_t const dp_collection, char const * const stream_id)
{
    ccapi_dp_error_t error = CCAPI_DP_ERROR_NONE;
    ccapi_bool_t found = CCAPI_FALSE;

    if (dp_collection == NULL || !valid_stream_id(stream_id))
    {
        error = CCAPI_DP_ERROR_INVALID_ARGUMENT;
        goto done;
    }

    if (dp_collection->ccapi_data_stream_list == NULL)
    {
        error = CCAPI_DP_ERROR_INVALID_STREAM_ID;
        goto done;
    }

    {
        ccimp_status_t ccimp_status;

        ccimp_status = ccapi_syncr_acquire(dp_collection->syncr);
        switch (ccimp_status)
        {
            case CCIMP_STATUS_OK:
                break;
            case CCIMP_STATUS_ERROR:
            case CCIMP_STATUS_BUSY:
                error = CCAPI_DP_ERROR_SYNCR_FAILED;
                goto done;
        }
    }

    {
        ccapi_dp_data_stream_t * * const p_start_of_linked_list = &dp_collection->ccapi_data_stream_list;
        ccapi_dp_data_stream_t * current_data_stream = *p_start_of_linked_list;
        ccapi_dp_data_stream_t * previous_data_stream = NULL;

        while (current_data_stream != NULL)
        {
            if (strcmp(current_data_stream->ccfsm_data_stream->stream_id, stream_id) == 0)
            {
                found = CCAPI_TRUE;

                if (previous_data_stream == NULL)
                {
                    *p_start_of_linked_list = NULL;
                }
                else
                {
                    previous_data_stream->next = current_data_stream->next;
                }

                free_data_points_in_data_stream(current_data_stream->ccfsm_data_stream);
                free_ccapi_data_stream(current_data_stream);
            }

            previous_data_stream = current_data_stream;
            current_data_stream = current_data_stream->next;
        }
    }

    {
        ccimp_status_t ccimp_status;

        ccimp_status = ccapi_syncr_release(dp_collection->syncr);
        switch (ccimp_status)
        {
            case CCIMP_STATUS_OK:
                break;
            case CCIMP_STATUS_ERROR:
            case CCIMP_STATUS_BUSY:
                error = CCAPI_DP_ERROR_SYNCR_FAILED;
                goto done;
        }
    }

    if (!found)
    {
        error = CCAPI_DP_ERROR_INVALID_STREAM_ID;
        goto done;
    }

done:
    return error;
}

ccapi_dp_error_t ccapi_dp_add_data_stream_to_collection(ccapi_dp_collection_t * const dp_collection, char const * const stream_id, char const * const format_string)
{
    return ccapi_dp_add_data_stream_to_collection_extra(dp_collection, stream_id, format_string, NULL, NULL);
}
#endif
