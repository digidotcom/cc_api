/*
 * ccapi_connector_config.h
 *
 *  Created on: Mar 6, 2014
 *      Author: hbujanda
 */

#ifndef _CCAPI_CONNECTOR_CONFIG_H_
#define _CCAPI_CONNECTOR_CONFIG_H_

#include "custom/custom_connector_config.h"

/* if implementer does not define CCIMP_DEBUG_ENABLED, there won't be debugging in any layer */
#ifdef CCIMP_DEBUG_ENABLED
#define CONNECTOR_DEBUG
#endif

#ifdef CCIMP_SM_UDP_MAX_RX_SEGMENTS
#define CONNECTOR_SM_UDP_MAX_RX_SEGMENTS CCIMP_SM_UDP_MAX_RX_SEGMENTS
#endif

#ifdef CCIMP_SM_SMS_MAX_RX_SEGMENTS
#define CONNECTOR_SM_SMS_MAX_RX_SEGMENTS CCIMP_SM_SMS_MAX_RX_SEGMENTS
#endif

#endif /* _CCAPI_CONNECTOR_CONFIG_H_ */
