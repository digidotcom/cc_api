#include "ccapi_definitions.h"
#include "ccapi/ccapi_transport_sms.h"

#if (defined CCIMP_SMS_TRANSPORT_ENABLED)
static ccapi_bool_t valid_malloc(void * ptr, ccapi_sms_start_error_t * const error)
{
    if (ptr == NULL)
    {
        *error = CCAPI_SMS_START_ERROR_INSUFFICIENT_MEMORY;
        return CCAPI_FALSE;
    }
    else
    {
        return CCAPI_TRUE;
    }
}

static void copy_ccapi_sms_info_t_structure(ccapi_sms_info_t * const dest, ccapi_sms_info_t const * const source)
{

    *dest = *source;

    if (dest->limit.max_sessions == 0)
    {
        dest->limit.max_sessions = CCAPI_SM_SMS_MAX_SESSIONS_DEFAULT;
    }
}


ccapi_sms_start_error_t ccxapi_start_transport_sms(ccapi_data_t * const ccapi_data, ccapi_sms_info_t const * const sms_start)
{
    ccapi_sms_start_error_t error = CCAPI_SMS_START_ERROR_NONE;
    UNUSED_ARGUMENT(sms_start);

    if (!CCAPI_RUNNING(ccapi_data))
    {
        ccapi_logging_line("ccxapi_start_transport_sms: CCAPI not started");

        error = CCAPI_SMS_START_ERROR_CCAPI_STOPPED;
        goto done;
    }

    if (sms_start == NULL)
    {
        ccapi_logging_line("ccxapi_start_transport_sms: invalid argument");
        error = CCAPI_SMS_START_ERROR_NULL_POINTER;
        goto done;
    }

    if (sms_start->limit.max_sessions > CCAPI_SM_SMS_MAX_SESSIONS_LIMIT)
    {
        ccapi_logging_line("ccxapi_start_transport_sms: invalid argument MAX SESSIONS");
        error = CCAPI_SMS_START_ERROR_MAX_SESSIONS;
        goto done;
    }

    ccapi_data->transport_sms.info = ccapi_malloc(sizeof *ccapi_data->transport_sms.info);
    if (!valid_malloc(ccapi_data->transport_sms.info, &error))
    {
        goto done;
    }

    copy_ccapi_sms_info_t_structure(ccapi_data->transport_sms.info, sms_start);

    ccapi_data->transport_sms.started = CCAPI_FALSE;

    {
        connector_transport_t transport = connector_transport_sms;
        connector_status_t const connector_status = connector_initiate_action_secure(ccapi_data, connector_initiate_transport_start, &transport);
        switch (connector_status)
        {
            case connector_success:
                break;
            case connector_init_error:
            case connector_invalid_data:
            case connector_service_busy:
                error = CCAPI_SMS_START_ERROR_INIT;
                break;
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
                error = CCAPI_SMS_START_ERROR_INIT;
                ASSERT_MSG_GOTO(connector_status != connector_success, done);
                break;
        }
    }
    {
        ccapi_bool_t const wait_forever = CCAPI_BOOL(sms_start->timeout == 0);

        if (wait_forever)
        {
            do {
                ccimp_os_yield();
            } while (!ccapi_data->transport_sms.started);
        }
        else
        {
            ccimp_os_system_up_time_t time_start;
            ccimp_os_system_up_time_t end_time;
            unsigned long const jitter = 1;

            ccimp_os_get_system_time(&time_start);
            end_time.sys_uptime = time_start.sys_uptime + sms_start->timeout + jitter;
            do {
                ccimp_os_system_up_time_t system_uptime;

                ccimp_os_yield();
                ccimp_os_get_system_time(&system_uptime);
                if (system_uptime.sys_uptime > end_time.sys_uptime)
                {
                    error = CCAPI_SMS_START_ERROR_TIMEOUT;
                    goto done;
                }
            } while (!ccapi_data->transport_sms.started);
        }
    }
done:
    return error;
}

ccapi_sms_start_error_t ccapi_start_transport_sms(ccapi_sms_info_t const * const sms_start)
{
    return ccxapi_start_transport_sms(ccapi_data_single_instance, sms_start);
}
#endif
