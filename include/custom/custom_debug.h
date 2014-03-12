/*
 * ccustom_debug.h
 *
 *  Created on: Mar 6, 2014
 *      Author: hbujanda
 */

#ifndef CCIMP_DEBUG_H_
#define CCIMP_DEBUG_H_

#include "custom/custom_connector_config.h"

#if (defined CCIMP_DEBUG_ENABLED)
/**
 * Debug output from Cloud Connector, Writes a formatted string to stdout
 * This function behaves exactly as printf.
 *
 */
void ccimp_debug_printf(char const * const message);

/**
 *  Verify that the condition is true, otherwise halt the program.
 */
#define ASSERT(cond)        assert(cond)

#else
#define ASSERT(cond)
#endif

#endif /* CCIMP_DEBUG_H_ */
