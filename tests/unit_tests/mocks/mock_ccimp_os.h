/*
 * mock_ccimp_os.h
 *
 *  Created on: Feb 14, 2014
 *      Author: spastor
 */

#ifndef _MOCK_CCIMP_OS_H_
#define _MOCK_CCIMP_OS_H_

void Mock_ccimp_malloc_create(void);
void Mock_ccimp_malloc_destroy(void);
void Mock_ccimp_malloc_expectAndReturn(size_t expect, void * retval);

#endif
