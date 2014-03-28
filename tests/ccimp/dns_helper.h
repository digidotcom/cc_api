/*
 * Copyright (c) 2013 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */

#ifndef _NETWORK_DNS_H
#define _NETWORK_DNS_H

int dns_resolve(char const * const domain_name, in_addr_t * const ip_addr);
void dns_set_redirected(int const state);
void dns_cache_invalidate(void);

#endif
