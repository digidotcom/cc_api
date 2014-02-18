#include "mocks/mock_ccimp_os.h"

void Mock_ccimp_malloc_create(void)
{
    return;
}

void Mock_ccimp_malloc_destroy(void)
{
    mock().clear();
}

void Mock_ccimp_malloc_expectAndReturn(size_t expect, void * retval)
{
    mock().expectOneCall("ccimp_malloc")
            .withParameter("size", (int)expect)
            .andReturnValue(retval);
}

void Mock_ccimp_create_thread_create(void)
{
    return;
}

void Mock_ccimp_create_thread_destroy(void)
{
    mock().clear();
}

void Mock_ccimp_create_thread_expectAndReturn(ccimp_create_thread_info_t * const create_thread_info, ccapi_bool_t retval)
{
    UNUSED_ARGUMENT(create_thread_info);
    mock().expectOneCall("ccimp_create_thread")
            .andReturnValue(retval);
}

extern "C" {
#include "CppUTestExt/MockSupport_c.h"
#include "ccapi_definitions.h"
#include <pthread.h>

/******************** LINUX IMPLEMENTATION ********************/
static void * thread_wrapper(void * argument)
{
    ccimp_create_thread_info_t * create_thread_info = (ccimp_create_thread_info_t *)argument;

    create_thread_info->start(create_thread_info->argument);

    return NULL;
}

ccapi_bool_t ccimp_create_thread_real(ccimp_create_thread_info_t * const create_thread_info)
{
    pthread_t pthread;
    int ccode = pthread_create(&pthread, NULL, thread_wrapper, create_thread_info);

    if (ccode != 0)
    {
        printf("ccimp_create_thread() error %d\n", ccode);
        return (CCAPI_FALSE);
    }

    return CCAPI_TRUE;
}
/***********************************************************/

ccapi_bool_t ccimp_create_thread(ccimp_create_thread_info_t * create_thread_info)
{
    mock_c()->actualCall("ccimp_create_thread");
    if ((ccapi_bool_t)mock_c()->returnValue().value.intValue == CCAPI_TRUE)
    {
        ccimp_create_thread_real(create_thread_info);
    }

    return (ccapi_bool_t)mock_c()->returnValue().value.intValue;
}

void * ccimp_malloc(size_t size)
{
    mock_c()->actualCall("ccimp_malloc")->withIntParameters("size", size);
    return mock_c()->returnValue().value.pointerValue;
}

}
