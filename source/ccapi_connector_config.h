/*
 * ccapi_connector_config.h
 *
 *  Created on: Mar 6, 2014
 *      Author: hbujanda
 */

#ifndef _CCAPI_CONNECTOR_CONFIG_H_
#define _CCAPI_CONNECTOR_CONFIG_H_

#include "custom/custom_connector_config.h"

/* +++ Layer2 Digi Controlled defines +++ */

#undef CONNECTOR_SM_UDP_MAX_SESSIONS
#undef CONNECTOR_SM_SMS_MAX_SESSIONS

/* --- Layer2 Digi Controlled defines --- */


/* +++ Match Layer2 User Controlled defines to Layer1 defines +++ */

#ifdef CCIMP_DEBUG_ENABLED
/* if implementer does not define CCIMP_DEBUG_ENABLED, there won't be debugging in any layer */
#define CONNECTOR_DEBUG
#endif

#ifdef CCIMP_SM_UDP_MAX_RX_SEGMENTS
#define CONNECTOR_SM_UDP_MAX_RX_SEGMENTS CCIMP_SM_UDP_MAX_RX_SEGMENTS
#endif

#ifdef CCIMP_SM_SMS_MAX_RX_SEGMENTS
#define CONNECTOR_SM_SMS_MAX_RX_SEGMENTS CCIMP_SM_SMS_MAX_RX_SEGMENTS
#endif

/* --- Match Layer2 User Controlled defines to Layer1 defines --- */

#endif /* _CCAPI_CONNECTOR_CONFIG_H_ */
