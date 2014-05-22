#ifndef CCIMP_HAL_H_
#define CCIMP_HAL_H_

#include "ccimp/ccimp_types.h"

#if (defined CCIMP_DEBUG_ENABLED)
ccimp_status_t ccimp_hal_halt(void);
#endif

ccimp_status_t ccimp_hal_reset(void);

#endif
