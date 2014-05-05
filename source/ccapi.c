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


void * ccapi_syncr_create(void)
{
    ccimp_os_syncr_create_t create_data;
    ccimp_status_t const ccimp_status = ccimp_os_syncr_create(&create_data);

    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_BUSY:
        case CCIMP_STATUS_ERROR:
            ccapi_logging_line("ccapi_syncr_create() failed!");
            create_data.syncr_object = NULL;
            break;
    }

    return create_data.syncr_object;
}

void * ccapi_syncr_create_and_release(void)
{
    ccimp_status_t ccimp_status;
    void * syncr_object;
    syncr_object = ccapi_syncr_create();
    if (syncr_object == NULL)
    {
        goto done;
    }

    ccimp_status = ccapi_syncr_release(syncr_object);
    switch(ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_BUSY:
        case CCIMP_STATUS_ERROR:
            syncr_object = NULL;
            goto done;
    }

done:
    return syncr_object;
}

ccimp_status_t ccapi_syncr_acquire(void * syncr_object)
{
    ccimp_os_syncr_acquire_t acquire_data;
    ccimp_status_t ccimp_status = CCIMP_STATUS_ERROR;

    ASSERT_MSG_GOTO(syncr_object != NULL, done);
    acquire_data.syncr_object = syncr_object;
    acquire_data.timeout_ms = OS_SYNCR_ACQUIRE_INFINITE;

    ccimp_status = ccimp_os_syncr_acquire(&acquire_data);
    if (ccimp_status == CCIMP_STATUS_OK && acquire_data.acquired != CCAPI_TRUE)
    {
        ccimp_status = CCIMP_STATUS_ERROR;
    }

done:
    return ccimp_status;
}

ccimp_status_t ccapi_syncr_release(void * syncr_object)
{
    ccimp_os_syncr_release_t release_data;

    release_data.syncr_object = syncr_object;
        
    return ccimp_os_syncr_release(&release_data);
}

ccimp_status_t ccapi_syncr_destroy(void * syncr_object)
{
    ccimp_os_syncr_destroy_t destroy_data;

    destroy_data.syncr_object = syncr_object;

    return ccimp_os_syncr_destroy(&destroy_data);
}

#ifdef CCIMP_FILE_SYSTEM_SERVICE_ENABLED

/* TODO: handle errnum in ccapi_fs functions */

ccimp_status_t ccapi_fs_file_open(ccapi_data_t * const ccapi_data, char const * const local_path, int const flags, ccimp_fs_handle_t * file_handler)
{
    ccimp_fs_file_open_t ccimp_fs_file_open_data = {0};
    ccapi_bool_t loop_done = CCAPI_FALSE;
    ccapi_bool_t syncr_acquired = CCAPI_FALSE;
    ccimp_status_t ccimp_status;

    file_handler->pointer = NULL;

    ccimp_status = ccapi_syncr_acquire(ccapi_data->service.file_system.syncr_access);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            syncr_acquired = CCAPI_TRUE;
            break;
        case CCIMP_STATUS_BUSY:
        case CCIMP_STATUS_ERROR:
            goto done;
    }

    ccimp_fs_file_open_data.path = local_path;
    ccimp_fs_file_open_data.flags = flags;
    ccimp_fs_file_open_data.errnum.pointer = NULL;
    ccimp_fs_file_open_data.imp_context = ccapi_data->service.file_system.imp_context;

    do {
        ccimp_status = ccimp_fs_file_open(&ccimp_fs_file_open_data);
        switch (ccimp_status)
        {
            case CCIMP_STATUS_OK:
            case CCIMP_STATUS_ERROR:
                ccapi_data->service.file_system.imp_context = ccimp_fs_file_open_data.imp_context;
                loop_done = CCAPI_TRUE;
                break;
            case CCIMP_STATUS_BUSY:
                break;
        }
        ccimp_os_yield();
    } while (!loop_done);

    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            goto done;
            break;
    }

    *file_handler = ccimp_fs_file_open_data.handle;

done:
    if (syncr_acquired)
    {
        ASSERT_MSG(ccapi_syncr_release(ccapi_data->service.file_system.syncr_access) == CCIMP_STATUS_OK);
    }

    return ccimp_status;
}

ccimp_status_t ccapi_fs_file_read(ccapi_data_t * const ccapi_data, ccimp_fs_handle_t const file_handler, void * const data, size_t const bytes_available, size_t * const bytes_used)
{
    ccimp_fs_file_read_t ccimp_fs_file_read_data = {0};
    ccapi_bool_t loop_done = CCAPI_FALSE;
    ccapi_bool_t syncr_acquired = CCAPI_FALSE;
    ccimp_status_t ccimp_status;

    *bytes_used = 0;

    ccimp_status = ccapi_syncr_acquire(ccapi_data->service.file_system.syncr_access);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            syncr_acquired = CCAPI_TRUE;
            break;
        case CCIMP_STATUS_BUSY:
        case CCIMP_STATUS_ERROR:
            goto done;
    }

    ccimp_fs_file_read_data.handle = file_handler;
    ccimp_fs_file_read_data.buffer = data;
    ccimp_fs_file_read_data.bytes_available = bytes_available;
    ccimp_fs_file_read_data.errnum.pointer = NULL;
    ccimp_fs_file_read_data.imp_context = ccapi_data->service.file_system.imp_context;

    do {
        ccimp_status = ccimp_fs_file_read(&ccimp_fs_file_read_data);
        switch (ccimp_status)
        {
            case CCIMP_STATUS_OK:
            case CCIMP_STATUS_ERROR:
                ccapi_data->service.file_system.imp_context = ccimp_fs_file_read_data.imp_context;
                loop_done = CCAPI_TRUE;
                break;
            case CCIMP_STATUS_BUSY:
                break;
        }
        ccimp_os_yield();
    } while (!loop_done);

    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            goto done;
            break;
    }

    *bytes_used = ccimp_fs_file_read_data.bytes_used;

done:
    if (syncr_acquired)
    {
        ASSERT_MSG(ccapi_syncr_release(ccapi_data->service.file_system.syncr_access) == CCIMP_STATUS_OK);
    }

    return ccimp_status;
}

ccimp_status_t ccapi_fs_file_close(ccapi_data_t * const ccapi_data, ccimp_fs_handle_t const file_handler)
{
    ccimp_fs_file_close_t ccimp_fs_file_close_data = {0};
    ccapi_bool_t loop_done = CCAPI_FALSE;
    ccapi_bool_t syncr_acquired = CCAPI_FALSE;
    ccimp_status_t ccimp_status;

    ccimp_status = ccapi_syncr_acquire(ccapi_data->service.file_system.syncr_access);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            syncr_acquired = CCAPI_TRUE;
            break;
        case CCIMP_STATUS_BUSY:
        case CCIMP_STATUS_ERROR:
            goto done;
    }

    ccimp_fs_file_close_data.handle = file_handler;
    ccimp_fs_file_close_data.errnum.pointer = NULL;
    ccimp_fs_file_close_data.imp_context = ccapi_data->service.file_system.imp_context;

    do {
        ccimp_status = ccimp_fs_file_close(&ccimp_fs_file_close_data);
        switch (ccimp_status)
        {
            case CCIMP_STATUS_OK:
            case CCIMP_STATUS_ERROR:
                ccapi_data->service.file_system.imp_context = ccimp_fs_file_close_data.imp_context;
                loop_done = CCAPI_TRUE;
                break;
            case CCIMP_STATUS_BUSY:
                break;
        }
        ccimp_os_yield();
    } while (!loop_done);

done:
    if (syncr_acquired)
    {
        ASSERT_MSG(ccapi_syncr_release(ccapi_data->service.file_system.syncr_access) == CCIMP_STATUS_OK);
    }

    return ccimp_status;
}

ccimp_status_t ccapi_fs_dir_entry_status(ccapi_data_t * const ccapi_data, char const * const local_path, ccimp_fs_stat_t * const fs_status)
{
    ccimp_fs_dir_entry_status_t ccimp_fs_dir_entry_status_data = {0};
    ccapi_bool_t loop_done = CCAPI_FALSE;
    ccapi_bool_t syncr_acquired = CCAPI_FALSE;
    ccimp_status_t ccimp_status;

    {
        ccimp_status_t const ccapi_syncr_acquire_status = ccapi_syncr_acquire(ccapi_data->service.file_system.syncr_access);
        switch (ccapi_syncr_acquire_status)
        {
            case CCIMP_STATUS_OK:
                syncr_acquired = CCAPI_TRUE;
                break;
            case CCIMP_STATUS_BUSY:
            case CCIMP_STATUS_ERROR:
                goto done;
        }
    }

    ccimp_fs_dir_entry_status_data.path = local_path;
    ccimp_fs_dir_entry_status_data.errnum.pointer = NULL;
    ccimp_fs_dir_entry_status_data.imp_context = ccapi_data->service.file_system.imp_context;

    do {
        ccimp_status = ccimp_fs_dir_entry_status(&ccimp_fs_dir_entry_status_data);
        switch (ccimp_status)
        {
            case CCIMP_STATUS_OK:
            case CCIMP_STATUS_ERROR:
                ccapi_data->service.file_system.imp_context = ccimp_fs_dir_entry_status_data.imp_context;
                loop_done = CCAPI_TRUE;
                break;
            case CCIMP_STATUS_BUSY:
                break;
        }
        ccimp_os_yield();
    } while (!loop_done);

    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            goto done;
            break;
    }

    memcpy(fs_status, &ccimp_fs_dir_entry_status_data.status, sizeof(ccimp_fs_stat_t));

done:
    if (syncr_acquired)
    {
        ASSERT_MSG(ccapi_syncr_release(ccapi_data->service.file_system.syncr_access) == CCIMP_STATUS_OK);
    }

    return ccimp_status;
}
#endif

connector_status_t connector_initiate_action_secure(ccapi_data_t * const ccapi_data, connector_initiate_request_t const request, void const * const request_data)
{
    connector_status_t status;

    ASSERT_MSG(ccapi_syncr_acquire(ccapi_data->initiate_action_syncr) == CCIMP_STATUS_OK);

    status = connector_initiate_action(ccapi_data->connector_handle, request, request_data);

    ASSERT_MSG(ccapi_syncr_release(ccapi_data->initiate_action_syncr) == CCIMP_STATUS_OK);

    return status;
}

char * ccapi_strdup(char const * const string)
{
    size_t const string_size = strlen(string) + 1;
    char * dup_string = ccapi_malloc(string_size);

    if (dup_string != NULL)
    {
        memcpy(dup_string, string, string_size);
    }

    return dup_string;
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
            callback_status = connector_callback_error;
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
#ifdef CCIMP_FILE_SYSTEM_SERVICE_ENABLED
        case connector_request_id_config_file_system:
            {
                connector_config_supported_t * filesystem_supported = data;
                filesystem_supported->supported = ccapi_data->config.filesystem_supported;
            }
            break;
#endif
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

#ifdef CCIMP_FILE_SYSTEM_SERVICE_ENABLED
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

typedef struct {
    ccimp_fs_handle_t ccimp_handle;
    char * file_path;
    ccapi_fs_request_t request;
} ccapi_fs_file_handle_t;

typedef struct {
    ccapi_fs_virtual_dir_t * dir_entry;
} ccapi_fs_virtual_rootdir_listing_handle_t;

static char const * get_path_without_virtual_dir(char const * const full_path)
{
    char const * c;
    char const * const dir_name_start = full_path + 1; /* Skip leading '/' */

    for (c = dir_name_start; *c != '\0' && *c != CCAPI_FS_DIR_SEPARATOR; c++);

    return c;
}

static char const * get_local_path_from_cloud_path(ccapi_data_t * ccapi_data, char const * const full_path, ccapi_bool_t * const must_free_path)
{
    ccapi_bool_t const virtual_dirs_present = ccapi_data->service.file_system.virtual_dir_list != NULL ? CCAPI_TRUE : CCAPI_FALSE;
    char const * local_path = NULL;
    ccapi_bool_t free_path = CCAPI_FALSE;

    if (!virtual_dirs_present)
    {
        local_path = full_path;
        goto done;
    }

    {
        char const * const path_without_virtual_dir = get_path_without_virtual_dir(full_path);
        char const * const virtual_dir_name_start = full_path + 1;
        size_t const virtual_dir_name_length = path_without_virtual_dir - full_path - sizeof (char);
        ccapi_fs_virtual_dir_t * const dir_entry = *get_pointer_to_dir_entry_from_virtual_dir_name(ccapi_data, virtual_dir_name_start, virtual_dir_name_length);

        if (dir_entry == NULL)
        {
            goto done;
        }

        {
            char * translated_path = NULL;
            size_t path_without_virtual_dir_length = strlen(path_without_virtual_dir);
            translated_path = ccapi_malloc(dir_entry->local_dir_length + path_without_virtual_dir_length + 1);

            ASSERT_MSG_GOTO(translated_path != 0, done);
            memcpy(translated_path, dir_entry->local_dir, strlen(dir_entry->local_dir));
            memcpy(translated_path + dir_entry->local_dir_length, path_without_virtual_dir, path_without_virtual_dir_length);
            translated_path[dir_entry->local_dir_length + path_without_virtual_dir_length] = '\0';
            local_path = translated_path;
            free_path = CCAPI_TRUE;
        }
    }

done:
    *must_free_path = free_path;

    return local_path;
}


static void free_local_path(char const * local_path)
{
    ccapi_free((void *)local_path);
}

connector_callback_status_t ccapi_filesystem_handler(connector_request_id_file_system_t filesystem_request, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status;
    ccimp_status_t ccimp_status = CCIMP_STATUS_ERROR;
    ccapi_bool_t syncr_acquired = CCAPI_FALSE;

    {
        ccimp_status_t const ccapi_syncr_acquire_status = ccapi_syncr_acquire(ccapi_data->service.file_system.syncr_access);

        switch (ccapi_syncr_acquire_status)
        {
            case CCIMP_STATUS_OK:
                syncr_acquired = CCAPI_TRUE;
                break;
            case CCIMP_STATUS_BUSY:
            case CCIMP_STATUS_ERROR:
                ccimp_status = ccapi_syncr_acquire_status;
                goto done;
        }
    }

    switch (filesystem_request)
    {
        case connector_request_id_file_system_open:
        {
            connector_file_system_open_t * ccfsm_open_data = data;
            ccimp_fs_file_open_t ccimp_open_data;
            ccapi_fs_access_t access;
            ccapi_fs_request_t request = CCAPI_FS_REQUEST_UNKNOWN;
            ccapi_bool_t must_free_local_path;
            char const * const local_path = get_local_path_from_cloud_path(ccapi_data, ccfsm_open_data->path, &must_free_local_path);

            if (local_path == NULL)
            {
                goto done;
            }

            ccimp_open_data.path = local_path;
            ccimp_open_data.errnum.pointer = NULL;
            ccimp_open_data.handle.pointer = NULL;
            ccimp_open_data.flags = ccfsm_open_data->oflag;
            ccimp_open_data.imp_context = ccapi_data->service.file_system.imp_context;

            if (ccimp_open_data.flags & CCIMP_FILE_O_WRONLY)
            {
                request = CCAPI_FS_REQUEST_WRITE;
            }
            else if (ccimp_open_data.flags & CCIMP_FILE_O_RDWR)
            {
                request = CCAPI_FS_REQUEST_READWRITE;
            }
            else
            {
                request = CCAPI_FS_REQUEST_READ;
            }
            ASSERT_MSG_GOTO(request != CCAPI_FS_REQUEST_UNKNOWN, done);

            if (ccapi_data->service.file_system.user_callbacks.access_cb != NULL)
            {
                access = ccapi_data->service.file_system.user_callbacks.access_cb(ccimp_open_data.path, request);
            }
            else
            {
                access = CCAPI_FS_ACCESS_ALLOW;
            }

            switch (access)
            {
                case CCAPI_FS_ACCESS_ALLOW:
                    ccimp_status = ccimp_fs_file_open(&ccimp_open_data);
                    break;
                case CCAPI_FS_ACCESS_DENY:
                    ccimp_status = CCIMP_STATUS_ERROR;
                    break;
            }

            ccfsm_open_data->errnum = ccimp_open_data.errnum.pointer;
            ccapi_data->service.file_system.imp_context = ccimp_open_data.imp_context;

            switch (ccimp_status)
            {
                case CCIMP_STATUS_OK:
                {
                    ccapi_fs_file_handle_t * ccapi_fs_handle = ccapi_malloc(sizeof *ccapi_fs_handle);

                    ASSERT_MSG_GOTO(ccapi_fs_handle != NULL, done);
                    ccapi_fs_handle->file_path = ccapi_strdup(ccimp_open_data.path);
                    ASSERT_MSG_GOTO(ccapi_fs_handle->file_path != NULL, done);
                    ccapi_fs_handle->ccimp_handle.pointer = ccimp_open_data.handle.pointer;
                    ccapi_fs_handle->request = request;
                    ccfsm_open_data->handle = ccapi_fs_handle;
                    break;
                }
                case CCIMP_STATUS_ERROR:
                case CCIMP_STATUS_BUSY:
                {
                    ccfsm_open_data->handle = NULL;
                    break;
                }
            }

            if (must_free_local_path)
            {
                free_local_path(local_path);
            }
            break;
        }

        case connector_request_id_file_system_read:
        {
            connector_file_system_read_t * ccfsm_read_data = data;
            ccimp_fs_file_read_t ccimp_read_data;
            ccapi_fs_file_handle_t * ccapi_fs_handle = ccfsm_read_data->handle;

            ccimp_read_data.errnum.pointer = NULL;
            ccimp_read_data.bytes_used = 0;
            ccimp_read_data.handle.pointer = ccapi_fs_handle->ccimp_handle.pointer;
            ccimp_read_data.imp_context = ccapi_data->service.file_system.imp_context;
            ccimp_read_data.buffer = ccfsm_read_data->buffer;
            ccimp_read_data.bytes_available = ccfsm_read_data->bytes_available;

            ccimp_status = ccimp_fs_file_read(&ccimp_read_data);

            ccfsm_read_data->errnum = ccimp_read_data.errnum.pointer;
            ccapi_data->service.file_system.imp_context = ccimp_read_data.imp_context;
            ccfsm_read_data->bytes_used = ccimp_read_data.bytes_used;
            break;
        }

        case connector_request_id_file_system_write:
        {
            connector_file_system_write_t * ccfsm_write_data = data;
            ccimp_fs_file_write_t ccimp_write_data;
            ccapi_fs_file_handle_t * ccapi_fs_handle = ccfsm_write_data->handle;

            ccimp_write_data.errnum.pointer = NULL;
            ccimp_write_data.bytes_used = 0;
            ccimp_write_data.handle.pointer = ccapi_fs_handle->ccimp_handle.pointer;
            ccimp_write_data.imp_context = ccapi_data->service.file_system.imp_context;
            ccimp_write_data.buffer = ccfsm_write_data->buffer;
            ccimp_write_data.bytes_available = ccfsm_write_data->bytes_available;

            ccimp_status = ccimp_fs_file_write(&ccimp_write_data);

            ccfsm_write_data->errnum = ccimp_write_data.errnum.pointer;
            ccapi_data->service.file_system.imp_context = ccimp_write_data.imp_context;
            ccfsm_write_data->bytes_used = ccimp_write_data.bytes_used;
            break;
        }
        
        case connector_request_id_file_system_lseek:
        {
            connector_file_system_lseek_t * ccfsm_seek_data = data;
            ccimp_fs_file_seek_t ccimp_seek_data;
            ccapi_fs_file_handle_t * ccapi_fs_handle = ccfsm_seek_data->handle;

            ccimp_seek_data.errnum.pointer = NULL;
            ccimp_seek_data.handle.pointer = ccapi_fs_handle->ccimp_handle.pointer;
            ccimp_seek_data.imp_context = ccapi_data->service.file_system.imp_context;
            ccimp_seek_data.resulting_offset = 0;
            ccimp_seek_data.requested_offset = ccfsm_seek_data->requested_offset;
            ccimp_seek_data.origin = ccimp_seek_origin_from_ccfsm_seek_origin(ccfsm_seek_data->origin);

            ccimp_status = ccimp_fs_file_seek(&ccimp_seek_data);

            ccfsm_seek_data->errnum = ccimp_seek_data.errnum.pointer;
            ccapi_data->service.file_system.imp_context = ccimp_seek_data.imp_context;
            ccfsm_seek_data->resulting_offset = ccimp_seek_data.resulting_offset;
            break;
        }

        case connector_request_id_file_system_close:
        {
            connector_file_system_close_t * ccfsm_close_data = data;
            ccimp_fs_file_close_t ccimp_close_data;
            ccapi_fs_file_handle_t * ccapi_fs_handle = ccfsm_close_data->handle;

            ccimp_close_data.errnum.pointer = NULL;
            ccimp_close_data.handle.pointer = ccapi_fs_handle->ccimp_handle.pointer;
            ccimp_close_data.imp_context = ccapi_data->service.file_system.imp_context;

            ccimp_status = ccimp_fs_file_close(&ccimp_close_data);

            ccfsm_close_data->errnum = ccimp_close_data.errnum.pointer;
            ccapi_data->service.file_system.imp_context = ccimp_close_data.imp_context;

            if (ccapi_data->service.file_system.user_callbacks.changed_cb != NULL)
            {
                switch (ccapi_fs_handle->request)
                {
                    case CCAPI_FS_REQUEST_READWRITE:
                    case CCAPI_FS_REQUEST_WRITE:
                        ccapi_data->service.file_system.user_callbacks.changed_cb(ccapi_fs_handle->file_path, CCAPI_FS_CHANGED_MODIFIED);
                        break;
                    case CCAPI_FS_REQUEST_LIST:
                    case CCAPI_FS_REQUEST_READ:
                    case CCAPI_FS_REQUEST_UNKNOWN:
                    case CCAPI_FS_REQUEST_REMOVE:
                        break;
                }
            }

            switch(ccimp_status)
            {
                case CCIMP_STATUS_OK:
                case CCIMP_STATUS_ERROR:
                {
                    ccapi_free(ccapi_fs_handle->file_path);
                    ccapi_free(ccapi_fs_handle);
                    break;
                }
                case CCIMP_STATUS_BUSY:
                    break;
            }

            break;
        }

        case connector_request_id_file_system_ftruncate:
        {
            connector_file_system_truncate_t * ccfsm_truncate_data = data;
            ccimp_fs_file_truncate_t ccimp_truncate_data;
            ccapi_fs_file_handle_t * ccapi_fs_handle = ccfsm_truncate_data->handle;

            ccimp_truncate_data.errnum.pointer = NULL;
            ccimp_truncate_data.handle.pointer = ccapi_fs_handle->ccimp_handle.pointer;
            ccimp_truncate_data.imp_context = ccapi_data->service.file_system.imp_context;
            ccimp_truncate_data.length_in_bytes = ccfsm_truncate_data->length_in_bytes;

            ccimp_status = ccimp_fs_file_truncate(&ccimp_truncate_data);

            ccfsm_truncate_data->errnum = ccimp_truncate_data.errnum.pointer;
            ccapi_data->service.file_system.imp_context = ccimp_truncate_data.imp_context;
            break;
        }

        case connector_request_id_file_system_remove:
        {
            connector_file_system_remove_t * ccfsm_remove_data = data;
            ccimp_fs_file_remove_t ccimp_remove_data;
            ccapi_bool_t must_free_local_path;
            ccapi_fs_access_t access;
            char const * const local_path = get_local_path_from_cloud_path(ccapi_data, ccfsm_remove_data->path, &must_free_local_path);

            if (local_path == NULL)
            {
                goto done;
            }

            ccimp_remove_data.path = local_path;
            ccimp_remove_data.errnum.pointer = NULL;
            ccimp_remove_data.imp_context = ccapi_data->service.file_system.imp_context;

            if (ccapi_data->service.file_system.user_callbacks.access_cb != NULL)
            {
                access = ccapi_data->service.file_system.user_callbacks.access_cb(ccimp_remove_data.path, CCAPI_FS_REQUEST_REMOVE);
            }
            else
            {
                access = CCAPI_FS_ACCESS_ALLOW;
            }

            switch (access)
            {
                case CCAPI_FS_ACCESS_ALLOW:
                    ccimp_status = ccimp_fs_file_remove(&ccimp_remove_data);
                    break;
                case CCAPI_FS_ACCESS_DENY:
                    ccimp_status = CCIMP_STATUS_ERROR;
                    break;
            }

            ccfsm_remove_data->errnum = ccimp_remove_data.errnum.pointer;
            ccapi_data->service.file_system.imp_context = ccimp_remove_data.imp_context;
            switch (ccimp_status)
            {
                case CCIMP_STATUS_OK:
                {
                    if (ccapi_data->service.file_system.user_callbacks.changed_cb != NULL)
                    {
                        ccapi_data->service.file_system.user_callbacks.changed_cb(ccimp_remove_data.path, CCAPI_FS_CHANGED_REMOVED);
                    }
                    break;
                }
                case CCIMP_STATUS_ERROR:
                case CCIMP_STATUS_BUSY:
                    break;
            }

            if (must_free_local_path)
            {
                free_local_path(local_path);
            }

            break;
        }

        case connector_request_id_file_system_opendir:
        {
            connector_file_system_opendir_t * ccfsm_dir_open_data = data;

            if (ccfsm_dir_open_data->user_context != NULL)
            {
                ccfsm_dir_open_data->handle = ccfsm_dir_open_data->user_context;
                ccimp_status = CCIMP_STATUS_OK;
            }
            else
            {
                ccimp_fs_dir_open_t ccimp_dir_open_data;
                ccapi_fs_access_t access;
                ccapi_bool_t must_free_local_path;
                char const * const local_path = get_local_path_from_cloud_path(ccapi_data, ccfsm_dir_open_data->path, &must_free_local_path);

                if (local_path == NULL)
                {
                    goto done;
                }

                ccimp_dir_open_data.path = local_path;
                ccimp_dir_open_data.errnum.pointer = NULL;
                ccimp_dir_open_data.handle.pointer = NULL;
                ccimp_dir_open_data.imp_context = ccapi_data->service.file_system.imp_context;

                if (ccapi_data->service.file_system.user_callbacks.access_cb != NULL)
                {
                    access = ccapi_data->service.file_system.user_callbacks.access_cb(ccimp_dir_open_data.path, CCAPI_FS_REQUEST_LIST);
                }
                else
                {
                    access = CCAPI_FS_ACCESS_ALLOW;
                }

                switch (access)
                {
                    case CCAPI_FS_ACCESS_ALLOW:
                        ccimp_status = ccimp_fs_dir_open(&ccimp_dir_open_data);
                        break;
                    case CCAPI_FS_ACCESS_DENY:
                        ccimp_status = CCIMP_STATUS_ERROR;
                        break;
                }

                ccfsm_dir_open_data->errnum = ccimp_dir_open_data.errnum.pointer;
                ccapi_data->service.file_system.imp_context = ccimp_dir_open_data.imp_context;
                ccfsm_dir_open_data->handle = ccimp_dir_open_data.handle.pointer;

                if (must_free_local_path)
                {
                    free_local_path(local_path);
                }
            }
            break;
        }

        case connector_request_id_file_system_readdir:
        {
            connector_file_system_readdir_t * ccfsm_dir_read_entry_data = data;
            if (ccfsm_dir_read_entry_data->user_context != NULL)
            {
                #define CCAPI_MIN_OF(a, b)  ((a) < (b) ? (a) : (b))
                ccapi_fs_virtual_rootdir_listing_handle_t * root_dir_listing_handle = ccfsm_dir_read_entry_data->user_context;
                ccapi_fs_virtual_dir_t * dir_entry = root_dir_listing_handle->dir_entry;

                if (dir_entry != NULL)
                {
                    size_t const virtual_dir_strsize = strlen(dir_entry->virtual_dir) + 1;
                    size_t const mecmpy_size = CCAPI_MIN_OF(virtual_dir_strsize, ccfsm_dir_read_entry_data->bytes_available);

                    memcpy(ccfsm_dir_read_entry_data->entry_name, dir_entry->virtual_dir, mecmpy_size);

                    root_dir_listing_handle->dir_entry = dir_entry->next;
                }
                else
                {
                    strcpy(ccfsm_dir_read_entry_data->entry_name, "");
                }
                ccimp_status = CCIMP_STATUS_OK;
            }
            else
            {
                ccimp_fs_dir_read_entry_t ccimp_dir_read_entry_data;

                ccimp_dir_read_entry_data.errnum.pointer = NULL;
                ccimp_dir_read_entry_data.imp_context = ccapi_data->service.file_system.imp_context;
                ccimp_dir_read_entry_data.handle.pointer = ccfsm_dir_read_entry_data->handle;
                ccimp_dir_read_entry_data.entry_name = ccfsm_dir_read_entry_data->entry_name;
                ccimp_dir_read_entry_data.bytes_available = ccfsm_dir_read_entry_data->bytes_available;

                ccimp_status = ccimp_fs_dir_read_entry(&ccimp_dir_read_entry_data);

                ccfsm_dir_read_entry_data->errnum = ccimp_dir_read_entry_data.errnum.pointer;
                ccapi_data->service.file_system.imp_context = ccimp_dir_read_entry_data.imp_context;
            }
            break;
        }

        case connector_request_id_file_system_stat_dir_entry:
        {
            connector_file_system_stat_dir_entry_t * ccfsm_dir_entry_status_data = data;
            if (ccfsm_dir_entry_status_data->user_context != NULL)
            {
                ccfsm_dir_entry_status_data->statbuf.flags = connector_file_system_file_type_is_dir;
                ccfsm_dir_entry_status_data->statbuf.last_modified = 0;
                ccfsm_dir_entry_status_data->statbuf.file_size = 0;
                ccimp_status = CCIMP_STATUS_OK;
            }
            else
            {
                ccimp_fs_dir_entry_status_t ccimp_dir_entry_status_data;
                ccapi_bool_t must_free_local_path;
                char const * const local_path = get_local_path_from_cloud_path(ccapi_data, ccfsm_dir_entry_status_data->path, &must_free_local_path);

                if (local_path == NULL)
                {
                    goto done;
                }

                ccimp_dir_entry_status_data.path = local_path;
                ccimp_dir_entry_status_data.errnum.pointer = NULL;
                ccimp_dir_entry_status_data.imp_context = ccapi_data->service.file_system.imp_context;
                ccimp_dir_entry_status_data.status.file_size = 0;
                ccimp_dir_entry_status_data.status.last_modified = 0;
                ccimp_dir_entry_status_data.status.type = CCIMP_FS_DIR_ENTRY_UNKNOWN;

                ccimp_status = ccimp_fs_dir_entry_status(&ccimp_dir_entry_status_data);

                ccfsm_dir_entry_status_data->errnum = ccimp_dir_entry_status_data.errnum.pointer;
                ccapi_data->service.file_system.imp_context = ccimp_dir_entry_status_data.imp_context;
                ccfsm_dir_entry_status_data->statbuf.file_size = ccimp_dir_entry_status_data.status.file_size;
                ccfsm_dir_entry_status_data->statbuf.last_modified = ccimp_dir_entry_status_data.status.last_modified;
                ccfsm_dir_entry_status_data->statbuf.flags = ccfsm_file_system_file_type_from_ccimp_fs_dir_entry_type(ccimp_dir_entry_status_data.status.type);

                if (must_free_local_path)
                {
                    free_local_path(local_path);
                }
            }
            break;
        }

        case connector_request_id_file_system_closedir:
        {
            connector_file_system_close_t * ccfsm_dir_close_data = data;
            if (ccfsm_dir_close_data->user_context != NULL)
            {
                ccimp_status = ccapi_free(ccfsm_dir_close_data->user_context);
                ccfsm_dir_close_data->user_context = NULL;
            }
            else
            {
                ccimp_fs_dir_close_t ccimp_dir_close_data;

                ccimp_dir_close_data.errnum.pointer = NULL;
                ccimp_dir_close_data.imp_context = ccapi_data->service.file_system.imp_context;
                ccimp_dir_close_data.handle.pointer = ccfsm_dir_close_data->handle;

                ccimp_status = ccimp_fs_dir_close(&ccimp_dir_close_data);

                ccfsm_dir_close_data->errnum = ccimp_dir_close_data.errnum.pointer;
                ccapi_data->service.file_system.imp_context = ccimp_dir_close_data.imp_context;
            }
            break;
        }

        case connector_request_id_file_system_stat:
        {
            connector_file_system_stat_t * ccfsm_hash_status_data = data;

            if (ccapi_data->service.file_system.virtual_dir_list != NULL && strcmp(ccfsm_hash_status_data->path, CCAPI_FS_ROOT_PATH) == 0)
            {
                ccapi_fs_virtual_rootdir_listing_handle_t * root_dir_listing_handle = malloc(sizeof *root_dir_listing_handle);

                ASSERT_MSG_GOTO(root_dir_listing_handle != NULL, done);
                root_dir_listing_handle->dir_entry = ccapi_data->service.file_system.virtual_dir_list;
                ccfsm_hash_status_data->user_context = root_dir_listing_handle;
                ccfsm_hash_status_data->hash_algorithm.actual = connector_file_system_hash_none;
                ccfsm_hash_status_data->statbuf.file_size = 0;
                ccfsm_hash_status_data->statbuf.last_modified = 0;
                ccfsm_hash_status_data->statbuf.flags = connector_file_system_file_type_is_dir;
                ccimp_status = CCIMP_STATUS_OK;
            }
            else
            {
                ccimp_fs_hash_status_t ccimp_hash_status_data;
                ccapi_bool_t must_free_local_path;
                char const * const local_path = get_local_path_from_cloud_path(ccapi_data, ccfsm_hash_status_data->path, &must_free_local_path);

                if (local_path == NULL)
                {
                    goto done;
                }

                ccimp_hash_status_data.path = local_path;
                ccimp_hash_status_data.errnum.pointer = NULL;
                ccimp_hash_status_data.imp_context = ccapi_data->service.file_system.imp_context;
                ccimp_hash_status_data.hash_alg.actual = CCIMP_FS_HASH_NONE;
                ccimp_hash_status_data.hash_alg.requested = ccimp_fs_hash_alg_from_ccfsm_file_system_hash_algorithm(ccfsm_hash_status_data->hash_algorithm.requested);
                ccimp_hash_status_data.status.file_size = 0;
                ccimp_hash_status_data.status.last_modified = 0;
                ccimp_hash_status_data.status.type = CCIMP_FS_DIR_ENTRY_UNKNOWN;

                ccimp_status = ccimp_fs_hash_status(&ccimp_hash_status_data);

                ccfsm_hash_status_data->errnum = ccimp_hash_status_data.errnum.pointer;
                ccapi_data->service.file_system.imp_context = ccimp_hash_status_data.imp_context;
                ccfsm_hash_status_data->statbuf.file_size = ccimp_hash_status_data.status.file_size;
                ccfsm_hash_status_data->statbuf.last_modified = ccimp_hash_status_data.status.last_modified;
                ccfsm_hash_status_data->statbuf.flags = ccfsm_file_system_file_type_from_ccimp_fs_dir_entry_type(ccimp_hash_status_data.status.type);
                ccfsm_hash_status_data->hash_algorithm.actual = ccfsm_file_system_hash_algorithm_from_ccimp_fs_hash_alg(ccimp_hash_status_data.hash_alg.actual);

                if (must_free_local_path)
                {
                    free_local_path(local_path);
                }
            }
            break;
        }

        case connector_request_id_file_system_hash:
        {
            connector_file_system_hash_t * ccfsm_hash_file_data = data;
            ccimp_fs_hash_file_t ccimp_hash_file_data;
            ccapi_bool_t must_free_local_path;
            char const * const local_path = get_local_path_from_cloud_path(ccapi_data, ccfsm_hash_file_data->path, &must_free_local_path);

            if (local_path == NULL)
            {
                goto done;
            }

            ccimp_hash_file_data.path = local_path;
            ccimp_hash_file_data.errnum.pointer = NULL;
            ccimp_hash_file_data.imp_context = ccapi_data->service.file_system.imp_context;
            ccimp_hash_file_data.hash_algorithm = ccimp_fs_hash_alg_from_ccfsm_file_system_hash_algorithm(ccfsm_hash_file_data->hash_algorithm);
            ccimp_hash_file_data.hash_value = ccfsm_hash_file_data->hash_value;
            ccimp_hash_file_data.bytes_requested = ccfsm_hash_file_data->bytes_requested;

            ccimp_status = ccimp_fs_hash_file(&ccimp_hash_file_data);

            ccfsm_hash_file_data->errnum = ccimp_hash_file_data.errnum.pointer;
            ccapi_data->service.file_system.imp_context = ccimp_hash_file_data.imp_context;

            if (must_free_local_path)
            {
                free_local_path(local_path);
            }
            break;
        }

        case connector_request_id_file_system_get_error:
        {
            connector_file_system_get_error_t * ccfsm_error_desc_data = data;
            ccimp_fs_error_desc_t ccimp_error_desc_data;

            ccimp_error_desc_data.errnum.pointer = ccfsm_error_desc_data->errnum;
            ccimp_error_desc_data.imp_context = ccapi_data->service.file_system.imp_context;
            ccimp_error_desc_data.error_string = ccfsm_error_desc_data->buffer;
            ccimp_error_desc_data.bytes_available = ccfsm_error_desc_data->bytes_available;
            ccimp_error_desc_data.bytes_used = 0;
            ccimp_error_desc_data.error_status = CCIMP_FS_ERROR_UNKNOWN;

            ccimp_status = ccimp_fs_error_desc(&ccimp_error_desc_data);

            ccapi_data->service.file_system.imp_context = ccimp_error_desc_data.imp_context;
            ccfsm_error_desc_data->bytes_used = ccimp_error_desc_data.bytes_used;
            ccfsm_error_desc_data->error_status = ccfsm_file_system_error_status_from_ccimp_fs_error(ccimp_error_desc_data.error_status);
            break;
        }

        case connector_request_id_file_system_session_error:
        {
            connector_file_system_session_error_t * ccfsm_session_error_data = data;
            ccimp_fs_session_error_t ccimp_session_error_data;

            ccimp_session_error_data.imp_context = ccapi_data->service.file_system.imp_context;
            ccimp_session_error_data.session_error = ccimp_session_error_from_ccfsm_session_error(ccfsm_session_error_data->session_error);

            ccimp_status = ccimp_fs_session_error(&ccimp_session_error_data);

            ccapi_data->service.file_system.imp_context = ccimp_session_error_data.imp_context;
            break;
        }
    }

done:
    if (syncr_acquired)
    {
        ccimp_status_t const ccapi_syncr_release_status = ccapi_syncr_release(ccapi_data->service.file_system.syncr_access);

        switch (ccapi_syncr_release_status)
        {
            case CCIMP_STATUS_OK:
                break;
            case CCIMP_STATUS_BUSY:
            case CCIMP_STATUS_ERROR:
                ccimp_status = ccapi_syncr_release_status;
                break;
        }
    }

    connector_status = connector_callback_status_from_ccimp_status(ccimp_status);
    return connector_status;
}
#endif

#ifdef CCIMP_DATA_SERVICE_ENABLED
static connector_callback_status_t ccapi_process_send_data_request(connector_data_service_send_data_t *send_ptr)
{
    connector_callback_status_t status = connector_callback_abort;
	
    if (send_ptr != NULL)
    {
        ccapi_svc_send_data_t * const svc_send = (ccapi_svc_send_data_t *)send_ptr->user_context;
        size_t bytes_expected_to_read;

        ASSERT_MSG_GOTO(svc_send != NULL, done);
        bytes_expected_to_read = (send_ptr->bytes_available > svc_send->bytes_remaining) ? svc_send->bytes_remaining : send_ptr->bytes_available;

        if (svc_send->file_handler.pointer == NULL)
        {
            memcpy(send_ptr->buffer, svc_send->next_data, bytes_expected_to_read);
            svc_send->next_data = ((char *)svc_send->next_data) + bytes_expected_to_read;
        }
        else
        {
            size_t bytes_read;

            ccimp_status_t ccimp_status = ccapi_fs_file_read(svc_send->ccapi_data, svc_send->file_handler, send_ptr->buffer, bytes_expected_to_read, &bytes_read);
            ASSERT_MSG_GOTO(ccimp_status == CCIMP_STATUS_OK, done);

            if (bytes_expected_to_read != bytes_read)
            {
                bytes_expected_to_read = bytes_read;
                ASSERT_MSG(0); /* TODO: Remove later */
            }
        }

        send_ptr->bytes_used = bytes_expected_to_read;
        svc_send->bytes_remaining -= send_ptr->bytes_used;
        send_ptr->more_data = (svc_send->bytes_remaining > 0) ? connector_true : connector_false;

        status = connector_callback_continue;
    }
    else
    {
        ccapi_logging_line("process_send_data_request: no app data set to send\n");
    }

done:
    return status;
}

static connector_callback_status_t ccapi_process_send_data_response(connector_data_service_send_response_t const * const resp_ptr)
{

    ccapi_svc_send_data_t * const svc_send = (ccapi_svc_send_data_t *)resp_ptr->user_context;

    /* TODO: we could have a flag in svc_send where to check if user wants a response or not to skip this callback */

    ccapi_logging_line("Received %s response from Device Cloud\n", (resp_ptr->response == connector_data_service_send_response_success) ? "success" : "error");

    switch (resp_ptr->response)
    {
        case connector_data_service_send_response_success:
            svc_send->response_error = CCAPI_SEND_ERROR_NONE;
            break;
        case connector_data_service_send_response_bad_request:
            svc_send->response_error = CCAPI_SEND_ERROR_RESPONSE_BAD_REQUEST;
            break;
        case connector_data_service_send_response_unavailable:
            svc_send->response_error = CCAPI_SEND_ERROR_RESPONSE_UNAVAILABLE;
            break;
        case connector_data_service_send_response_cloud_error:
            svc_send->response_error = CCAPI_SEND_ERROR_RESPONSE_CLOUD_ERROR;
            break;
        case connector_data_service_send_response_COUNT:
            ASSERT_MSG_GOTO(0, done);
            break;
    }

    if (resp_ptr->hint != NULL)
    {
        ccapi_logging_line("Device Cloud response hint %s\n", resp_ptr->hint);
    }

    if (svc_send->hint != NULL && resp_ptr->hint != NULL)
    {
        strncpy(svc_send->hint->string, resp_ptr->hint, svc_send->hint->length - 1);
        svc_send->hint->string[svc_send->hint->length - 1] = '\0';
    }

done:
    return connector_callback_continue;
}

static connector_callback_status_t ccapi_process_send_data_status(connector_data_service_status_t const * const status_ptr)
{
    ccapi_svc_send_data_t * const svc_send = (ccapi_svc_send_data_t *)status_ptr->user_context;
    connector_callback_status_t connector_status = connector_callback_error;

    ccapi_logging_line("Data service status: %d\n", status_ptr->status);
   
    switch (status_ptr->status)
    {
        case connector_data_service_status_complete:
            svc_send->status_error = CCAPI_SEND_ERROR_NONE;
            break;
        case connector_data_service_status_cancel:
            svc_send->status_error = CCAPI_SEND_ERROR_STATUS_CANCEL;
            break;
        case connector_data_service_status_timeout:
            svc_send->status_error = CCAPI_SEND_ERROR_STATUS_TIMEOUT;
            break;
        case connector_data_service_status_session_error:
            svc_send->status_error = CCAPI_SEND_ERROR_STATUS_SESSION_ERROR;
            break;
        case connector_data_service_status_COUNT:
            ASSERT_MSG_GOTO(0, done);
            break;
    }

    ASSERT_MSG_GOTO(svc_send->send_syncr != NULL, done);
    
    connector_status = (ccapi_syncr_release(svc_send->send_syncr) == CCIMP_STATUS_OK) ? connector_callback_continue : connector_callback_error;

done:
    return connector_status;
}

connector_callback_status_t ccapi_data_service_handler(connector_request_id_data_service_t const data_service_request, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t connector_status;

    UNUSED_ARGUMENT(ccapi_data);

    switch (data_service_request)
    {
        case connector_request_id_data_service_send_data:
        {
            connector_data_service_send_data_t * send_ptr = data;

            connector_status = ccapi_process_send_data_request(send_ptr);

            break;
        }
        case connector_request_id_data_service_send_response:
        {
            connector_data_service_send_response_t * const resp_ptr = data;

            connector_status = ccapi_process_send_data_response(resp_ptr);

            break;
        }
        case connector_request_id_data_service_send_status:
        {
            connector_data_service_status_t * const status_ptr = data;
            
            connector_status = ccapi_process_send_data_status(status_ptr);

            break;
        }
        default:
            connector_status = connector_callback_unrecognized;
            ASSERT_MSG_GOTO(0, done);
            break;
    }

done:
    return connector_status;
}
#endif

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
#ifdef CCIMP_FILE_SYSTEM_SERVICE_ENABLED
        case connector_class_id_file_system:
            status = ccapi_filesystem_handler(request_id.file_system_request, data, ccapi_data);
            break;
#endif
#ifdef CCIMP_DATA_SERVICE_ENABLED
        case connector_class_id_data_service:
            status = ccapi_data_service_handler(request_id.data_service_request, data, ccapi_data);
            break;
#endif
        default:
            status = connector_callback_unrecognized;
            ASSERT_MSG_GOTO(0, done);
            break;
    }

done:
    return status;
}
