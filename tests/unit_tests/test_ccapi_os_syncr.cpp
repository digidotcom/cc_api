#include "CppUTest/CommandLineTestRunner.h"

#define CCAPI_CONST_PROTECTION_UNLOCK

#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
}

#include "test_helper_functions.h"

using namespace std;

TEST_GROUP(ccapi_os_syncr_test)
{
    void * syncr_object;

    void setup()
    {
        syncr_object = NULL;
        {
            ccimp_os_syncr_create_t create_data;
            ccimp_status_t status;
    
            status = ccimp_os_syncr_create(&create_data);

            CHECK(status == CCIMP_STATUS_OK);

            syncr_object= create_data.syncr_object;
        }
        CHECK(syncr_object != NULL);

        Mock_create_all();
    }

    void teardown()
    {
        Mock_destroy_all();

        {
            ccimp_os_syncr_destroy_t destroy_data;
            ccimp_status_t status;

            destroy_data.syncr_object = syncr_object;
        
            status = ccimp_os_syncr_destroy(&destroy_data);

            CHECK(status == CCIMP_STATUS_OK);
        }
    }
};

TEST(ccapi_os_syncr_test, CreateLeavesObjectInClearedState)
{
    ccimp_status_t status;

    ccimp_os_syncr_adquire_t adquire_data;

    adquire_data.syncr_object = syncr_object;       
    adquire_data.timeout_ms= 100;

    /* Should fail to adquire the object as it's created in clear state */
    status = ccimp_os_syncr_adquire(&adquire_data);

    CHECK(status == CCIMP_STATUS_OK);
    CHECK(adquire_data.acquired == CCAPI_FALSE);
}

static void * thread_release(void * argument)
{
    ccimp_status_t status;
    ccimp_os_syncr_release_t release_data;

    release_data.syncr_object = argument;
        
    status = ccimp_os_syncr_release(&release_data);

    CHECK(status == CCIMP_STATUS_OK);

    return NULL;
}

pthread_t create_thread_release(void * argument)
{
    pthread_t pthread;
    int ccode = pthread_create(&pthread, NULL, thread_release, argument);

    if (ccode != 0)
    {
        printf("create_thread_release() error %d\n", ccode);
    }

    return pthread;
}

TEST(ccapi_os_syncr_test, AdquireClearsObjectAutomatically)
{
    ccimp_os_syncr_adquire_t adquire_data;
    ccimp_status_t status;

    adquire_data.syncr_object = syncr_object;       

    create_thread_release(syncr_object);

    /* Wait infinite. The other thread should release the object */
    adquire_data.timeout_ms= OS_SYNCR_ADQUIRE_INFINITE;

    status = ccimp_os_syncr_adquire(&adquire_data);

    CHECK(status == CCIMP_STATUS_OK);
    CHECK(adquire_data.acquired == CCAPI_TRUE);

    /* Should fail to adquire again the object */
    adquire_data.timeout_ms= OS_SYNCR_ADQUIRE_NOWAIT;

    status = ccimp_os_syncr_adquire(&adquire_data);

    CHECK(status == CCIMP_STATUS_OK);
    CHECK(adquire_data.acquired == CCAPI_FALSE);
}

/* This test fails as linux semaphores don't support max count.
   TODO: Should we delete that limitation?
 */
IGNORE_TEST(ccapi_os_syncr_test, ReleaseMaxCountIs1)
{
    ccimp_os_syncr_adquire_t adquire_data;
    ccimp_os_syncr_release_t release_data;
    ccimp_status_t status;

    adquire_data.syncr_object = syncr_object;       
    release_data.syncr_object = syncr_object;       

    status = ccimp_os_syncr_release(&release_data);
    CHECK(status == CCIMP_STATUS_OK);

    /* This should have no effect */
    status = ccimp_os_syncr_release(&release_data);
    CHECK(status == CCIMP_STATUS_OK);

    /* Wait infinite. The object should be released */
    adquire_data.timeout_ms= OS_SYNCR_ADQUIRE_INFINITE;

    status = ccimp_os_syncr_adquire(&adquire_data);

    CHECK(status == CCIMP_STATUS_OK);
    CHECK(adquire_data.acquired == CCAPI_TRUE);

    /* Should fail to adquire again the object as Max Count is 1 */
    adquire_data.timeout_ms= OS_SYNCR_ADQUIRE_NOWAIT;

    status = ccimp_os_syncr_adquire(&adquire_data);

    CHECK(status == CCIMP_STATUS_OK);
    CHECK(adquire_data.acquired == CCAPI_FALSE);
}

