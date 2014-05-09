/*
 * ccimp_logging.c
 *
 *  Created on: Mar 7, 2014
 *      Author: hbujanda
 */
#include "ccimp/ccimp_logging.h"

#if (defined UNIT_TEST)
#define ccimp_hal_logging_vprintf       ccimp_hal_logging_vprintf_real
#endif

/******************** LINUX IMPLEMENTATION ********************/

#if (defined CCIMP_DEBUG_ENABLED)
#include <stdio.h>
#include <stdlib.h>

void ccimp_hal_logging_vprintf(debug_t const debug, char const * const format, va_list args)
{
    if ((debug == debug_all) || (debug == debug_beg))
    {
        /* lock mutex here. */
        printf("CCAPI: ");
    }

    vprintf(format, args);

    if ((debug == debug_all) || (debug == debug_end))
    {
        /* unlock mutex here */
        printf("\n");
        fflush(stdout);
    }
}
#else
 /* to avoid ISO C forbids an empty translation unit compiler error */
typedef int dummy;
#endif
