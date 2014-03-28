/*
 * test_helper_functions.cpp
 *
 *  Created on: Mar 18, 2014
 *      Author: spastor
 */

#include <stdio.h>
#include <pthread.h>
#include "test_helper_functions.h"

void fill_start_structure_with_good_parameters(ccapi_start_t * start)
{
    uint8_t device_id[DEVICE_ID_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x9D, 0xFF, 0xFF, 0xAB, 0xCD, 0xEF};
    char const * const device_cloud_url = DEVICE_CLOUD_URL_STRING;
    char const * const device_type = DEVICE_TYPE_STRING;
    start->vendor_id = 0x12345678; /* Set vendor_id or ccapi_init_error_invalid_vendorid will be returned instead */
    memcpy(start->device_id, device_id, sizeof start->device_id);
    start->device_cloud_url = device_cloud_url;
    start->device_type = device_type;

    start->service.cli = NULL;
    start->service.receive = NULL;
    start->service.file_system = NULL;
    start->service.firmware = NULL;
    start->service.rci = NULL;
}

static void * thread_wrapper(void * argument)
{

    ccapi_start((ccapi_start_t *)argument);

    return NULL;
}

pthread_t aux_ccapi_start(void * argument)
{
    pthread_t pthread;
    int ccode = pthread_create(&pthread, NULL, thread_wrapper, argument);

    if (ccode != 0)
    {
        printf("aux_ccapi_start() error %d\n", ccode);
    }

    return pthread;
}

int stop_aux_thread(pthread_t pthread)
{
    int error;

    error = pthread_cancel(pthread);
    if (error < 0)
    {
        printf("pthread_cancel failed with %d\n", error);
        goto done;
    }

    error = pthread_join(pthread, NULL);
    if (error < 0)
    {
        printf("pthread_cancel failed with %d\n", error);
        goto done;
    }
done:
    return error;
}
