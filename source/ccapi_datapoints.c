#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

#if (defined CCIMP_DATA_SERVICE_ENABLED)

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

ccapi_dp_error_t ccapi_dp_create_data_stream_extra(ccapi_dp_data_stream_t * * const p_stream_info, char const * const stream_id, char const * const format_string, char const * const units, char const * const forward_to)
{
    ccapi_dp_error_t error = CCAPI_DP_ERROR_NONE;
    ccapi_dp_data_stream_t * stream_info = NULL;

    if (p_stream_info == NULL)
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

    {
        ccapi_dp_argument_t * arg_list;
        size_t arg_count;
        error = get_arg_list_from_format_string(format_string, &arg_list, &arg_count);
        if (error != CCAPI_DP_ERROR_NONE)
        {
            goto done;
        }

        stream_info = ccapi_malloc(sizeof *stream_info);
        if (stream_info == NULL)
        {
            ccapi_free(arg_list);
            error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
            goto done;
        }

        stream_info->arguments.count = arg_count;
        stream_info->arguments.list = arg_list;
    }

    stream_info->stream_id = ccapi_strdup(stream_id);
    if (stream_info->stream_id == NULL)
    {
        ccapi_free(stream_info->arguments.list);
        reset_heap_ptr(&stream_info);
        error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
        goto done;
    }

    if (units == NULL)
    {
        stream_info->units = units;
    }
    else
    {
        stream_info->units = ccapi_strdup(units);
        if (stream_info->units == NULL)
        {
            error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
            ccapi_free((void *)stream_info->stream_id);
            ccapi_free(stream_info->arguments.list);
            reset_heap_ptr(&stream_info);
            goto done;
        }
    }

    if (forward_to == NULL)
    {
        stream_info->forward_to = forward_to;
    }
    else
    {
        stream_info->forward_to = ccapi_strdup(forward_to);
        if (stream_info->forward_to == NULL)
        {
            error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
            if (units != NULL)
            {
                ccapi_free((void *)stream_info->units);
            }
            ccapi_free((void *)stream_info->stream_id);
            ccapi_free(stream_info->arguments.list);
            reset_heap_ptr(&stream_info);
            goto done;
        }
    }

done:
    if (p_stream_info != NULL)
    {
        *p_stream_info = stream_info;
    }

    return error;
}

ccapi_dp_error_t ccapi_dp_destroy_data_stream(ccapi_dp_data_stream_t * const stream_info)
{
    ccapi_dp_error_t error = CCAPI_DP_ERROR_NONE;

    if (stream_info == NULL || stream_info->stream_id == NULL || stream_info->arguments.list == NULL)
    {
        error = CCAPI_DP_ERROR_INVALID_ARGUMENT;
        goto done;
    }

    ccapi_free((void*)stream_info->stream_id);
    ccapi_free(stream_info->arguments.list);

    if (stream_info->units != NULL)
    {
        ccapi_free((void*)stream_info->units);
    }

    if (stream_info->forward_to != NULL)
    {
        ccapi_free((void*)stream_info->forward_to);
    }

done:
    return error;
}

ccapi_dp_error_t ccapi_dp_create_data_stream(ccapi_dp_data_stream_t * * const p_stream_info, char const * const stream_id, char const * const format_string)
{
    return ccapi_dp_create_data_stream_extra(p_stream_info, stream_id, format_string, NULL, NULL);
}

#endif
