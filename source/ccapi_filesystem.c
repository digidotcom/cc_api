#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

static ccapi_fs_error_t add_virtual_dir_entry(ccapi_data_t * const ccapi_data, ccapi_fs_virtual_dir_t * const new_entry)
{
    ccapi_fs_error_t error = CCAPI_FS_ERROR_NONE;
    ccimp_status_t ccimp_status;

    ccimp_status = ccapi_syncr_acquire(ccapi_data->service.file_system.syncr_access);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            error = CCAPI_FS_ERROR_SYNCR_FAILED;
            goto done;
    }

    {
        ccapi_fs_virtual_dir_t * const next_entry = ccapi_data->service.file_system.virtual_dir_list;

        new_entry->next = next_entry;
        ccapi_data->service.file_system.virtual_dir_list = new_entry;
    }

    ccimp_status = ccapi_syncr_release(ccapi_data->service.file_system.syncr_access);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            error = CCAPI_FS_ERROR_SYNCR_FAILED;
            ASSERT_MSG_GOTO(ccimp_status == CCIMP_STATUS_OK, done);
            goto done;
    }

done:
    return error;
}

static ccapi_fs_virtual_dir_t * create_virtual_dir_entry(char const * const virtual_dir, char const * const local_dir)
{
    ccapi_fs_virtual_dir_t * new_dir_entry = ccapi_malloc(sizeof *new_dir_entry);

    if (new_dir_entry == NULL)
    {
        goto done;
    }

    new_dir_entry->local_dir = ccapi_strdup(local_dir);
    if (new_dir_entry->local_dir == NULL)
    {
        reset_heap_ptr(&new_dir_entry);
        goto done;
    }

    new_dir_entry->virtual_dir = ccapi_strdup(virtual_dir);
    if (new_dir_entry->virtual_dir == NULL)
    {
        ccapi_free(new_dir_entry->local_dir);
        reset_heap_ptr(&new_dir_entry);
        new_dir_entry = NULL;
        goto done;
    }
    new_dir_entry->virtual_dir_length = strlen(virtual_dir);
    new_dir_entry->local_dir_length = strlen(local_dir);
    new_dir_entry->next = NULL;
done:
    return new_dir_entry;
}

static ccapi_bool_t is_a_dir(ccapi_data_t * const ccapi_data, char const * const local_dir)
{
    ccapi_bool_t is_dir = CCAPI_TRUE;
    ccimp_fs_dir_open_t ccimp_fs_dir_open_data = {0};
    ccimp_fs_dir_close_t ccimp_fs_dir_close_data = {0};
    ccimp_status_t ccimp_status;
    ccapi_bool_t loop_done = CCAPI_FALSE;

    ccimp_fs_dir_open_data.path = local_dir;
    ccimp_fs_dir_open_data.handle.pointer = NULL;
    ccimp_fs_dir_open_data.errnum.pointer = NULL;
    ccimp_fs_dir_open_data.imp_context = ccapi_data->service.file_system.imp_context;

    do {
        ccimp_status = ccimp_fs_dir_open(&ccimp_fs_dir_open_data);
        switch (ccimp_status)
        {
            case CCIMP_STATUS_OK:
            case CCIMP_STATUS_ERROR:
                loop_done = CCAPI_TRUE;
                break;
            case CCIMP_STATUS_BUSY:
                break;
        }
        ccimp_os_yield();
    } while (!loop_done);

    ccapi_data->service.file_system.imp_context = ccimp_fs_dir_open_data.imp_context;

    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            is_dir = CCAPI_FALSE;
            goto done;
            break;
    }

    ccimp_fs_dir_close_data.errnum.pointer = NULL;
    ccimp_fs_dir_close_data.handle.pointer = ccimp_fs_dir_open_data.handle.pointer;
    ccimp_fs_dir_close_data.imp_context = ccapi_data->service.file_system.imp_context;

    loop_done = CCAPI_FALSE;
    do {
        ccimp_status = ccimp_fs_dir_close(&ccimp_fs_dir_close_data);
        switch (ccimp_status)
        {
            case CCIMP_STATUS_OK:
            case CCIMP_STATUS_ERROR:
                loop_done = CCAPI_TRUE;
                break;
            case CCIMP_STATUS_BUSY:
                break;
        }
        ccimp_os_yield();
    } while (!loop_done);

    ASSERT_MSG_GOTO(ccimp_status == CCIMP_STATUS_OK, done);
    ccapi_data->service.file_system.imp_context = ccimp_fs_dir_open_data.imp_context;

done:
    return is_dir;
}

ccapi_fs_error_t ccxapi_fs_add_virtual_dir(ccapi_data_t * const ccapi_data, char const * const virtual_dir, char const * const local_dir)
{
    ccapi_fs_error_t error = CCAPI_FS_ERROR_NONE;
    ccapi_fs_virtual_dir_t * new_dir_entry;

    if (virtual_dir == NULL || local_dir == NULL || virtual_dir[0] == '\0' || local_dir[0] == '\0' || strstr(virtual_dir, "/") != NULL || strstr(virtual_dir, "\\"))
    {
        error = CCAPI_FS_ERROR_INVALID_PATH;
        goto done;
    }

    if (ccapi_data == NULL || ccapi_data->thread.connector_run->status == CCAPI_THREAD_NOT_STARTED)
    {
        ccapi_logging_line("ccxapi_fs_add_virtual_dir: CCAPI not started");

        error = CCAPI_FS_ERROR_CCAPI_STOPPED;
        goto done;
    }

    if (NULL != *get_pointer_to_dir_entry_from_virtual_dir_name(ccapi_data, virtual_dir, strlen(virtual_dir)))
    {
        error = CCAPI_FS_ERROR_ALREADY_MAPPED;
        goto done;
    }

    if (!is_a_dir(ccapi_data, local_dir))
    {
        error = CCAPI_FS_ERROR_NOT_A_DIR;
        goto done;
    }

    new_dir_entry = create_virtual_dir_entry(virtual_dir, local_dir);
    if (new_dir_entry == NULL)
    {
        error = CCAPI_FS_ERROR_INSUFFICIENT_MEMORY;
        goto done;
    }
    error = add_virtual_dir_entry(ccapi_data, new_dir_entry);

done:
    return error;
}

ccapi_fs_virtual_dir_t * * get_pointer_to_dir_entry_from_virtual_dir_name(ccapi_data_t * const ccapi_data, char const * const virtual_dir, unsigned int virtual_dir_length)
{
    ccapi_fs_virtual_dir_t * * p_dir_entry = &ccapi_data->service.file_system.virtual_dir_list;
    ccapi_bool_t finished = CCAPI_FALSE;

    do {
        ccapi_fs_virtual_dir_t * dir_entry = *p_dir_entry;

        if (dir_entry != NULL)
        {
            unsigned int const longest_strlen = CCAPI_MAX_OF(dir_entry->virtual_dir_length, virtual_dir_length);
            if (strncmp(dir_entry->virtual_dir, virtual_dir, longest_strlen) == 0)
            {
                finished = CCAPI_TRUE;
            }
            else
            {
                p_dir_entry = &dir_entry->next;
            }
        }
        else
        {
            finished = CCAPI_TRUE;
        }
    } while (!finished);

    return p_dir_entry;
}

ccapi_fs_error_t ccxapi_fs_remove_virtual_dir(ccapi_data_t * const ccapi_data, char const * const virtual_dir)
{
    ccapi_fs_error_t error = CCAPI_FS_ERROR_NONE;
    ccapi_fs_virtual_dir_t * * p_dir_entry = NULL;
    ccimp_status_t ccimp_status;

    if (ccapi_data == NULL || ccapi_data->thread.connector_run->status == CCAPI_THREAD_NOT_STARTED)
    {
        ccapi_logging_line("ccxapi_fs_remove_virtual_dir: CCAPI not started");

        error = CCAPI_FS_ERROR_CCAPI_STOPPED;
        goto done;
    }

    if (virtual_dir == NULL || virtual_dir[0] == '\0' || virtual_dir[0] == '/')
    {
        error = CCAPI_FS_ERROR_INVALID_PATH;
        goto done;
    }

    p_dir_entry = get_pointer_to_dir_entry_from_virtual_dir_name(ccapi_data, virtual_dir, strlen(virtual_dir));
    if (*p_dir_entry == NULL)
    {
        error = CCAPI_FS_ERROR_NOT_MAPPED;
        goto done;
    }

    ccimp_status = ccapi_syncr_acquire(ccapi_data->service.file_system.syncr_access);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            error = CCAPI_FS_ERROR_SYNCR_FAILED;
            goto done;
    }

    {
        ccapi_fs_virtual_dir_t * next_dir_entry = (*p_dir_entry)->next;
        ccapi_fs_virtual_dir_t * dir_entry = *p_dir_entry;

        ccapi_free(dir_entry->local_dir);
        ccapi_free(dir_entry->virtual_dir);
        ccapi_free(dir_entry);
        *p_dir_entry = next_dir_entry;
    }

    ccimp_status = ccapi_syncr_release(ccapi_data->service.file_system.syncr_access);
    switch (ccimp_status)
    {
        case CCIMP_STATUS_OK:
            break;
        case CCIMP_STATUS_ERROR:
        case CCIMP_STATUS_BUSY:
            error = CCAPI_FS_ERROR_SYNCR_FAILED;
            goto done;
    }
done:
    return error;
}

ccapi_fs_error_t ccapi_fs_add_virtual_dir(char const * const virtual_dir, char const * const local_dir)
{
    return ccxapi_fs_add_virtual_dir(ccapi_data_single_instance, virtual_dir, local_dir);
}

ccapi_fs_error_t ccapi_fs_remove_virtual_dir(char const * const virtual_dir)
{
    return ccxapi_fs_remove_virtual_dir(ccapi_data_single_instance, virtual_dir);
}
