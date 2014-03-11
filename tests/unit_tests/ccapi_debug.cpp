#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupport.h"
#include "mocks/mock_ccimp_debug.h"
#include "mocks/mock_connector_api.h"
extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"

#include "connector_debug.h"	/* For Layer1 interface tests */
}

using namespace std;

#define DEVICE_TYPE_STRING      "Device type"
#define DEVICE_CLOUD_URL_STRING "login.etherios.com"

static void fill_start_structure_with_good_parameters(ccapi_start_t * start)
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

TEST_GROUP(ccapi_debug_test)
{
    void setup()
    {
        Mock_connector_run_create();
    }

    void teardown()
    {
        Mock_connector_run_destroy();

        mock().checkExpectations();

        ASSERT_CLEAN()

        mock().removeAllComparators();
        mock().clear();

        /* Manually cleaning ccapi_data while we don't have a ccapi_stop() function */
        free(ccapi_data);
        ccapi_data = NULL;
    }
};

/* Testing ccapi_start() debug configuration */
TEST(ccapi_debug_test, testDbgStartZoneNotEnabled)
{
    ccapi_init_error_t error;
    ccapi_start_t start = {0};
    fill_start_structure_with_good_parameters(&start);

    start.debug.init_zones = ZONE_NONE;
    start.debug.init_level = LEVEL_INFO;

    /* Manually enable the mock to check that nothing is called */
    mock("ccimp_debug").setData("behavior", MOCK_DEBUG_ENABLED);

    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);
}

TEST(ccapi_debug_test, testDbgStartLevelNotEnabled)
{
    ccapi_init_error_t error;
    ccapi_start_t start = {0};
    fill_start_structure_with_good_parameters(&start);

    start.debug.init_zones = ZONE_ALL;
    start.debug.init_level = LEVEL_ERROR;

    /* Manually enable the mock to check that nothing is called */
    mock("ccimp_debug").setData("behavior", MOCK_DEBUG_ENABLED);

    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);
}

TEST(ccapi_debug_test, testDbgStartZoneAndLevelEnabled)
{
    ccapi_init_error_t error;
    ccapi_start_t start = {0};
    fill_start_structure_with_good_parameters(&start);

    start.debug.init_zones = ZONE_START_STOP;
    start.debug.init_level = LEVEL_INFO;

    Mock_ccimp_debug_expect("ccapi_start ret 0\n");

    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);
}

TEST(ccapi_debug_test, testDbgStartZoneAndLevelEnabledALL)
{
    ccapi_init_error_t error;
    ccapi_start_t start = {0};
    fill_start_structure_with_good_parameters(&start);

    start.debug.init_zones = ZONE_ALL;
    start.debug.init_level = LEVEL_INFO;

    Mock_ccimp_debug_expect("ccapi_start ret 0\n");

    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);
}

TEST(ccapi_debug_test, testDbgStartZoneError)
{
    ccapi_init_error_t error;
    ccapi_start_t start = {0};
    fill_start_structure_with_good_parameters(&start);

    start.debug.init_zones = (ccapi_debug_zones_t)(ZONE_ALL + 1);
    start.debug.init_level = LEVEL_INFO;

    /* Manually enable the mock to check that nothing is called */
    mock("ccimp_debug").setData("behavior", MOCK_DEBUG_ENABLED);

    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_INVALID_DEBUG_CONFIG);
}

TEST(ccapi_debug_test, testDbgStartLevelError)
{
    ccapi_init_error_t error;
    ccapi_start_t start = {0};
    fill_start_structure_with_good_parameters(&start);

    start.debug.init_zones = ZONE_ALL;
    start.debug.init_level = LEVEL_COUNT;

    /* Manually enable the mock to check that nothing is called */
    mock("ccimp_debug").setData("behavior", MOCK_DEBUG_ENABLED);

    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_INVALID_DEBUG_CONFIG);
}

/* Testing ccapi_config_debug() */
TEST(ccapi_debug_test, testDbgConfigErrorNotStarted)
{
    ccapi_config_debug_error_t config_debug_error;

    config_debug_error = ccapi_config_debug(ZONE_ALL, LEVEL_INFO);
    CHECK(config_debug_error == CCAPI_CONFIG_DEBUG_ERROR_NOT_STARTED);

    /* Manually enable the mock to check that nothing is called */
    mock("ccimp_debug").setData("behavior", MOCK_DEBUG_ENABLED);

    ccapi_debug_printf(ZONE_START_STOP, LEVEL_INFO, "hello testDbgConfigErrorNotStarted\n");
}

TEST(ccapi_debug_test, testDbgConfigErrorInvalidZones)
{
    ccapi_init_error_t error;
    ccapi_config_debug_error_t config_debug_error;
    ccapi_start_t start = {0};
    fill_start_structure_with_good_parameters(&start);

    start.debug.init_zones = ZONE_NONE;

    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    config_debug_error = ccapi_config_debug((ccapi_debug_zones_t)(ZONE_ALL + 1), LEVEL_INFO);
    CHECK(config_debug_error == CCAPI_CONFIG_DEBUG_ERROR_INVALID_ZONES);

    /* Manually enable the mock to check that nothing is called */
    mock("ccimp_debug").setData("behavior", MOCK_DEBUG_ENABLED);

    ccapi_debug_printf(ZONE_START_STOP, LEVEL_INFO, "hello testDbgConfigErrorInvalidZones\n");
}

TEST(ccapi_debug_test, testDbgConfigErrorInvalidLevel)
{
    ccapi_init_error_t error;
    ccapi_config_debug_error_t config_debug_error;
    ccapi_start_t start = {0};
    fill_start_structure_with_good_parameters(&start);

    start.debug.init_zones = ZONE_NONE;

    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    config_debug_error = ccapi_config_debug(ZONE_ALL, LEVEL_COUNT);
    CHECK(config_debug_error == CCAPI_CONFIG_DEBUG_ERROR_INVALID_LEVEL);

    /* Manually enable the mock to check that nothing is called */
    mock("ccimp_debug").setData("behavior", MOCK_DEBUG_ENABLED);

    ccapi_debug_printf(ZONE_START_STOP, LEVEL_INFO, "hello testDbgConfigErrorInvalidZones\n");
}

TEST(ccapi_debug_test, testDbgConfigZoneAndLevelEnabled)
{
    ccapi_init_error_t error;
    ccapi_config_debug_error_t config_debug_error;
    ccapi_start_t start = {0};
    fill_start_structure_with_good_parameters(&start);

    start.debug.init_zones = ZONE_NONE;

    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    config_debug_error = ccapi_config_debug(ZONE_SEND_DATA, LEVEL_INFO);
    CHECK(config_debug_error == CCAPI_CONFIG_DEBUG_ERROR_NONE);

    Mock_ccimp_debug_expect("hello testDbgStartZoneAndLevelEnabled\n");

    ccapi_debug_printf(ZONE_SEND_DATA, LEVEL_INFO, "hello testDbgStartZoneAndLevelEnabled\n");
}

TEST(ccapi_debug_test, testDbgConfigZoneAndLevelDisabled)
{
    ccapi_init_error_t error;
    ccapi_config_debug_error_t config_debug_error;
    ccapi_start_t start = {0};
    fill_start_structure_with_good_parameters(&start);

    start.debug.init_zones = ZONE_NONE;

    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    config_debug_error = ccapi_config_debug(ZONE_NONE, LEVEL_INFO);
    CHECK(config_debug_error == CCAPI_CONFIG_DEBUG_ERROR_NONE);

    /* Manually enable the mock to check that nothing is called */
    mock("ccimp_debug").setData("behavior", MOCK_DEBUG_ENABLED);

    ccapi_debug_printf(ZONE_SEND_DATA, LEVEL_INFO, "hello testDbgStartZoneAndLevelEnabled\n");
}

TEST(ccapi_debug_test, testDbgVariableArgument)
{
    ccapi_init_error_t error;
    ccapi_start_t start = {0};
    int test_int = 33;
    float test_float = 1.67;
    char test_buffer[100];

    fill_start_structure_with_good_parameters(&start);

    start.debug.init_zones = ZONE_SEND_DATA;
    start.debug.init_level = LEVEL_INFO;

    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    sprintf(test_buffer, "hello testDbgVariableArgument int=%d, float=%f\n", test_int, test_float);

    Mock_ccimp_debug_expect(test_buffer);

    ccapi_debug_printf(ZONE_SEND_DATA, LEVEL_INFO, "hello testDbgVariableArgument int=%d, float=%f\n", test_int, test_float);
}

/* Testing Layer1 interface */
TEST(ccapi_debug_test, testDbgLayer1ZoneEnabled)
{
    ccapi_init_error_t error;
    ccapi_start_t start = {0};
    fill_start_structure_with_good_parameters(&start);

    start.debug.init_zones = ZONE_LAYER1;
    start.debug.init_level = LEVEL_INFO;

    Mock_ccimp_debug_expect("hello testDbgLayer1ZoneEnabled\n");

    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    connector_debug_printf("hello testDbgLayer1ZoneEnabled\n");
}

TEST(ccapi_debug_test, testDbgLayer1ZoneDisabled)
{
    ccapi_init_error_t error;
    ccapi_start_t start = {0};
    fill_start_structure_with_good_parameters(&start);

    start.debug.init_zones = ZONE_NONE;
    start.debug.init_level = LEVEL_INFO;

    /* Manually enable the mock to check that nothing is called */
    mock("ccimp_debug").setData("behavior", MOCK_DEBUG_ENABLED);

    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    connector_debug_printf("hello testDbgLayer1ZoneDisabled\n");
}

TEST(ccapi_debug_test, testDbgLayer1VariableArgument)
{
    ccapi_init_error_t error;
    ccapi_start_t start = {0};
    int test_int = 33;
    float test_float = 1.67;
    char test_buffer[100];

    fill_start_structure_with_good_parameters(&start);

    start.debug.init_zones = ZONE_LAYER1;
    start.debug.init_level = LEVEL_INFO;

    error = ccapi_start(&start);
    CHECK(error == CCAPI_INIT_ERROR_NONE);

    sprintf(test_buffer, "hello testDbgLayer1VariableArgument int=%d, float=%f\n", test_int, test_float);

    Mock_ccimp_debug_expect(test_buffer);

    connector_debug_printf("hello testDbgLayer1VariableArgument int=%d, float=%f\n", test_int, test_float);
}