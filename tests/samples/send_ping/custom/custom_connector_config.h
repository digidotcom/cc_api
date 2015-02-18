/*
* Copyright (c) 2014 Etherios, a Division of Digi International, Inc.
* All rights not expressly granted are reserved.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Etherios 11001 Bren Road East, Minnetonka, MN 55343
* =======================================================================
*/

#ifndef _CUSTOM_CONNECTOR_CONFIG_H_
#define _CUSTOM_CONNECTOR_CONFIG_H_

/* Cloud Connector Configuration Categories */

/* Transports */
#define CCIMP_UDP_TRANSPORT_ENABLED
#define CCIMP_SMS_TRANSPORT_ENABLED

/* Services */
#undef CCIMP_DATA_SERVICE_ENABLED
#undef CCIMP_DATA_POINTS_ENABLED
#undef CCIMP_RCI_SERVICE_ENABLED
#undef CCIMP_FIRMWARE_SERVICE_ENABLED
#undef CCIMP_FILE_SYSTEM_SERVICE_ENABLED

/* OS Features */
#define CCIMP_LITTLE_ENDIAN
#define CCIMP_COMPRESSION_ENABLED
#define CCIMP_64_BIT_INTEGERS_SUPPORTED
#define CCIMP_FLOATING_POINT_SUPPORTED

#define CCIMP_HAS_STDINT_HEADER

/* Debugging (Logging / Halt) */
#define CCIMP_DEBUG_ENABLED

/* Limits */
#define CCIMP_FILE_SYSTEM_MAX_PATH_LENGTH   256
#undef  CCIMP_FILE_SYSTEM_LARGE_FILES_SUPPORTED

#define CCIMP_SM_UDP_MAX_RX_SEGMENTS   256
#define CCIMP_SM_SMS_MAX_RX_SEGMENTS   256

#define CCIMP_IDLE_SLEEP_TIME_MS 100

#endif /* _CUSTOM_CONNECTOR_CONFIG_H_ */
