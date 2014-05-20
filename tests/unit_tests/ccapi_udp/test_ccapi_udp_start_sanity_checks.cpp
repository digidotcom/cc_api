#include "test_helper_functions.h"

/* This group doesn't call ccapi_start/stop functions */
TEST_GROUP(ccapi_udp_start_with_no_ccapi) {};
TEST(ccapi_udp_start_with_no_ccapi, testNotStarted)
{
    ccapi_udp_start_error_t error;
    ccapi_udp_info_t udp_start = {{}};
    IGNORE_ALL_LEAKS_IN_TEST(); /* TODO: if CCAPI is not started it detects memory leaks */
    error = ccapi_start_transport_udp(&udp_start);

    CHECK_EQUAL(CCAPI_UDP_START_ERROR_CCAPI_STOPPED, error);
}
TEST_GROUP(test_ccapi_udp_start_sanity_checks)
{
    void setup()
    {
        Mock_create_all();

        th_start_ccapi();
    }

    void teardown()
    {
        th_stop_ccapi(ccapi_data_single_instance);

        Mock_destroy_all();
    }
};

static ccapi_bool_t ccapi_udp_close_cb(ccapi_udp_close_cause_t cause)
{
    UNUSED_ARGUMENT(cause);
    return CCAPI_TRUE;
}

TEST(test_ccapi_udp_start_sanity_checks, testNullPointer)
{
    ccapi_udp_start_error_t error;

    error = ccapi_start_transport_udp(NULL);
    CHECK_EQUAL(CCAPI_UDP_START_ERROR_NULL_POINTER, error);
}

TEST(test_ccapi_udp_start_sanity_checks, testDefaults)
{
    ccapi_udp_start_error_t error;
    ccapi_udp_info_t udp_start = {{0}};

    connector_transport_t connector_transport = connector_transport_udp;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);
    error = ccapi_start_transport_udp(&udp_start);
    CHECK_EQUAL(CCAPI_UDP_START_ERROR_NONE, error);
    CHECK_EQUAL(CCAPI_SM_UDP_MAX_SESSIONS_DEFAULT, ccapi_data_single_instance->transport_udp.info->limit.max_sessions);
}

TEST(test_ccapi_udp_start_sanity_checks, testMaxSessions)
{
    ccapi_udp_start_error_t error;
    ccapi_udp_info_t udp_start = {{0}};
    udp_start.limit.max_sessions=300;

    error = ccapi_start_transport_udp(&udp_start);
    CHECK_EQUAL(CCAPI_UDP_START_ERROR_MAX_SESSIONS, error);
}


TEST(test_ccapi_udp_start_sanity_checks, testUdpInfoNoMemory)
{
    ccapi_udp_start_error_t error;
    ccapi_udp_info_t udp_start = {{0}};
    void * malloc_for_ccapi_udp = NULL;

    Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_udp_info_t), malloc_for_ccapi_udp);
    error = ccapi_start_transport_udp(&udp_start);
    CHECK_EQUAL(CCAPI_UDP_START_ERROR_INSUFFICIENT_MEMORY, error);

}

TEST(test_ccapi_udp_start_sanity_checks, testInfoIsCopied)
{
    ccapi_udp_start_error_t error;
    ccapi_udp_info_t udp_start = {{0}};
    udp_start.limit.max_sessions=140;
    udp_start.limit.rx_timeout=0;
    udp_start.callback.close=ccapi_udp_close_cb;

    connector_transport_t connector_transport = connector_transport_udp;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data_single_instance->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    error = ccapi_start_transport_udp(&udp_start);
    CHECK_EQUAL(CCAPI_UDP_START_ERROR_NONE, error);
    CHECK_EQUAL(udp_start.callback.close, ccapi_data_single_instance->transport_udp.info->callback.close);
    CHECK_EQUAL(udp_start.limit.max_sessions, ccapi_data_single_instance->transport_udp.info->limit.max_sessions);
    CHECK_EQUAL(udp_start.limit.rx_timeout, ccapi_data_single_instance->transport_udp.info->limit.rx_timeout);
}
