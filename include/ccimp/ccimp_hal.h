/*
 * ccimp_hal.h
 *
 *  Created on: Mar 24, 2014
 *      Author: hbujanda
 */

#ifndef CCIMP_HAL_H_
#define CCIMP_HAL_H_

#include "ccimp/ccimp_types.h"

#if (defined CCIMP_DEBUG_ENABLED)
/* TODO: call it ccimp_hal_halt() ??? */
ccimp_status_t ccimp_hal_assertion_hit(char const * const message);
#endif

ccimp_status_t ccimp_hal_reset(void);

#endif
