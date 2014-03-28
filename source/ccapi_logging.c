
#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

#if (defined CCIMP_DEBUG_ENABLED)

void * logging_syncr = NULL;

/* TODO:
       - categories
*/

ccimp_status_t ccapi_logging_lock_acquire(void)
{
    ccimp_os_syncr_acquire_t acquire_data;
    ccimp_status_t status = CCIMP_STATUS_ABORT;
    
    if (logging_syncr != NULL)
    {
        acquire_data.syncr_object = logging_syncr;
        acquire_data.timeout_ms= OS_SYNCR_ACQUIRE_INFINITE;

        status = ccimp_os_syncr_acquire(&acquire_data);
    }

    return status;
}

ccimp_status_t ccapi_logging_lock_release(void)
{
    ccimp_os_syncr_release_t release_data;
    ccimp_status_t status = CCIMP_STATUS_ABORT;
    
    if (logging_syncr != NULL)
    {
        release_data.syncr_object = logging_syncr;

        status = ccimp_os_syncr_release(&release_data);
    }

    return status;
}

void connector_debug_vprintf(debug_t const debug, char const * const format, va_list args)
{
    ccapi_logging_lock_acquire();

    /* TODO: Macro in ccapi_definitions.h? */
    ccimp_hal_logging_vprintf(debug, format, args);

    ccapi_logging_lock_release();
}

#define CALL_LOGGING_VPRINTF(type, format) \
    do \
    { \
        va_list args; \
 \
        va_start(args, (format)); \
        ccimp_hal_logging_vprintf((type), (format), args); \
        va_end(args); \
    } \
    while (0)

void ccapi_logging_line(char const * const format, ...)
{
    ccapi_logging_lock_acquire();
    CALL_LOGGING_VPRINTF(debug_all, format);
    ccapi_logging_lock_release();
}

static void ccapi_logging_line_beg(char const * const format, ...)
{
    CALL_LOGGING_VPRINTF(debug_beg, format);
}

static void ccapi_logging_line_mid(char const * const format, ...)
{
    CALL_LOGGING_VPRINTF(debug_mid, format);
}

static void ccapi_logging_line_end(char const * const format, ...)
{
    CALL_LOGGING_VPRINTF(debug_end, format);
}

void ccapi_logging_print_buffer(char const * const label, void const * const buffer, size_t const length)
{
    size_t i;
    uint8_t const * const content = buffer;

    ccapi_logging_lock_acquire();

    ccapi_logging_line_beg("%s:", label);
    for (i = 0; i < length; i++)
    {
        if ((i % 16) == 0)
        {
            ccapi_logging_line_mid("\n");
        }

        ccapi_logging_line_mid(" %02X", content[i]);
    }
    ccapi_logging_line_end("");

    ccapi_logging_lock_release();
}
#else
void ccapi_logging_line(char const * const format, ...)
{
    (void)format;
}
void ccapi_logging_print_buffer(char const * const label, void const * const buffer, size_t const length)
{  
    (void)label;
    (void)buffer;
    (void)length;
}
#endif /* (defined CCIMP_DEBUG_ENABLED) */

