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

#include "ccapi_definitions.h"
#include "ccapi/ccapi_transport_udp.h"

#if (defined CCIMP_UDP_TRANSPORT_ENABLED)

static ccapi_bool_t valid_malloc(void * const ptr, ccapi_udp_start_error_t * const error)
{
    if (ptr == NULL)
    {
        *error = CCAPI_UDP_START_ERROR_INSUFFICIENT_MEMORY;
        return CCAPI_FALSE;
    }
    else
    {
        return CCAPI_TRUE;
    }
}

static void copy_ccapi_udp_info_t_structure(ccapi_udp_info_t * const dest, ccapi_udp_info_t const * const source)
{
    *dest = *source; /* Strings and pointers to buffer need to be copied manually to allocated spaces. */

    if (dest->limit.max_sessions == 0)
    {
        dest->limit.max_sessions = CCAPI_SM_UDP_MAX_SESSIONS_DEFAULT;
    }
}

ccapi_udp_start_error_t ccxapi_start_transport_udp(ccapi_data_t * const ccapi_data, ccapi_udp_info_t const * const udp_start)
{
    ccapi_udp_start_error_t error = CCAPI_UDP_START_ERROR_NONE;

    if (!CCAPI_RUNNING(ccapi_data))
    {
        ccapi_logging_line("ccxapi_start_transport_udp: CCAPI not started");

        error = CCAPI_UDP_START_ERROR_CCAPI_STOPPED;
        goto done;
    }

    if (ccapi_data->transport_udp.started)
    {
        error = CCAPI_UDP_START_ERROR_ALREADY_STARTED;
        goto done;
    }

    if (udp_start == NULL)
    {
        ccapi_logging_line("ccxapi_start_transport_udp: invalid argument");
        error = CCAPI_UDP_START_ERROR_NULL_POINTER;
        goto done;
    }

    if (udp_start->limit.max_sessions > CCAPI_SM_UDP_MAX_SESSIONS_LIMIT)
    {
        ccapi_logging_line("ccxapi_start_transport_udp: invalid argument MAX SESSIONS");
        error = CCAPI_UDP_START_ERROR_MAX_SESSIONS;
        goto done;
    }

    ccapi_data->transport_udp.info = ccapi_malloc(sizeof *ccapi_data->transport_udp.info);
    if (!valid_malloc(ccapi_data->transport_udp.info, &error))
    {
        goto done;
    }

    copy_ccapi_udp_info_t_structure(ccapi_data->transport_udp.info, udp_start);

    ccapi_data->transport_udp.started = CCAPI_FALSE;

    {
        connector_transport_t const transport = connector_transport_udp;
        connector_status_t const connector_status = connector_initiate_action_secure(ccapi_data, connector_initiate_transport_start, &transport);

        switch (connector_status)
        {
            case connector_success:
                break;
            case connector_init_error:
            case connector_invalid_data:
            case connector_service_busy:
                error = CCAPI_UDP_START_ERROR_INIT;
                goto done;
            case connector_invalid_data_size:
            case connector_invalid_data_range:
            case connector_keepalive_error:
            case connector_bad_version:
            case connector_device_terminated:
            case connector_invalid_response:
            case connector_no_resource:
            case connector_unavailable:
            case connector_idle:
            case connector_working:
            case connector_pending:
            case connector_active:
            case connector_abort:
            case connector_device_error:
            case connector_exceed_timeout:
            case connector_invalid_payload_packet:
            case connector_open_error:
                error = CCAPI_UDP_START_ERROR_INIT;
                ASSERT_MSG_GOTO(connector_status == connector_success, done);
        }
    }

    {
        ccapi_bool_t const wait_forever = CCAPI_BOOL(udp_start->start_timeout == CCAPI_UDP_START_WAIT_FOREVER);

        if (wait_forever)
        {
            do {
                ccimp_os_yield();
            } while (!ccapi_data->transport_udp.started);
        }
        else
        {
            ccimp_os_system_up_time_t time_start;
            ccimp_os_system_up_time_t end_time;
            unsigned long const jitter = 1;

            ccimp_os_get_system_time(&time_start);
            end_time.sys_uptime = time_start.sys_uptime + udp_start->start_timeout + jitter;
            do {
                ccimp_os_system_up_time_t system_uptime;

                ccimp_os_yield();
                ccimp_os_get_system_time(&system_uptime);
                if (system_uptime.sys_uptime > end_time.sys_uptime)
                {
                    error = CCAPI_UDP_START_ERROR_TIMEOUT;
                    goto done;
                }
            } while (!ccapi_data->transport_udp.started);
        }
    }

done:
    switch (error)
    {
        case CCAPI_UDP_START_ERROR_NONE:
        case CCAPI_UDP_START_ERROR_ALREADY_STARTED:
            break;
        case CCAPI_UDP_START_ERROR_CCAPI_STOPPED:
        case CCAPI_UDP_START_ERROR_NULL_POINTER:
        case CCAPI_UDP_START_ERROR_INIT:
        case CCAPI_UDP_START_ERROR_MAX_SESSIONS:
        case CCAPI_UDP_START_ERROR_INSUFFICIENT_MEMORY:
        case CCAPI_UDP_START_ERROR_TIMEOUT:
            if (ccapi_data != NULL)
            {
                if (ccapi_data->transport_udp.info != NULL)
                {
                    ccapi_free(ccapi_data->transport_udp.info);
                    ccapi_data->transport_udp.info = NULL;
                }
            }
            break;
    }

    return error;
}

ccapi_udp_stop_error_t ccxapi_stop_transport_udp(ccapi_data_t * const ccapi_data, ccapi_udp_stop_t const * const udp_stop)
{
    ccapi_udp_stop_error_t error = CCAPI_UDP_STOP_ERROR_NONE;
    connector_status_t connector_status;

    if (!CCAPI_RUNNING(ccapi_data)|| !ccapi_data->transport_udp.started)
    {
        error = CCAPI_UDP_STOP_ERROR_NOT_STARTED;
        goto done;
    }

    connector_status = ccapi_initiate_transport_stop(ccapi_data, CCAPI_TRANSPORT_UDP, udp_stop->behavior);
    if (connector_status != connector_success)
    {
        error = CCAPI_UDP_STOP_ERROR_CCFSM;
        goto done;
    }

    do {
        ccimp_os_yield();
    } while (ccapi_data->transport_udp.started);

    ASSERT(ccapi_data->transport_udp.info != NULL);

    if (ccapi_data->transport_udp.info != NULL)
    {
        ccapi_free(ccapi_data->transport_udp.info);
        ccapi_data->transport_udp.info = NULL;
    }
done:
    return error;
}

ccapi_udp_start_error_t ccapi_start_transport_udp(ccapi_udp_info_t const * const udp_start)
{
    return ccxapi_start_transport_udp(ccapi_data_single_instance, udp_start);
}

ccapi_udp_stop_error_t ccapi_stop_transport_udp(ccapi_udp_stop_t const * const udp_stop)
{
    return ccxapi_stop_transport_udp(ccapi_data_single_instance, udp_stop);
}
#endif
