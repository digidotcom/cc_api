/*
 * ccapi.c
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

ccapi_data_t * ccapi_data_single_instance = NULL;

void * ccapi_malloc(size_t size)
{
    ccimp_os_malloc_t malloc_info;
    ccimp_status_t status;

    malloc_info.size = size;
    status = ccimp_os_malloc(&malloc_info);

    switch (status)
    {
        case CCIMP_STATUS_OK:
            return malloc_info.ptr;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            break;
    }
    return NULL;
}

ccimp_status_t ccapi_free(void * ptr)
{
    ccimp_os_free_t free_info;

    free_info.ptr = ptr;

    return ccimp_os_free(&free_info);
}

ccimp_status_t ccapi_syncr_release(void * syncr_object)
{
    ccimp_os_syncr_release_t release_data;

    release_data.syncr_object = syncr_object;
        
    return ccimp_os_syncr_release(&release_data);
}

void ccapi_connector_run_thread(void * const argument)
{
    ccapi_data_t * ccapi_data = argument;

    /* ccapi_data is corrupted, it's likely the implementer made it wrong passing argument to the new thread */
    ASSERT_MSG_GOTO(ccapi_data != NULL, done);

    ccapi_data->thread.connector_run->status = CCAPI_THREAD_RUNNING;
    while (ccapi_data->thread.connector_run->status == CCAPI_THREAD_RUNNING)
    {
        connector_status_t const status = connector_run(ccapi_data->connector_handle);

        ASSERT_MSG_GOTO(status != connector_init_error, done);

        switch(status)
        {
            case connector_device_terminated:
                ccapi_data->thread.connector_run->status = CCAPI_THREAD_REQUEST_STOP;
                break;
            default:
                break;
        }
    }
    ASSERT_MSG_GOTO(ccapi_data->thread.connector_run->status == CCAPI_THREAD_REQUEST_STOP, done);

    ccapi_data->thread.connector_run->status = CCAPI_THREAD_NOT_STARTED;
done:
    return;
}

static connector_callback_status_t connector_callback_status_from_ccimp_status(ccimp_status_t const ccimp_status)
{
    connector_callback_status_t callback_status = connector_callback_abort;

    switch(ccimp_status)
    {
        case CCIMP_STATUS_ERROR:
            callback_status = connector_callback_abort;
            break;
        case CCIMP_STATUS_OK:
            callback_status = connector_callback_continue;
            break;
        case CCIMP_STATUS_BUSY:
            callback_status = connector_callback_busy;
            break;
    }

    return callback_status;
}

connector_callback_status_t ccapi_config_handler(connector_request_id_config_t config_request, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t status = connector_callback_continue;

    ccapi_logging_line(TMP_INFO_PREFIX "ccapi_config_handler: config_request %d", config_request);

    switch (config_request)
    {
        case connector_request_id_config_device_id:
            {
                connector_config_pointer_data_t * device_id = data;
                device_id->data = ccapi_data->config.device_id;
            }
            break;
        case connector_request_id_config_device_cloud_url:
            {
                connector_config_pointer_string_t * device_cloud = data;
                device_cloud->string = ccapi_data->config.device_cloud_url;
                device_cloud->length = strlen(ccapi_data->config.device_cloud_url);
            }
            break;
        case connector_request_id_config_vendor_id:
            {
                connector_config_vendor_id_t * vendor_id = data;
                vendor_id->id = ccapi_data->config.vendor_id;
            }
            break;
        case connector_request_id_config_device_type:
            {
                connector_config_pointer_string_t * device_type = data;
                device_type->string = ccapi_data->config.device_type;
                device_type->length = strlen(ccapi_data->config.device_type);
            }
            break;
        case connector_request_id_config_firmware_facility:
            {
                connector_config_supported_t * firmware_supported = data;
                firmware_supported->supported = ccapi_data->config.firmware_supported;
            }
            break;
        case connector_request_id_config_file_system:
            {
                connector_config_supported_t * filesystem_supported = data;
                filesystem_supported->supported = ccapi_data->config.filesystem_supported;
            }
            break;
        case connector_request_id_config_remote_configuration:
            {
                connector_config_supported_t * rci_supported = data;
                rci_supported->supported = ccapi_data->config.rci_supported;
            }
            break;
        case connector_request_id_config_data_service:
            {
                connector_config_supported_t * data_service_supported = data;
                data_service_supported->supported = connector_true;
            }
            break;
        case connector_request_id_config_connection_type:
            {
                connector_config_connection_type_t * connection_type = data;
                connection_type->type = ccapi_data->transport_tcp.info->connection.type;
            }
            break;
        case connector_request_id_config_mac_addr:
            {
                connector_config_pointer_data_t * mac_addr = data;
                mac_addr->data = ccapi_data->transport_tcp.info->connection.info.lan.mac_address;
            }
            break;
        case connector_request_id_config_ip_addr:
            {
                connector_config_ip_address_t * ip_addr = data;
                switch(ccapi_data->transport_tcp.info->connection.ip.type)
                {
                    case CCAPI_IPV4:
                    {
                        ip_addr->ip_address_type = connector_ip_address_ipv4;
                        ip_addr->address = ccapi_data->transport_tcp.info->connection.ip.address.ipv4;
                        break;
                    }
                    case CCAPI_IPV6:
                    {
                        ip_addr->ip_address_type = connector_ip_address_ipv6;
                        ip_addr->address = ccapi_data->transport_tcp.info->connection.ip.address.ipv6;
                        break;
                    }
                }
            }
            break;
        case connector_request_id_config_identity_verification:
            {
                connector_config_identity_verification_t * id_verification = data;
                int const has_password = ccapi_data->transport_tcp.info->connection.password != NULL;

                id_verification->type = has_password ? connector_identity_verification_password : connector_identity_verification_simple;
            }
            break;
        case connector_request_id_config_password:
            {
                connector_config_pointer_string_t * password = data;
                password->string = ccapi_data->transport_tcp.info->connection.password;
                password->length = strlen(password->string);
            }
            break;
        case connector_request_id_config_max_transaction:
            {
                connector_config_max_transaction_t * max_transaction = data;
                max_transaction->count = ccapi_data->transport_tcp.info->connection.max_transactions;
            }
            break;
        case connector_request_id_config_rx_keepalive:
            {
                connector_config_keepalive_t * rx_keepalives = data;
                rx_keepalives->interval_in_seconds = ccapi_data->transport_tcp.info->keepalives.rx;
            }
            break;
        case connector_request_id_config_tx_keepalive:
            {
                connector_config_keepalive_t * tx_keepalives = data;
                tx_keepalives->interval_in_seconds = ccapi_data->transport_tcp.info->keepalives.tx;
            }
            break;
        case connector_request_id_config_wait_count:
            {
                connector_config_wait_count_t * wc_keepalives = data;
                wc_keepalives->count = ccapi_data->transport_tcp.info->keepalives.wait_count;
            }
            break;
        case connector_request_id_config_link_speed:
            {
                connector_config_link_speed_t * link_speed = data;
                link_speed->speed = ccapi_data->transport_tcp.info->connection.info.wan.link_speed;
            }
            break;
        case connector_request_id_config_phone_number:
            {
                connector_config_pointer_string_t * phone_number = data;
                phone_number->string = ccapi_data->transport_tcp.info->connection.info.wan.phone_number;
                phone_number->length = strlen(ccapi_data->transport_tcp.info->connection.info.wan.phone_number);
            }
            break;
        default:
            status = connector_callback_unrecognized;
            ASSERT_MSG_GOTO(0, done);
            break;
    }
done:
    return status;
}

connector_callback_status_t ccapi_os_handler(connector_request_id_os_t os_request, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status;
    ccimp_status_t ccimp_status = CCIMP_STATUS_ERROR;

    UNUSED_ARGUMENT(ccapi_data);
    switch (os_request) {
        case connector_request_id_os_malloc:
        {
            connector_os_malloc_t * connector_malloc_data = data;
            ccimp_os_malloc_t ccimp_malloc_data;

            ccimp_malloc_data.size = connector_malloc_data->size;
            ccimp_malloc_data.ptr = connector_malloc_data->ptr;

            ccimp_status = ccimp_os_malloc(&ccimp_malloc_data);

            connector_malloc_data->ptr = ccimp_malloc_data.ptr;
            break;
        }

        case connector_request_id_os_free:
        {
            connector_os_free_t * connector_free_data = data;
            ccimp_os_free_t ccimp_free_data;

            ccimp_free_data.ptr = connector_free_data->ptr;
            ccimp_status = ccimp_os_free(&ccimp_free_data);
            break;
        }

        case connector_request_id_os_yield:
        {
            ccimp_status = ccimp_os_yield();
            break;
        }

        case connector_request_id_os_system_up_time:
        {
            connector_os_system_up_time_t * connector_system_uptime = data;
            ccimp_os_system_up_time_t ccimp_system_uptime;

            ccimp_status = ccimp_os_get_system_time(&ccimp_system_uptime);

            connector_system_uptime->sys_uptime = ccimp_system_uptime.sys_uptime;
            break;
        }

        case connector_request_id_os_reboot:
        {
            ccimp_status = ccimp_hal_reset();
            break;
        }

        case connector_request_id_os_realloc:
        {
            connector_os_realloc_t * connector_realloc_data = data;
            ccimp_os_realloc_t ccimp_realloc_data;

            ccimp_realloc_data.new_size = connector_realloc_data->new_size;
            ccimp_realloc_data.old_size = connector_realloc_data->old_size;
            ccimp_realloc_data.ptr = connector_realloc_data->ptr;
            ccimp_status = ccimp_os_realloc(&ccimp_realloc_data);

            connector_realloc_data->ptr = ccimp_realloc_data.ptr;
            break;
        }
    }

    connector_status = connector_callback_status_from_ccimp_status(ccimp_status);

    return connector_status;
}

static ccapi_bool_t ask_user_if_reconnect(connector_close_status_t const close_status, ccapi_data_t const * const ccapi_data)
{
    ccapi_tcp_close_cb_t const close_cb = ccapi_data->transport_tcp.info->callback.close;
    ccapi_tcp_close_cause_t ccapi_close_cause;
    ccapi_bool_t reconnect = CCAPI_FALSE;

    if (close_cb != NULL)
    {
        switch (close_status)
        {
            case connector_close_status_cloud_disconnected:
                ccapi_close_cause = CCAPI_TCP_CLOSE_DISCONNECTED;
                break;
            case connector_close_status_cloud_redirected:
                ccapi_close_cause = CCAPI_TCP_CLOSE_REDIRECTED;
                break;
            case connector_close_status_device_error:
                ccapi_close_cause = CCAPI_TCP_CLOSE_DATA_ERROR;
                break;
            case connector_close_status_no_keepalive:
                ccapi_close_cause = CCAPI_TCP_CLOSE_NO_KEEPALIVE;
                break;
            case connector_close_status_device_stopped:
            case connector_close_status_device_terminated:
            case connector_close_status_abort:
                ASSERT_MSG_GOTO(0, done);
                break;
        }
        reconnect = close_cb(ccapi_close_cause);
    }
done:
    return reconnect;
}

connector_callback_status_t ccapi_network_handler(connector_request_id_network_t network_request, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status;
    ccimp_status_t ccimp_status = CCIMP_STATUS_ERROR;

    UNUSED_ARGUMENT(ccapi_data);
    switch (network_request)
    {
        case connector_request_id_network_open:
        {
            connector_network_open_t * connector_open_data = data;
            ccimp_network_open_t ccimp_open_data;

            ccimp_open_data.device_cloud.url = connector_open_data->device_cloud.url;
            ccimp_open_data.handle = connector_open_data->handle;

            ccimp_status = ccimp_network_tcp_open(&ccimp_open_data);

            connector_open_data->handle = ccimp_open_data.handle;
            break;
        }

        case connector_request_id_network_send:
        {
            connector_network_send_t * connector_send_data = data;
            ccimp_network_send_t ccimp_send_data;

            ccimp_send_data.buffer = connector_send_data->buffer;
            ccimp_send_data.bytes_available = connector_send_data->bytes_available;
            ccimp_send_data.handle = connector_send_data->handle;
            ccimp_send_data.bytes_used = 0;

            ccimp_status = ccimp_network_tcp_send(&ccimp_send_data);

            connector_send_data->bytes_used = ccimp_send_data.bytes_used;
            break;
        }

        case connector_request_id_network_receive:
        {
            connector_network_receive_t * connector_receive_data = data;
            ccimp_network_receive_t ccimp_receive_data;

            ccimp_receive_data.buffer = connector_receive_data->buffer;
            ccimp_receive_data.bytes_available = connector_receive_data->bytes_available;
            ccimp_receive_data.handle = connector_receive_data->handle;
            ccimp_receive_data.bytes_used = 0;

            ccimp_status = ccimp_network_tcp_receive(&ccimp_receive_data);

            connector_receive_data->bytes_used = ccimp_receive_data.bytes_used;
            break;
        }

        case connector_request_id_network_close:
        {
            connector_network_close_t * connector_close_data = data;
            ccimp_network_close_t close_data;
            connector_close_status_t const close_status = connector_close_data->status;

            close_data.handle = connector_close_data->handle;
            ccimp_status = ccimp_network_tcp_close(&close_data);

            switch(ccimp_status)
            {
                case CCIMP_STATUS_OK:
                    ccapi_data->transport_tcp.connected = CCAPI_FALSE;
                    break;
                case CCIMP_STATUS_ERROR:
                case CCIMP_STATUS_BUSY:
                    goto done;
                    break;
            }

            switch (close_status)
            {
                case connector_close_status_cloud_disconnected:
                case connector_close_status_cloud_redirected:
                case connector_close_status_device_error:
                case connector_close_status_no_keepalive:
                {
                    connector_close_data->reconnect = ask_user_if_reconnect(close_status, ccapi_data);
                    break;
                }
                case connector_close_status_device_stopped:
                case connector_close_status_device_terminated:
                case connector_close_status_abort:
                {
                    connector_close_data->reconnect = connector_false;
                    break;
                }
            }
            break;
        }

        default:
        {
            ccimp_status = CCIMP_STATUS_ERROR;
            break;
        }
    }

done:

    connector_status = connector_callback_status_from_ccimp_status(ccimp_status);

    return connector_status;
}

connector_callback_status_t ccapi_status_handler(connector_request_id_status_t status_request, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status = connector_callback_continue;

    switch (status_request)
    {
        case connector_request_id_status_tcp:
        {
            connector_status_tcp_event_t * tcp_event = data;

            switch(tcp_event->status)
            {
                case connector_tcp_communication_started:
                {
                    ccapi_data->transport_tcp.connected = CCAPI_TRUE;
                    break;
                }
                case connector_tcp_keepalive_missed:
                {
                    ccapi_keepalive_status_t const keepalive_status = CCAPI_KEEPALIVE_MISSED;

                    if (ccapi_data->transport_tcp.info->callback.keepalive != NULL)
                    {
                        ccapi_data->transport_tcp.info->callback.keepalive(keepalive_status);
                    }
                    break;
                }
                case connector_tcp_keepalive_restored:
                {
                    ccapi_keepalive_status_t const keepalive_status = CCAPI_KEEPALIVE_RESTORED;

                    if (ccapi_data->transport_tcp.info->callback.keepalive != NULL)
                    {
                        ccapi_data->transport_tcp.info->callback.keepalive(keepalive_status);
                    }
                    break;
                }
            }
            break;
        }
        case connector_request_id_status_stop_completed:
        {
            connector_initiate_stop_request_t * stop_request = data;

            switch (stop_request->transport)
            {
                case connector_transport_tcp:
                case connector_transport_all:
                {
                    ccapi_data->transport_tcp.connected = CCAPI_FALSE;
                    break;
                }
#if defined CONNECTOR_TRANSPORT_UDP
                case connector_transport_udp:
                    /* TODO */
                    break;
#endif
#if defined CONNECTOR_TRANSPORT_SMS
                case connector_transport_sms:
                    /* TODO */
                    break;
#endif
            }
            break;
        }
    }

    return connector_status;
}


static ccimp_fs_seek_origin_t ccimp_seek_origin_from_ccfsm_seek_origin(connector_file_system_seek_origin_t ccfsm_origin)
{
    ccimp_fs_seek_origin_t ccimp_fs_seek = CCIMP_SEEK_CUR;

    switch (ccfsm_origin)
    {
        case connector_file_system_seek_cur:
            ccimp_fs_seek = CCIMP_SEEK_CUR;
            break;
        case connector_file_system_seek_end:
            ccimp_fs_seek = CCIMP_SEEK_END;
            break;
        case connector_file_system_seek_set:
            ccimp_fs_seek =  CCIMP_SEEK_SET;
            break;
    }

    return ccimp_fs_seek;
}

static connector_file_system_file_type_t ccfsm_file_system_file_type_from_ccimp_fs_dir_entry_type(ccimp_fs_dir_entry_type_t ccimp_entry_type)
{
    connector_file_system_file_type_t ccfsm_file_type = connector_file_system_file_type_none;

    switch (ccimp_entry_type)
    {
        case CCIMP_FS_DIR_ENTRY_UNKNOWN:
            ccfsm_file_type = connector_file_system_file_type_none;
            break;
        case CCIMP_FS_DIR_ENTRY_FILE:
            ccfsm_file_type = connector_file_system_file_type_is_reg;
            break;
        case CCIMP_FS_DIR_ENTRY_DIR:
            ccfsm_file_type = connector_file_system_file_type_is_dir;
            break;
    }

    return ccfsm_file_type;
}

static connector_file_system_hash_algorithm_t ccfsm_file_system_hash_algorithm_from_ccimp_fs_hash_alg(ccimp_fs_hash_alg_t ccimp_hash_alg)
{
    connector_file_system_hash_algorithm_t ccfsm_hash_algorithm = connector_file_system_hash_none;

    switch (ccimp_hash_alg)
    {
        case CCIMP_FS_HASH_NONE:
            ccfsm_hash_algorithm = connector_file_system_hash_none;
            break;
        case CCIMP_FS_HASH_BEST:
            ccfsm_hash_algorithm = connector_file_system_hash_best;
            break;
        case CCIMP_FS_HASH_CRC32:
            ccfsm_hash_algorithm = connector_file_system_hash_crc32;
            break;
        case CCIMP_FS_HASH_MD5:
            ccfsm_hash_algorithm = connector_file_system_hash_md5;
            break;
    }

    return ccfsm_hash_algorithm;
}

static ccimp_fs_hash_alg_t ccimp_fs_hash_alg_from_ccfsm_file_system_hash_algorithm(connector_file_system_hash_algorithm_t ccimp_hash_alg)
{
    ccimp_fs_hash_alg_t ccimp_hash_algorithm = connector_file_system_hash_none;

    switch (ccimp_hash_alg)
    {
        case connector_file_system_hash_none:
            ccimp_hash_algorithm = CCIMP_FS_HASH_NONE;
            break;
        case connector_file_system_hash_best:
            ccimp_hash_algorithm = CCIMP_FS_HASH_BEST;
            break;
        case connector_file_system_hash_crc32:
            ccimp_hash_algorithm = CCIMP_FS_HASH_CRC32;
            break;
        case connector_file_system_hash_md5:
            ccimp_hash_algorithm = CCIMP_FS_HASH_MD5;
            break;
    }

    return ccimp_hash_algorithm;
}

static connector_file_system_error_t ccfsm_file_system_error_status_from_ccimp_fs_error(ccimp_fs_error_t ccimp_fs_error)
{
    connector_file_system_error_t ccfsm_fs_error = connector_file_system_unspec_error;

    switch(ccimp_fs_error)
    {
        case CCIMP_FS_ERROR_UNKNOWN:
            ccfsm_fs_error = connector_file_system_unspec_error;
            break;
        case CCIMP_FS_ERROR_PATH_NOT_FOUND:
            ccfsm_fs_error = connector_file_system_path_not_found;
            break;
        case CCIMP_FS_ERROR_INSUFFICIENT_SPACE:
            ccfsm_fs_error = connector_file_system_insufficient_storage_space;
            break;
        case CCIMP_FS_ERROR_FORMAT:
            ccfsm_fs_error = connector_file_system_request_format_error;
            break;
        case CCIMP_FS_ERROR_INVALID_PARAMETER:
            ccfsm_fs_error = connector_file_system_invalid_parameter;
            break;
        case CCIMP_FS_ERROR_INSUFFICIENT_MEMORY:
            ccfsm_fs_error = connector_file_system_out_of_memory;
            break;
        case CCIMP_FS_ERROR_PERMISSION_DENIED:
            ccfsm_fs_error = connector_file_system_permission_denied;
            break;
    }
    return ccfsm_fs_error;
}

static ccimp_session_error_status_t ccimp_session_error_from_ccfsm_session_error(connector_session_error_t ccfsm_session_error)
{
    ccimp_session_error_status_t cccimp_session_error = connector_session_error_none;

    switch(ccfsm_session_error)
      {
        case connector_session_error_none:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_NONE;
            break;
        case connector_session_error_fatal:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_FATAL;
            break;
        case connector_session_error_invalid_opcode:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_INVALID_OPCODE;
            break;
        case connector_session_error_format:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_FORMAT;
            break;
        case connector_session_error_session_in_use:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_SESSION_IN_USE;
            break;
        case connector_session_error_unknown_session:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_UNKNOWN_SESSION;
            break;
        case connector_session_error_compression_failure:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_COMPRESSION_FAILURE;
            break;
        case connector_session_error_decompression_failure:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_DECOMPRESSION_FAILURE;
            break;
        case connector_session_error_memory:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_MEMORY_FAILED;
            break;
        case connector_session_error_send:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_SENDING;
            break;
        case connector_session_error_cancel:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_CANCEL;
            break;
        case connector_session_error_busy:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_BUSY;
            break;
        case connector_session_error_ack:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_ACK;
            break;
        case connector_session_error_timeout:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_TIMEOUT;
            break;
        case connector_session_error_no_service:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_NO_SERVICE;
            break;
        case connector_session_error_count:
            cccimp_session_error = CCIMP_FS_SESSION_ERROR_COUNT;
            break;
      }

      return cccimp_session_error;
}

connector_callback_status_t ccapi_filesystem_handler(connector_request_id_file_system_t filesystem_request, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status;
    ccimp_status_t ccimp_status = CCIMP_STATUS_ERROR;

    UNUSED_ARGUMENT(ccapi_data);
    switch (filesystem_request)
    {
        case connector_request_id_file_system_open:
        {
            connector_file_system_open_t * ccfsm_open_data = data;
            ccimp_fs_file_open_t ccimp_open_data;

            ccimp_open_data.errnum.pointer = NULL;
            ccimp_open_data.handle.pointer = NULL;
            ccimp_open_data.flags = ccfsm_open_data->oflag;
            ccimp_open_data.path = ccfsm_open_data->path;
            ccimp_open_data.imp_context = ccfsm_open_data->user_context;

            ccimp_status = ccimp_fs_file_open(&ccimp_open_data);

            ccfsm_open_data->handle = ccimp_open_data.handle.pointer;
            ccfsm_open_data->errnum = ccimp_open_data.errnum.pointer;
            ccfsm_open_data->user_context = ccimp_open_data.imp_context;
            break;
        }

        case connector_request_id_file_system_read:
        {
            connector_file_system_read_t * ccfsm_read_data = data;
            ccimp_fs_file_read_t ccimp_read_data;

            ccimp_read_data.errnum.pointer = NULL;
            ccimp_read_data.bytes_used = 0;
            ccimp_read_data.handle.pointer = ccfsm_read_data->handle;
            ccimp_read_data.imp_context = ccfsm_read_data->user_context;
            ccimp_read_data.buffer = ccfsm_read_data->buffer;
            ccimp_read_data.bytes_available = ccfsm_read_data->bytes_available;

            ccimp_status = ccimp_fs_file_read(&ccimp_read_data);

            ccfsm_read_data->errnum = ccimp_read_data.errnum.pointer;
            ccfsm_read_data->user_context = ccimp_read_data.imp_context;
            ccfsm_read_data->bytes_used = ccimp_read_data.bytes_used;
            break;
        }

        case connector_request_id_file_system_write:
        {
            connector_file_system_write_t * ccfsm_write_data = data;
            ccimp_fs_file_write_t ccimp_write_data;

            ccimp_write_data.errnum.pointer = NULL;
            ccimp_write_data.bytes_used = 0;
            ccimp_write_data.handle.pointer = ccfsm_write_data->handle;
            ccimp_write_data.imp_context = ccfsm_write_data->user_context;
            ccimp_write_data.buffer = ccfsm_write_data->buffer;
            ccimp_write_data.bytes_available = ccfsm_write_data->bytes_available;

            ccimp_status = ccimp_fs_file_write(&ccimp_write_data);

            ccfsm_write_data->errnum = ccimp_write_data.errnum.pointer;
            ccfsm_write_data->user_context = ccimp_write_data.imp_context;
            ccfsm_write_data->bytes_used = ccimp_write_data.bytes_used;
            break;
        }
        
        case connector_request_id_file_system_lseek:
        {
            connector_file_system_lseek_t * ccfsm_seek_data = data;
            ccimp_fs_file_seek_t ccimp_seek_data;

            ccimp_seek_data.errnum.pointer = NULL;
            ccimp_seek_data.handle.pointer = ccfsm_seek_data->handle;
            ccimp_seek_data.imp_context = ccfsm_seek_data->user_context;
            ccimp_seek_data.resulting_offset = 0;
            ccimp_seek_data.requested_offset = ccfsm_seek_data->requested_offset;
            ccimp_seek_data.origin = ccimp_seek_origin_from_ccfsm_seek_origin(ccfsm_seek_data->origin);

            ccimp_status = ccimp_fs_file_seek(&ccimp_seek_data);

            ccfsm_seek_data->errnum = ccimp_seek_data.errnum.pointer;
            ccfsm_seek_data->user_context = ccimp_seek_data.imp_context;
            ccfsm_seek_data->resulting_offset = ccimp_seek_data.resulting_offset;
            break;
        }

        case connector_request_id_file_system_close:
        {
            connector_file_system_close_t * ccfsm_close_data = data;
            ccimp_fs_file_close_t ccimp_close_data;

            ccimp_close_data.errnum.pointer = NULL;
            ccimp_close_data.handle.pointer = ccfsm_close_data->handle;
            ccimp_close_data.imp_context = ccfsm_close_data->user_context;

            ccimp_status = ccimp_fs_file_close(&ccimp_close_data);

            ccfsm_close_data->errnum = ccimp_close_data.errnum.pointer;
            ccfsm_close_data->user_context = ccimp_close_data.imp_context;
            break;
        }

        case connector_request_id_file_system_remove:
        {
            connector_file_system_remove_t * ccfsm_remove_data = data;
            ccimp_fs_file_remove_t ccimp_remove_data;

            ccimp_remove_data.errnum.pointer = NULL;
            ccimp_remove_data.path = ccfsm_remove_data->path;
            ccimp_remove_data.imp_context = ccfsm_remove_data->user_context;

            ccimp_status = ccimp_fs_file_remove(&ccimp_remove_data);

            ccfsm_remove_data->errnum = ccimp_remove_data.errnum.pointer;
            ccfsm_remove_data->user_context = ccimp_remove_data.imp_context;
            break;
        }

        case connector_request_id_file_system_opendir:
        {
            connector_file_system_opendir_t * ccfsm_dir_open_data = data;
            ccimp_fs_dir_open_t ccimp_dir_open_data;

            ccimp_dir_open_data.errnum.pointer = NULL;
            ccimp_dir_open_data.handle.pointer = NULL;
            ccimp_dir_open_data.path = ccfsm_dir_open_data->path;
            ccimp_dir_open_data.imp_context = ccfsm_dir_open_data->user_context;

            ccimp_status = ccimp_fs_dir_open(&ccimp_dir_open_data);

            ccfsm_dir_open_data->errnum = ccimp_dir_open_data.errnum.pointer;
            ccfsm_dir_open_data->user_context = ccimp_dir_open_data.imp_context;
            ccfsm_dir_open_data->handle = ccimp_dir_open_data.handle.pointer;
            break;
        }

        case connector_request_id_file_system_readdir:
        {
            connector_file_system_readdir_t * ccfsm_dir_read_entry_data = data;
            ccimp_fs_dir_read_entry_t ccimp_dir_read_entry_data;

            ccimp_dir_read_entry_data.errnum.pointer = NULL;
            ccimp_dir_read_entry_data.imp_context = ccfsm_dir_read_entry_data->user_context;
            ccimp_dir_read_entry_data.handle.pointer = ccfsm_dir_read_entry_data->handle;
            ccimp_dir_read_entry_data.entry_name = ccfsm_dir_read_entry_data->entry_name;
            ccimp_dir_read_entry_data.bytes_available = ccfsm_dir_read_entry_data->bytes_available;

            ccimp_status = ccimp_fs_dir_read_entry(&ccimp_dir_read_entry_data);

            ccfsm_dir_read_entry_data->errnum = ccimp_dir_read_entry_data.errnum.pointer;
            ccfsm_dir_read_entry_data->user_context = ccimp_dir_read_entry_data.imp_context;
            break;
        }

        case connector_request_id_file_system_stat_dir_entry:
        {
            connector_file_system_stat_dir_entry_t * ccfsm_dir_entry_status_data = data;
            ccimp_fs_dir_entry_status_t ccimp_dir_entry_status_data;

            ccimp_dir_entry_status_data.errnum.pointer = NULL;
            ccimp_dir_entry_status_data.imp_context = ccfsm_dir_entry_status_data->user_context;
            ccimp_dir_entry_status_data.path = ccfsm_dir_entry_status_data->path;
            ccimp_dir_entry_status_data.status.file_size = 0;
            ccimp_dir_entry_status_data.status.last_modified = 0;
            ccimp_dir_entry_status_data.status.type = CCIMP_FS_DIR_ENTRY_UNKNOWN;

            ccimp_status = ccimp_fs_dir_entry_status(&ccimp_dir_entry_status_data);

            ccfsm_dir_entry_status_data->errnum = ccimp_dir_entry_status_data.errnum.pointer;
            ccfsm_dir_entry_status_data->user_context = ccimp_dir_entry_status_data.imp_context;
            ccfsm_dir_entry_status_data->statbuf.file_size = ccimp_dir_entry_status_data.status.file_size;
            ccfsm_dir_entry_status_data->statbuf.last_modified = ccimp_dir_entry_status_data.status.last_modified;
            ccfsm_dir_entry_status_data->statbuf.flags = ccfsm_file_system_file_type_from_ccimp_fs_dir_entry_type(ccimp_dir_entry_status_data.status.type);
            break;
        }

        case connector_request_id_file_system_closedir:
        {
            connector_file_system_close_t * ccfsm_dir_close_data = data;
            ccimp_fs_dir_close_t ccimp_dir_close_data;

            ccimp_dir_close_data.errnum.pointer = NULL;
            ccimp_dir_close_data.imp_context = ccfsm_dir_close_data->user_context;
            ccimp_dir_close_data.handle.pointer = ccfsm_dir_close_data->handle;

            ccimp_status = ccimp_fs_dir_close(&ccimp_dir_close_data);

            ccfsm_dir_close_data->errnum = ccimp_dir_close_data.errnum.pointer;
            ccfsm_dir_close_data->user_context = ccimp_dir_close_data.imp_context;
            break;
        }

        case connector_request_id_file_system_stat:
        {
            connector_file_system_stat_t * ccfsm_hash_status_data = data;
            ccimp_fs_hash_status_t ccimp_hash_status_data;

            ccimp_hash_status_data.errnum.pointer = NULL;
            ccimp_hash_status_data.imp_context = ccfsm_hash_status_data->user_context;
            ccimp_hash_status_data.path = ccfsm_hash_status_data->path;
            ccimp_hash_status_data.hash_alg.actual = ccfsm_hash_status_data->hash_algorithm.actual;
            ccimp_hash_status_data.hash_alg.requested = ccfsm_hash_status_data->hash_algorithm.requested;
            ccimp_hash_status_data.status.file_size = 0;
            ccimp_hash_status_data.status.last_modified = 0;
            ccimp_hash_status_data.status.type = CCIMP_FS_DIR_ENTRY_UNKNOWN;

            ccimp_status = ccimp_fs_hash_status(&ccimp_hash_status_data);

            ccfsm_hash_status_data->errnum = ccimp_hash_status_data.errnum.pointer;
            ccfsm_hash_status_data->user_context = ccimp_hash_status_data.imp_context;
            ccfsm_hash_status_data->statbuf.file_size = ccimp_hash_status_data.status.file_size;
            ccfsm_hash_status_data->statbuf.last_modified = ccimp_hash_status_data.status.last_modified;
            ccfsm_hash_status_data->statbuf.flags = ccfsm_file_system_file_type_from_ccimp_fs_dir_entry_type(ccimp_hash_status_data.status.type);
            ccfsm_hash_status_data->hash_algorithm.actual = ccfsm_file_system_hash_algorithm_from_ccimp_fs_hash_alg(ccimp_hash_status_data.hash_alg.actual);
            break;
        }

        case connector_request_id_file_system_hash:
        {
            connector_file_system_hash_t * ccfsm_hash_file_data = data;
            ccimp_fs_hash_file_t ccimp_hash_file_data;

            ccimp_hash_file_data.errnum.pointer = NULL;
            ccimp_hash_file_data.imp_context = ccfsm_hash_file_data->user_context;
            ccimp_hash_file_data.path = ccfsm_hash_file_data->path;
            ccimp_hash_file_data.hash_algorithm = ccimp_fs_hash_alg_from_ccfsm_file_system_hash_algorithm(ccfsm_hash_file_data->hash_algorithm);
            ccimp_hash_file_data.hash_value = ccfsm_hash_file_data->hash_value;
            ccimp_hash_file_data.bytes_requested = ccfsm_hash_file_data->bytes_requested;

            ccimp_status = ccimp_fs_hash_file(&ccimp_hash_file_data);

            ccfsm_hash_file_data->errnum = ccimp_hash_file_data.errnum.pointer;
            ccfsm_hash_file_data->user_context = ccimp_hash_file_data.imp_context;
            break;
        }

        case connector_request_id_file_system_get_error:
        {
            connector_file_system_get_error_t * ccfsm_error_desc_data = data;
            ccimp_fs_error_desc_t ccimp_error_desc_data;

            ccimp_error_desc_data.errnum.pointer = ccfsm_error_desc_data->errnum;
            ccimp_error_desc_data.imp_context = ccfsm_error_desc_data->user_context;
            ccimp_error_desc_data.error_string = ccfsm_error_desc_data->buffer;
            ccimp_error_desc_data.bytes_available = ccfsm_error_desc_data->bytes_available;
            ccimp_error_desc_data.bytes_used = 0;
            ccimp_error_desc_data.error_status = CCIMP_FS_ERROR_UNKNOWN;

            ccimp_status = ccimp_fs_error_desc(&ccimp_error_desc_data);

            ccfsm_error_desc_data->user_context = ccimp_error_desc_data.imp_context;
            ccfsm_error_desc_data->bytes_used = ccimp_error_desc_data.bytes_used;
            ccfsm_error_desc_data->error_status = ccfsm_file_system_error_status_from_ccimp_fs_error(ccimp_error_desc_data.error_status);
            break;
        }

        case connector_request_id_file_system_session_error:
        {
            connector_file_system_session_error_t * ccfsm_session_error_data = data;
            ccimp_fs_session_error_t ccimp_session_error_data;

            ccimp_session_error_data.imp_context = ccfsm_session_error_data->user_context;
            ccimp_session_error_data.session_error = ccimp_session_error_from_ccfsm_session_error(ccfsm_session_error_data->session_error);

            ccimp_status = ccimp_fs_session_error(&ccimp_session_error_data);

            ccfsm_session_error_data->user_context = ccimp_session_error_data.imp_context;
            break;
        }
        default:
            break;
    }

    connector_status = connector_callback_status_from_ccimp_status(ccimp_status);
    return connector_status;
}

connector_callback_status_t ccapi_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id, void * const data, void * const context)
{
    connector_callback_status_t status = connector_callback_error;
    ccapi_data_t * ccapi_data = context;

    switch (class_id)
    {
        case connector_class_id_config:
            status = ccapi_config_handler(request_id.config_request, data, ccapi_data);
            break;
        case connector_class_id_operating_system:
            status = ccapi_os_handler(request_id.os_request, data, ccapi_data);
            break;
        case connector_class_id_network_tcp:
            status = ccapi_network_handler(request_id.network_request, data, ccapi_data);
            break;
        case connector_class_id_status:
            status = ccapi_status_handler(request_id.status_request, data, ccapi_data);
            break;
        case connector_class_id_file_system:
            status = ccapi_filesystem_handler(request_id.file_system_request, data, ccapi_data);
            break;
        default:
            status = connector_callback_unrecognized;
            ASSERT_MSG_GOTO(0, done);
            break;
    }

done:
    return status;
}
