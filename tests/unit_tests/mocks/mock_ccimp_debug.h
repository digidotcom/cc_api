/*
 * mock_ccimp_os.h
 *
 *  Created on: Feb 14, 2014
 *      Author: spastor
 */

#ifndef _MOCK_CCIMP_DEBUG_H_
#define _MOCK_CCIMP_DEBUG_H_

#include "CppUTestExt/MockSupport.h"

extern "C" {
#include "ccapi_definitions.h"
#include "custom/custom_debug.h"
}

#define CCIMP_DEBUG_PRINTF_DOESNT_EXPECT_A_CALL   "_MOCK_DOESNT_EXPECT_A_CALL_"

void Mock_ccimp_debug_printf_create(void);
void Mock_ccimp_debug_printf_destroy(void);
void Mock_ccimp_debug_printf_expect(char const * const message);

extern "C" {
void ccimp_debug_vprintf_real(debug_t const debug, char const * const format, va_list args);
}

#endif
