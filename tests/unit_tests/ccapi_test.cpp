#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"

extern "C" {
#include "CppUTestExt/MockSupport_c.h"
#include "ccapi/ccapi.h"
#include "internal/ccapi_definitions.h"
#include "ccimp/ccimp_os.h"
}

void * (*previous_ccimp_malloc)(size_t size);

void * ccimp_malloc_NULL(size_t size)
{
    UNUSED_ARGUMENT(size);
    return NULL;
}

void * ccimp_malloc_mock(size_t size)
{
    mock_c()->actualCall("ccimp_malloc");
    {
        void * pointer = mock_c()->returnValue().value.pointerValue;

        if (pointer != NULL)
            return ccimp_malloc_real(size);
        else
            return pointer;
    }
}

using namespace std;

static void fill_start_structure_with_good_parameters(ccapi_start_t * start)
{
    uint8_t device_id[DEVICE_ID_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x9D, 0xFF, 0xFF, 0xAB, 0xCD, 0xEF};
    char const * const device_cloud_url = "login.etherios.com";
    char const * const device_type = "Device type";
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

TEST_GROUP(ccapi_init_test)
{
};

TEST(ccapi_init_test, testParamNULL)
{
    ccapi_init_error_t error;
    ccapi_start_t * start = NULL;

    error = ccapi_start(start);
    CHECK(error == CCAPI_INIT_ERROR_NULL_PARAMETER);
}

TEST(ccapi_init_test, testVendorIdZero)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    start.vendor_id = 0;
    error = ccapi_start(&start);

    CHECK_EQUAL(error, CCAPI_INIT_ERROR_INVALID_VENDORID);
}

TEST(ccapi_init_test, testInvalidDeviceId)
{
    uint8_t device_id[DEVICE_ID_LENGTH] = {0};
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    memcpy(start.device_id, device_id, sizeof start.device_id);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_INVALID_DEVICEID);
}

TEST(ccapi_init_test, testNullDeviceCloudURL)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    start.device_cloud_url = NULL;
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_INVALID_URL);
}

TEST(ccapi_init_test, testInvalidDeviceCloudURL)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    start.device_cloud_url = "";
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_INVALID_URL);
}

TEST(ccapi_init_test, testNullDeviceType)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    start.device_type = NULL;
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_INVALID_DEVICETYPE);
}

TEST(ccapi_init_test, testInvalidDeviceType)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    start.device_type = "";
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_INVALID_DEVICETYPE);
}

TEST(ccapi_init_test, testStartGoodParameters)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_NONE);
}

TEST(ccapi_init_test, testNoMemory)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);

    UT_PTR_SET(ccimp_malloc, ccimp_malloc_NULL);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_INSUFFICIENT_MEMORY);
}

TEST(ccapi_init_test, testVendorID)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_NONE);
    CHECK(start.vendor_id == ccapi_config->vendor_id);
}

TEST(ccapi_init_test, testDeviceID)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_NONE);
    CHECK(memcmp(start.device_id, ccapi_config->device_id, sizeof start.device_id) == 0);
}

TEST(ccapi_init_test, testDeviceType)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);

    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_NONE);
    CHECK(strcmp(start.device_type, ccapi_config->device_type) == 0);
}

TEST(ccapi_init_test, testDeviceTypeNoMemory)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;
    void * pointer1 = &pointer1; /* Not-Null */
    void * pointer2 = NULL;

    fill_start_structure_with_good_parameters(&start);

    UT_PTR_SET(ccimp_malloc, ccimp_malloc_mock);

    mock().expectOneCall("ccimp_malloc")
            .andReturnValue(pointer1);
    mock().expectOneCall("ccimp_malloc")
            .andReturnValue(pointer2);
    error = ccapi_start(&start);

    mock().clear();
    CHECK(error == CCAPI_INIT_ERROR_INSUFFICIENT_MEMORY);
}


TEST(ccapi_init_test, testCliNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_NONE);
    CHECK(ccapi_config->cli_supported == CCAPI_FALSE);
}

TEST(ccapi_init_test, testCliSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;
    void * pointer = pointer; /* Not-NULL */

    fill_start_structure_with_good_parameters(&start);
    start.service.cli = &pointer;
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_NONE);
    CHECK(ccapi_config->cli_supported == CCAPI_TRUE);
}

TEST(ccapi_init_test, testReceiveSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;
    void * pointer = pointer; /* Not-NULL */

    fill_start_structure_with_good_parameters(&start);
    start.service.receive = &pointer;
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_NONE);
    CHECK(ccapi_config->receive_supported == CCAPI_TRUE);
}

TEST(ccapi_init_test, testReceiveNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_NONE);
    CHECK(ccapi_config->receive_supported == CCAPI_FALSE);
}

TEST(ccapi_init_test, testFirmwareSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;
    void * pointer = pointer; /* Not-NULL */

    fill_start_structure_with_good_parameters(&start);
    start.service.firmware = &pointer;
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_NONE);
    CHECK(ccapi_config->firmware_supported == CCAPI_TRUE);
}

TEST(ccapi_init_test, testFirmwareNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_NONE);
    CHECK(ccapi_config->firmware_supported == CCAPI_FALSE);
}

TEST(ccapi_init_test, testRciSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;
    void * pointer = pointer; /* Not-NULL */

    fill_start_structure_with_good_parameters(&start);
    start.service.rci = &pointer;
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_NONE);
    CHECK(ccapi_config->rci_supported == CCAPI_TRUE);
}

TEST(ccapi_init_test, testRciNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_NONE);
    CHECK(ccapi_config->rci_supported == CCAPI_FALSE);
}

TEST(ccapi_init_test, testFileSysSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;
    void * pointer = pointer; /* Not-NULL */

    fill_start_structure_with_good_parameters(&start);
    start.service.file_system = &pointer;
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_NONE);
    CHECK(ccapi_config->filesystem_supported == CCAPI_TRUE);
}

TEST(ccapi_init_test, testFileSysNotSupported)
{
    ccapi_start_t start = {0};
    ccapi_init_error_t error;

    fill_start_structure_with_good_parameters(&start);
    error = ccapi_start(&start);

    CHECK(error == CCAPI_INIT_ERROR_NONE);
    CHECK(ccapi_config->filesystem_supported == CCAPI_FALSE);
}
