/*
* Copyright (c) 2014 Etherios, a Division of Digi International, Inc.
* All rights not expressly granted are reserved.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Etherios 11001 Bren Road East, Minnetonka, MN 55343
* =======================================================================
*/

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

        switch(data_stream->type)
        {
            case connector_data_point_type_string:
            {
                ccapi_free(data_point->data.element.native.string_value);
                break;
            }
            case connector_data_point_type_integer:
            case connector_data_point_type_long:
            case connector_data_point_type_float:
            case connector_data_point_type_double:
            case connector_data_point_type_binary:
            case connector_data_point_type_json:
            case connector_data_point_type_geojson:
                break;
        }

        switch(data_point->time.source)
        {
            case connector_time_local_iso8601:
            {
                ccapi_free(data_point->time.value.iso8601_string);
                break;
            }
            case connector_time_cloud:
            case connector_time_local_epoch_fractional:
            case connector_time_local_epoch_whole:
                break;
        }
        ccapi_free(data_point);

        data_point = next_point;
    }
}

static void free_ccfsm_stream(connector_data_stream_t * const ccfsm_stream_info)
{
    if (ccfsm_stream_info != NULL)
    {
        ccapi_free(ccfsm_stream_info->stream_id);
        ccapi_free(ccfsm_stream_info->unit);
        ccapi_free(ccfsm_stream_info->forward_to);
        ccapi_free(ccfsm_stream_info);
    }
}

static void free_collection(ccapi_dp_collection_t * const dp_collection)
{
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

    free_collection(dp_collection);

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

    while (stream_id[i] != '\0') {
        static char const * const valid_special = "_-./[]!+:";

        if (!isalnum(stream_id[i]) && (strchr(valid_special, stream_id[i]) == NULL))
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

static ccapi_bool_t valid_arg_list(ccapi_dp_argument_t const * const list, unsigned int const count)
{
    ccapi_bool_t is_valid = CCAPI_TRUE;
    ccapi_bool_t type_found = CCAPI_FALSE;
    ccapi_bool_t timestamp_found = CCAPI_FALSE;
    ccapi_bool_t location_found = CCAPI_FALSE;
    ccapi_bool_t quality_found = CCAPI_FALSE;
    unsigned int i;

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
                if (type_found)
                {
                    ccapi_logging_line("ccapi_data_stream: ambiguous 'type' keyword");
                    is_valid = CCAPI_FALSE;
                    goto done;
                }
                else
                {
                    type_found = CCAPI_TRUE;
                }
                break;
            case CCAPI_DP_ARG_TIME_EPOCH:
            case CCAPI_DP_ARG_TIME_EPOCH_MSEC:
            case CCAPI_DP_ARG_TIME_ISO8601:
                if (timestamp_found)
                {
                    ccapi_logging_line("ccapi_data_stream: ambiguous 'timestamp' keyword");
                    is_valid = CCAPI_FALSE;
                    goto done;
                }
                else
                {
                    timestamp_found = CCAPI_TRUE;
                }
                break;
            case CCAPI_DP_ARG_LOC:
                if (location_found)
                {
                    ccapi_logging_line("ccapi_data_stream: ambiguous '" CCAPI_DP_KEY_LOCATION "' order");
                    is_valid = CCAPI_FALSE;
                    goto done;
                }
                else
                {
                    location_found = CCAPI_TRUE;
                }
                break;
            case CCAPI_DP_ARG_QUAL:
                if (quality_found)
                {
                    ccapi_logging_line("ccapi_data_stream: ambiguous '" CCAPI_DP_KEY_QUALITY "' order");
                    is_valid = CCAPI_FALSE;
                    goto done;
                }
                else
                {
                    quality_found = CCAPI_TRUE;
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

static ccapi_dp_argument_t * arg_list_dup(ccapi_dp_argument_t const * const original_arg_list, unsigned int const count)
{
    ccapi_dp_argument_t * const arg_list = ccapi_malloc(count * sizeof *arg_list);
    unsigned int i;

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

static ccapi_dp_error_t get_arg_list_from_format_string(char const * const format_string, ccapi_dp_argument_t * * const arg_list, unsigned int * const arg_list_count)
{
    static char const * const keyword_separator = " ";
    char const * keyword;
    char * format_string_copy;
    char * next_keyword = NULL;
    ccapi_dp_argument_t temp_arg_list[4] = {CCAPI_DP_ARG_INVALID, CCAPI_DP_ARG_INVALID, CCAPI_DP_ARG_INVALID, CCAPI_DP_ARG_INVALID};
    unsigned int temp_arg_list_count = 0;
    ccapi_dp_error_t error = CCAPI_DP_ERROR_NONE;

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
    ccapi_free(format_string_copy);

    return error;
}

static connector_data_point_type_t get_data_stream_type_from_arg_list(ccapi_dp_argument_t const * const list, unsigned int const count)
{
    connector_data_point_type_t type;
    ccapi_bool_t found = CCAPI_FALSE;
    unsigned int i;

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
    ccapi_free(ccapi_data_stream->ccfsm_data_stream->unit);
    ccapi_free(ccapi_data_stream->ccfsm_data_stream->forward_to);
    ccapi_free(ccapi_data_stream->ccfsm_data_stream);
    ccapi_free(ccapi_data_stream);
}

static void free_ccfsm_data_point(connector_data_point_t * const ccfsm_datapoint)
{
    ccapi_free(ccfsm_datapoint->data.element.native.string_value);
    ccapi_free(ccfsm_datapoint->time.value.iso8601_string);
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
    unsigned int arg_count;
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
        ccapi_free(arg_list);
        ccapi_free(ccapi_stream_info);
        free_ccfsm_stream(ccfsm_stream_info);
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

static ccapi_dp_error_t parse_argument_list_and_create_data_point(ccapi_dp_data_stream_t * const data_stream, va_list arg_list, connector_data_point_t * * const new_data_point)
{
    ccapi_dp_argument_t * const arg = data_stream->arguments.list;
    unsigned int const arg_count =  data_stream->arguments.count;
    connector_data_point_t * const ccfsm_datapoint = ccapi_malloc(sizeof *ccfsm_datapoint);
    ccapi_dp_error_t error = CCAPI_DP_ERROR_NONE;
    unsigned int i;

    if (ccfsm_datapoint == NULL)
    {
        error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
        goto done;
    }

    ccfsm_datapoint->data.type = connector_data_type_native;
    ccfsm_datapoint->quality.type = connector_quality_type_ignore;
    ccfsm_datapoint->location.type = connector_location_type_ignore;
    ccfsm_datapoint->time.source = connector_time_cloud;

    ccfsm_datapoint->description = NULL;
    ccfsm_datapoint->data.element.native.string_value = NULL;
    ccfsm_datapoint->time.value.iso8601_string = NULL;

    for (i = 0; i < arg_count; i++)
    {
        switch(arg[i])
        {
            case CCAPI_DP_ARG_DATA_INT32:
            {
                ccfsm_datapoint->data.element.native.int_value = va_arg(arg_list, int32_t);
                break;
            }

            case CCAPI_DP_ARG_DATA_INT64:
            {
                ccfsm_datapoint->data.element.native.long_value = va_arg(arg_list, int64_t);
                break;
            }

            case CCAPI_DP_ARG_DATA_FLOAT:
            {
                double const aux = va_arg(arg_list, double); /* ‘float’ is promoted to ‘double’ when passed through ‘...’ */

                ccfsm_datapoint->data.element.native.float_value = (float)aux;
                break;
            }

            case CCAPI_DP_ARG_DATA_DOUBLE:
            {
                ccfsm_datapoint->data.element.native.double_value = va_arg(arg_list, double);
                break;
            }

            case CCAPI_DP_ARG_DATA_STRING:
            {
                char const * const string_dp = va_arg(arg_list, char const * const);

                ccfsm_datapoint->data.element.native.string_value = ccapi_strdup(string_dp);

                if (ccfsm_datapoint->data.element.native.string_value == NULL)
                {
                    free_ccfsm_data_point(ccfsm_datapoint);
                    error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
                    goto done;
                }
                break;
            }

            case CCAPI_DP_ARG_TIME_EPOCH:
            {
                ccapi_timestamp_t const * const timestamp = va_arg(arg_list, ccapi_timestamp_t *);

                ccfsm_datapoint->time.source = connector_time_local_epoch_fractional;
                ccfsm_datapoint->time.value.since_epoch_fractional.seconds = timestamp->epoch.seconds;
                ccfsm_datapoint->time.value.since_epoch_fractional.milliseconds = timestamp->epoch.milliseconds;
                break;
            }

            case CCAPI_DP_ARG_TIME_EPOCH_MSEC:
            {
                ccapi_timestamp_t const * const timestamp = va_arg(arg_list, ccapi_timestamp_t *);

                ccfsm_datapoint->time.source = connector_time_local_epoch_whole;
                ccfsm_datapoint->time.value.since_epoch_whole.milliseconds = timestamp->epoch_msec;
                break;
            }

            case CCAPI_DP_ARG_TIME_ISO8601:
            {
                ccapi_timestamp_t const * const timestamp = va_arg(arg_list, ccapi_timestamp_t *);

                ccfsm_datapoint->time.source = connector_time_local_iso8601;
                ccfsm_datapoint->time.value.iso8601_string = ccapi_strdup(timestamp->iso8601);

                if (ccfsm_datapoint->time.value.iso8601_string == NULL)
                {
                    free_ccfsm_data_point(ccfsm_datapoint);
                    error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
                    goto done;
                }
                break;
            }
            case CCAPI_DP_ARG_LOC:
            {
                ccapi_location_t const * const location = va_arg(arg_list, ccapi_location_t *);

                ccfsm_datapoint->location.type = connector_location_type_native;
                ccfsm_datapoint->location.value.native.latitude = location->latitude;
                ccfsm_datapoint->location.value.native.longitude = location->longitude;
                ccfsm_datapoint->location.value.native.elevation = location->elevation;
                break;
            }
            case CCAPI_DP_ARG_QUAL:
            {
                ccfsm_datapoint->quality.type = connector_quality_type_native;
                ccfsm_datapoint->quality.value = (int)va_arg(arg_list, int32_t);
                break;
            }
            case CCAPI_DP_ARG_INVALID:
            {
                ASSERT_MSG_GOTO(arg[i] != CCAPI_DP_ARG_INVALID, done);
                break;
            }
        }
    }

done:
    *new_data_point = ccfsm_datapoint;
    return error;
}

ccapi_dp_error_t ccapi_dp_add(ccapi_dp_collection_t * const dp_collection, char const * const stream_id, ...)
{
    ccapi_dp_error_t error = CCAPI_DP_ERROR_NONE;
    ccapi_dp_data_stream_t * data_stream;

    if (dp_collection == NULL)
    {
        error = CCAPI_DP_ERROR_INVALID_ARGUMENT;
        goto done;
    }


    switch (ccapi_syncr_acquire(dp_collection->syncr))
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            error = CCAPI_DP_ERROR_SYNCR_FAILED;
            goto error;
    }

    if (!valid_stream_id(stream_id))
    {
        error = CCAPI_DP_ERROR_INVALID_STREAM_ID;
        goto done;
    }

    data_stream = find_stream_id_in_collection(dp_collection, stream_id);

    if (data_stream == NULL)
    {
        error = CCAPI_DP_ERROR_INVALID_STREAM_ID;
        goto done;
    }

    {
        connector_data_point_t * ccfsm_datapoint = NULL;
        va_list arg_list;

        va_start(arg_list, stream_id);
        error = parse_argument_list_and_create_data_point(data_stream, arg_list, &ccfsm_datapoint);
        va_end(arg_list);

        if (error != CCAPI_DP_ERROR_NONE)
        {
            goto done;
        }

        ASSERT(ccfsm_datapoint != NULL);
        ccfsm_datapoint->next = data_stream->ccfsm_data_stream->point;
        data_stream->ccfsm_data_stream->point = ccfsm_datapoint;
    }

done:
    switch (ccapi_syncr_release(dp_collection->syncr))
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            error = CCAPI_DP_ERROR_SYNCR_FAILED;
            goto error;
    }

error:
    return error;
}

static void chain_collection_ccfsm_data_streams(ccapi_dp_collection_t * const dp_collection)
{
    ccapi_dp_data_stream_t * current_data_stream = dp_collection->ccapi_data_stream_list;

    while (current_data_stream != NULL)
    {
        connector_data_stream_t * const ccfsm_data_stream = current_data_stream->ccfsm_data_stream;
        ccapi_dp_data_stream_t * const next_data_stream = current_data_stream->next;

        if (next_data_stream != NULL)
        {
            ccfsm_data_stream->next = next_data_stream->ccfsm_data_stream;
        }

        current_data_stream = current_data_stream->next;
    }
}

static void free_data_points_from_collection(ccapi_dp_collection_t * const dp_collection)
{
    ccapi_dp_data_stream_t * current_data_stream = dp_collection->ccapi_data_stream_list;

    while (current_data_stream != NULL)
    {
        connector_data_stream_t * const ccfsm_data_stream = current_data_stream->ccfsm_data_stream;
        ccapi_dp_data_stream_t const * const next_data_stream = current_data_stream->next;

        if (next_data_stream != NULL)
        {
            ccfsm_data_stream->next = next_data_stream->ccfsm_data_stream;
        }

        free_data_points_in_data_stream(ccfsm_data_stream);
        ccfsm_data_stream->point = NULL;
        current_data_stream = current_data_stream->next;
    }
}

static ccapi_dp_error_t send_collection(ccapi_data_t * const ccapi_data, ccapi_transport_t const transport, ccapi_dp_collection_t * const dp_collection, ccapi_bool_t const with_reply, unsigned long const timeout, ccapi_string_info_t * const hint)
{
    ccapi_dp_error_t error = CCAPI_DP_ERROR_NONE;
    ccapi_bool_t * p_transport_started = NULL;
    ccapi_dp_transaction_info_t * transaction_info = NULL;
    ccapi_bool_t collection_lock_acquired = CCAPI_FALSE;

    if (dp_collection == NULL || dp_collection->ccapi_data_stream_list == NULL)
    {
        error = CCAPI_DP_ERROR_INVALID_ARGUMENT;
        goto done;
    }

    if (!CCAPI_RUNNING(ccapi_data))
    {
        ccapi_logging_line("ccapi_dp_send_collection: CCAPI not started");
        error = CCAPI_DP_ERROR_CCAPI_NOT_RUNNING;
        goto done;
    }

    switch (transport)
    {
        case CCAPI_TRANSPORT_TCP:
            p_transport_started = &ccapi_data->transport_tcp.connected;
            break;
        case CCAPI_TRANSPORT_UDP:
#if (defined CCIMP_UDP_TRANSPORT_ENABLED)
            p_transport_started = &ccapi_data->transport_udp.started;
#endif
            break;
        case CCAPI_TRANSPORT_SMS:
#if (defined CCIMP_SMS_TRANSPORT_ENABLED)
            p_transport_started = &ccapi_data->transport_sms.started;
#endif
            break;
    }

    if (p_transport_started == NULL || !*p_transport_started)
    {
        ccapi_logging_line("ccapi_dp_send_collection: Transport not started");
        error = CCAPI_DP_ERROR_TRANSPORT_NOT_STARTED;
        goto done;
    }

    {
        connector_request_data_point_t ccfsm_request;
        connector_status_t ccfsm_status;

        transaction_info = ccapi_malloc(sizeof *transaction_info);
        if (transaction_info == NULL)
        {
            error = CCAPI_DP_ERROR_INSUFFICIENT_MEMORY;
            goto done;
        }

        transaction_info->hint = hint;
        transaction_info->syncr =  ccapi_syncr_create();
        if (transaction_info->syncr == NULL)
        {
            error = CCAPI_DP_ERROR_SYNCR_FAILED;
            goto done;
        }

        switch (ccapi_syncr_acquire(dp_collection->syncr))
        {
            case CCIMP_STATUS_OK:
                collection_lock_acquired = CCAPI_TRUE;
                break;
            case CCIMP_STATUS_ERROR:
            case CCIMP_STATUS_BUSY:
                error = CCAPI_DP_ERROR_SYNCR_FAILED;
                goto done;
        }

        chain_collection_ccfsm_data_streams(dp_collection);

        ccfsm_request.request_id = NULL;
        ccfsm_request.response_required = CCAPI_BOOL_TO_CONNECTOR_BOOL(with_reply);
        ccfsm_request.timeout_in_seconds = timeout;
        ccfsm_request.transport = ccapi_to_connector_transport(transport);
        ccfsm_request.user_context = transaction_info;
        ccfsm_request.stream = dp_collection->ccapi_data_stream_list->ccfsm_data_stream;

        ccfsm_status = connector_initiate_action_secure(ccapi_data, connector_initiate_data_point, &ccfsm_request);
        if (ccfsm_status != connector_success)
        {
            ccapi_logging_line("ccapi_dp_send_collection: failure when calling connector_initiate_action, error %d", ccfsm_status);
            error = CCAPI_DP_ERROR_INITIATE_ACTION_FAILED;
            goto done;
        }

        {
            ccimp_status_t const ccimp_status = ccapi_syncr_acquire(transaction_info->syncr);

            switch (ccimp_status)
            {
                case CCIMP_STATUS_OK:
                    break;
                case CCIMP_STATUS_BUSY:
                case CCIMP_STATUS_ERROR:
                    ccapi_logging_line("ccapi_dp_send_collection: lock_acquire failed");
                    error = CCAPI_DP_ERROR_SYNCR_FAILED;
                    goto done;
            }

            if (transaction_info->response_error != CCAPI_DP_ERROR_NONE)
            {
                error = transaction_info->response_error;
                goto done;
            }

            if (transaction_info->status != CCAPI_DP_ERROR_NONE)
            {
                error = transaction_info->status;
                goto done;
            }
        }

        free_data_points_from_collection(dp_collection);
    }

done:
    ccapi_free(transaction_info);

    if (collection_lock_acquired)
    {
        switch (ccapi_syncr_release(dp_collection->syncr))
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

ccapi_dp_error_t ccxapi_dp_send_collection(ccapi_data_t * const ccapi_data, ccapi_transport_t const transport, ccapi_dp_collection_t * const dp_collection)
{
    return send_collection(ccapi_data, transport, dp_collection, CCAPI_FALSE, OS_SYNCR_ACQUIRE_INFINITE, NULL);
}

ccapi_dp_error_t ccxapi_dp_send_collection_with_reply(ccapi_data_t * const ccapi_data, ccapi_transport_t const transport, ccapi_dp_collection_t * const dp_collection, unsigned long const timeout, ccapi_string_info_t * const hint)
{
    return send_collection(ccapi_data, transport, dp_collection, CCAPI_TRUE, timeout, hint);
}

ccapi_dp_error_t ccapi_dp_send_collection(ccapi_transport_t const transport, ccapi_dp_collection_t * const dp_collection)
{
    return ccxapi_dp_send_collection(ccapi_data_single_instance, transport, dp_collection);
}

ccapi_dp_error_t ccapi_dp_send_collection_with_reply(ccapi_transport_t const transport, ccapi_dp_collection_t * const dp_collection, unsigned long const timeout, ccapi_string_info_t * const hint)
{
    return ccxapi_dp_send_collection_with_reply(ccapi_data_single_instance, transport, dp_collection , timeout, hint);
}
#endif
