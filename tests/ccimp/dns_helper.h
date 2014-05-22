#ifndef _NETWORK_DNS_H
#define _NETWORK_DNS_H

int dns_resolve(char const * const domain_name, in_addr_t * const ip_addr);
void dns_set_redirected(int const state);
void dns_cache_invalidate(void);

#endif
