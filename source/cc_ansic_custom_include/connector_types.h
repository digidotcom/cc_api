#ifndef CONNECTOR_TYPES_H_
#define CONNECTOR_TYPES_H_

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stddef.h>

#include "custom/custom_ccimp_types.h"

/**
 * The Cloud Connector uses this function to produce output string according to the format.
 * This function behaves exactly as snprintf except that the variable argument list is passed
 * as a va_list instead of a succession of arguments.
 *
 * In the C library the prototype for vsnprintf is defined as vsnprintf(char * str, size_t size, const char *format, va_list ap);
 *
 */
int connector_snprintf(char * const str, size_t const size, char const * const format, ...);

/**
* @defgroup connector_network_handle_t Network Handle
* @{
*/
/**
 * Used to reference a network session and is passed into Cloud Connector API calls. This
 * type is defined as void * and is used by the application to 
 * keep track of and identify sessions.
 */
typedef void * connector_network_handle_t;
/**
* @}
*/


#endif
