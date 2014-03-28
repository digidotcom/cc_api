/*
 * ccimp_hal.h
 *
 *  Created on: Mar 24, 2014
 *      Author: hbujanda
 */

#ifndef CCIMP_HAL_H_
#define CCIMP_HAL_H_

#include "custom/custom_connector_config.h"
#include "ccimp/ccimp_types.h"

#if (defined CCIMP_DEBUG_ENABLED)
ccimp_status_t ccimp_hal_halt(void);
#endif

ccimp_status_t ccimp_hal_reset(void);

#endif
