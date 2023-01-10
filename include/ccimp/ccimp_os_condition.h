#ifndef CCIMP_OS_CONDITION_H__
#define CCIMP_OS_CONDITION_H__

#include "ccimp_types.h"

#define OS_CONDITION_WAIT_INFINITE 0

typedef struct ccimp_os_condition_st ccimp_os_condition_st;

typedef ccapi_bool_t (*ccimp_os_condition_predicate_fn)(void * pv);

ccimp_status_t
ccimp_os_condition_init(ccimp_os_condition_st * cond);

ccimp_status_t
ccimp_os_condition_destroy(ccimp_os_condition_st * cond);

ccimp_os_condition_st *
ccimp_os_condition_alloc(void);

void
ccimp_os_condition_free(ccimp_os_condition_st * cond);

ccimp_status_t
ccimp_os_condition_lock(ccimp_os_condition_st * cond);

ccimp_status_t
ccimp_os_condition_unlock(ccimp_os_condition_st * cond);

ccimp_status_t
ccimp_os_condition_wait(
    ccimp_os_condition_st * cond,
    unsigned millisecs,
    ccimp_os_condition_predicate_fn predicate_cb,
    void * cb_ctx);

ccimp_status_t
ccimp_os_condition_signal(ccimp_os_condition_st * cond);

ccimp_status_t
ccimp_os_condition_broadcast(ccimp_os_condition_st * cond);

#endif /* CCIMP_OS_CONDITION_H__ */

