/*
 * ccapi_debug.h
 *
 *  Created on: Mar 10, 2014
 *      Author: hbujanda
 */

#ifndef _CCAPI_DEBUG_H_
#define _CCAPI_DEBUG_H_

typedef enum {
    LEVEL_INFO,
    LEVEL_WARNING,
    LEVEL_ERROR,
	LEVEL_COUNT
} ccapi_debug_level_t;

typedef enum {
    ZONE_NONE       = 0,
    ZONE_LAYER1     = 1,
    ZONE_START_STOP = 2,
    ZONE_TRANSPORT  = 4,
    ZONE_SEND_DATA  = 8,
    ZONE_ALL        = ((ZONE_SEND_DATA << 1) - 1)
} ccapi_debug_zones_t;

typedef enum {
    CCAPI_CONFIG_DEBUG_ERROR_NONE,
    CCAPI_CONFIG_DEBUG_ERROR_NOT_STARTED,
    CCAPI_CONFIG_DEBUG_ERROR_INVALID_ZONES,
    CCAPI_CONFIG_DEBUG_ERROR_INVALID_LEVEL,
    CCAPI_CONFIG_DEBUG_ERROR_COUNT
} ccapi_config_debug_error_t;

#endif
