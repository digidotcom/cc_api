#ifndef _MOCK_CCIMP_LOGGING_H_
#define _MOCK_CCIMP_LOGGING_H_

#define CCIMP_LOGGING_PRINTF_DOESNT_EXPECT_A_CALL   "_MOCK_DOESNT_EXPECT_A_CALL_"

void Mock_ccimp_logging_printf_create(void);
void Mock_ccimp_logging_printf_destroy(void);
void Mock_ccimp_hal_logging_vprintf_expect(debug_t const debug, char const * const buffer);

extern "C" {
void ccimp_hal_logging_vprintf_real(debug_t const debug, char const * const format, va_list args);
ccimp_status_t ccimp_hal_halt_real(void);

/* TODO: Move to mock_ccimp_reset.cpp */
ccimp_status_t ccimp_hal_reset_real(void);
}

#endif
