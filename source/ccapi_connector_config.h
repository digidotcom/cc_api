/*
 * ccapi_connector_config.h
 *
 *  Created on: Mar 6, 2014
 *      Author: hbujanda
 */

#ifndef _CCAPI_CONNECTOR_CONFIG_H_
#define _CCAPI_CONNECTOR_CONFIG_H_

#include "custom/custom_connector_config.h"

/* if implementer does not define CCAPI_DEBUG, there won't be debugging in any layer */
#ifdef CCAPI_DEBUG
#define CONNECTOR_DEBUG
#endif

#endif /* _CCAPI_CONNECTOR_CONFIG_H_ */
