#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

static ccapi_fs_virtual_dir_t * * get_pointer_to_last_dir_entry(ccapi_data_t * const ccapi_data)
{
    ccapi_fs_virtual_dir_t * * p_last_dir_entry = &ccapi_data->service.file_system.virtual_dir_list;

    do {
        if (*p_last_dir_entry != NULL)
        {
            p_last_dir_entry = &(*p_last_dir_entry)->next;
        }
    } while (*p_last_dir_entry != NULL);

    return p_last_dir_entry;
}

static ccapi_fs_error_t add_virtual_dir_entry(ccapi_data_t * const ccapi_data, ccapi_fs_virtual_dir_t * const new_entry)
{
    ccapi_fs_error_t error = CCAPI_FS_ERROR_NONE;
    ccapi_fs_virtual_dir_t * * const p_last_entry = get_pointer_to_last_dir_entry(ccapi_data);
    *p_last_entry = new_entry;

    return error;
}

static ccapi_fs_virtual_dir_t * create_virtual_dir_entry(char const * const virtual_dir, char const * const local_dir)
{
    ccapi_fs_virtual_dir_t * new_dir_entry = ccapi_malloc(sizeof *new_dir_entry);

    if (new_dir_entry == NULL)
    {
        goto done;
    }

    new_dir_entry->local_path = ccapi_strdup(local_dir);
    if (new_dir_entry->local_path == NULL)
    {
        reset_heap_ptr(&new_dir_entry);
        goto done;
    }

    new_dir_entry->virtual_path = ccapi_strdup(virtual_dir);
    if (new_dir_entry->virtual_path == NULL)
    {
        ccapi_free(new_dir_entry->local_path);
        reset_heap_ptr(&new_dir_entry);
        new_dir_entry = NULL;
        goto done;
    }
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
    } while (loop_done == CCAPI_FALSE);

    if (ccimp_status != CCIMP_STATUS_OK)
    {
        is_dir = CCAPI_FALSE;
        goto done;
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
    } while (loop_done == CCAPI_FALSE);

    ASSERT_MSG_GOTO(ccimp_status == CCIMP_STATUS_OK, done);
done:
    return is_dir;
}

ccapi_fs_error_t ccxapi_fs_add_virtual_dir(ccapi_data_t * const ccapi_data, char const * const virtual_dir, char const * const local_dir)
{
    ccapi_fs_error_t error = CCAPI_FS_ERROR_NONE;
    ccapi_fs_virtual_dir_t * new_dir_entry;

    if (virtual_dir == NULL || local_dir == NULL || virtual_dir[0] == '\0' || virtual_dir[0] == '/' || local_dir[0] == '\0')
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

    if (is_a_dir(ccapi_data, local_dir) != CCAPI_TRUE)
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

static ccapi_fs_virtual_dir_t * * get_pointer_to_dir_entry_from_virtual_path(ccapi_data_t * const ccapi_data, char const * const virtual_dir)
{
    ccapi_fs_virtual_dir_t * * p_dir_entry = &ccapi_data->service.file_system.virtual_dir_list;
    ccapi_bool_t finished = CCAPI_FALSE;

    do {
        ccapi_fs_virtual_dir_t * dir_entry = *p_dir_entry;

        if (dir_entry != NULL)
        {
            if (strcmp(dir_entry->virtual_path, virtual_dir) == 0)
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
    } while (finished == CCAPI_FALSE);

    return p_dir_entry;
}

ccapi_fs_error_t ccxapi_fs_remove_virtual_dir(ccapi_data_t * const ccapi_data, char const * const virtual_dir)
{
    ccapi_fs_error_t error = CCAPI_FS_ERROR_NONE;
    ccapi_fs_virtual_dir_t * * p_dir_entry = NULL;

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

    p_dir_entry = get_pointer_to_dir_entry_from_virtual_path(ccapi_data, virtual_dir);
    if (*p_dir_entry == NULL)
    {
        error = CCAPI_FS_ERROR_NOT_MAPPED;
        goto done;
    }
    else
    {
        ccapi_fs_virtual_dir_t * next_dir_entry = (*p_dir_entry)->next;
        ccapi_fs_virtual_dir_t * dir_entry = *p_dir_entry;

        ccapi_free(dir_entry->local_path);
        ccapi_free(dir_entry->virtual_path);
        ccapi_free(dir_entry);
        *p_dir_entry = next_dir_entry;
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
