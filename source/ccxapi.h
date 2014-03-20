/*
 * ccxapi.h
 *
 *  Created on: Mar 20, 2014
 *      Author: spastor
 */

#ifndef _CCXAPI_H_
#define _CCXAPI_H_

#include "ccapi/ccapi.h"

ccapi_start_error_t ccxapi_start(ccapi_handle_t * const ccapi_handle, ccapi_start_t const * const start);
ccapi_stop_error_t ccxapi_stop(ccapi_handle_t ccapi_handle, ccapi_stop_t behavior);


#endif /* CCXAPI_H_ */
