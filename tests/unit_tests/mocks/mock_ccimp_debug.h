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

#define MOCK_DEBUG_ENABLED 1	/* Mock disabled. Do printf normally */

void Mock_ccimp_debug_create(void);
void Mock_ccimp_debug_destroy(void);
void Mock_ccimp_debug_expect(char const * const message);

extern "C" {
void ccimp_debug_printf_real(char const * const message);
}

#endif
