/*
 * ccimp_os.h
 *
 *  Created on: Feb 12, 2014
 *      Author: spastor
 */

#ifndef CCIMP_OS_H_
#define CCIMP_OS_H_

#include "ccimp/ccimp_types.h"

#if (defined UNIT_TEST)
extern void * (*ccimp_malloc)(size_t size);
extern void * ccimp_malloc_real(size_t size);

#else
void * ccimp_malloc(size_t size);
#endif

#endif
