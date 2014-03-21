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

#include <stdarg.h>
#include "connector_debug.h"

/**
 * Debug output from Cloud Connector, Writes a formatted string to stdout, expanding the format
 * tags with the value of the argument list arg.  This function behaves exactly as
 * vprintf() except an additional argument is passed indicating which part of the line is represented.
 *
 * debug_beg: Start of line. A fixed string could be added to each line to distinguish differing
 * types of output or to enable locking of the output until debug_eol is sent (too keep content from
 * breaking up in the middle). If format is NULL, there is no content.
 *
 * debug_mid: Middle of line. No header or trailer should be added.
 *
 * debug_end: End of line. A trailer may be added, but a newline MUST be added. At the point the message
 * is complete and if locking is enabled the unlock should be here. If format is NULL, there is no content.
 *
 * debug_all: The full message is contained in this one call. Equivalent to calling:
 *
 *      connector_debug_vprintf(debug_beg, format, args);
 *      connector_debug_vprintf(debug_end, "", args);
 *
 */
void ccimp_debug_vprintf(debug_t const debug, char const * const format, va_list args);

/**
 *  Verify that the condition is true, otherwise halt the program.
 */
#define ASSERT(cond)        assert(cond)

#else
#define ASSERT(cond)
#endif

#endif /* CCIMP_DEBUG_H_ */
