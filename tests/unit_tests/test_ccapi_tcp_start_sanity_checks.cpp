/*
 * test_ccapi_stop.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: spastor
 */

#include "CppUTest/CommandLineTestRunner.h"

#define CCAPI_CONST_PROTECTION_UNLOCK

#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
}

#include "test_helper_functions.h"

static ccapi_data_t * * spy_ccapi_data = (ccapi_data_t * *) &ccapi_data_single_instance;

/* This group doesn't call ccapi_start/stop functions */
TEST_GROUP(ccapi_tcp_start_with_no_ccapi) {};
TEST(ccapi_tcp_start_with_no_ccapi, testNotStarted)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};

    IGNORE_ALL_LEAKS_IN_TEST(); /* TODO: if CCAPI is not started it detects memory leaks */

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_CCAPI_STOPPED, error);
}

TEST_GROUP(ccapi_tcp_start_sanity_checks_test)
{
    mock_connector_api_info_t * mock_info; 
    ccapi_data_t * ccapi_data;

    void setup()
    {
        ccapi_start_error_t start_error;
        ccapi_start_t start;

        Mock_create_all();

        mock_info = alloc_mock_connector_api_info();
        CHECK(mock_info != NULL);

        fill_start_structure_with_good_parameters(&start);
        start_error = ccapi_start(&start);
        CHECK_EQUAL(CCAPI_START_ERROR_NONE, start_error);

        ccapi_data = *spy_ccapi_data;
        mock_info->ccapi_handle = (ccapi_handle_t)ccapi_data;
        mock_info->connector_handle = ccapi_data->connector_handle;
    }

    void teardown()
    {
        ccapi_stop_error_t stop_error;

        Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_terminate, NULL, connector_success);
        stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
        CHECK_EQUAL(CCAPI_STOP_ERROR_NONE, stop_error);

        free_mock_connector_api_info(mock_info);

        Mock_destroy_all();
    }
};

TEST(ccapi_tcp_start_sanity_checks_test, testNullPointer)
{
    ccapi_tcp_start_error_t error;

    error = ccapi_start_transport_tcp(NULL);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_NULL_POINTER, error);
}

TEST(ccapi_tcp_start_sanity_checks_test, testBadKeepalivesRx)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};

    tcp_start.keepalives.rx = CCAPI_KEEPALIVES_RX_MAX + 1;
    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_KEEPALIVES, error);

    tcp_start.keepalives.rx = CCAPI_KEEPALIVES_RX_MIN - 1;
    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_KEEPALIVES, error);
}

TEST(ccapi_tcp_start_sanity_checks_test, testBadKeepalivesTx)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};

    tcp_start.keepalives.tx = CCAPI_KEEPALIVES_TX_MAX + 1;
    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_KEEPALIVES, error);

    tcp_start.keepalives.tx = CCAPI_KEEPALIVES_TX_MIN - 1;
    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_KEEPALIVES, error);
}

TEST(ccapi_tcp_start_sanity_checks_test, testBadKeepalivesWaitCount)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};

    tcp_start.keepalives.wait_count = CCAPI_KEEPALIVES_WCNT_MAX + 1;
    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_KEEPALIVES, error);

    tcp_start.keepalives.wait_count = CCAPI_KEEPALIVES_WCNT_MIN - 1;
    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_KEEPALIVES, error);
}

TEST(ccapi_tcp_start_sanity_checks_test, testBadIP)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    uint8_t mac[] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF}; /* 00049D:ABCDEF */

    tcp_start.connection.type = CCAPI_CONNECTION_LAN;
    tcp_start.connection.info.lan.ip.type = CCAPI_IPV4;
    tcp_start.connection.info.lan.ip.address.ipv4 = 0;
    memcpy(tcp_start.connection.info.lan.mac_address, mac, sizeof tcp_start.connection.info.lan.mac_address);

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_IP, error);

    tcp_start.connection.info.lan.ip.type = CCAPI_IPV6;
    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_IP, error);
}

TEST(ccapi_tcp_start_sanity_checks_test, testLANIpv4)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    uint32_t ipv4 = 0xC0A80101; /* 192.168.1.1 */
    uint8_t mac[] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF}; /* 00049D:ABCDEF */

    tcp_start.connection.type = CCAPI_CONNECTION_LAN;
    tcp_start.connection.info.lan.ip.type = CCAPI_IPV4;
    tcp_start.connection.info.lan.ip.address.ipv4 = ipv4;
    memcpy(tcp_start.connection.info.lan.mac_address, mac, sizeof tcp_start.connection.info.lan.mac_address);

    connector_transport_t connector_transport = connector_transport_tcp;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, error);
    CHECK(ipv4 == (*spy_ccapi_data)->transport.tcp->connection.info.lan.ip.address.ipv4);
    CHECK(memcmp(mac, (*spy_ccapi_data)->transport.tcp->connection.info.lan.mac_address, sizeof mac) == 0);
}

TEST(ccapi_tcp_start_sanity_checks_test, testLANIpv6)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    uint8_t ipv6[] = {0x00, 0x00, 0x00, 0x00, 0xFE, 0x80, 0x00, 0x00, 0x02, 0x25, 0x64, 0xFF, 0xFE, 0x9B, 0xAF, 0x03}; /* fe80::225:64ff:fe9b:af03 */
    uint8_t mac[] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF}; /* 00049D:ABCDEF */

    tcp_start.connection.type = CCAPI_CONNECTION_LAN;
    tcp_start.connection.info.lan.ip.type = CCAPI_IPV6;
    memcpy(tcp_start.connection.info.lan.ip.address.ipv6, ipv6, sizeof tcp_start.connection.info.lan.ip.address.ipv6);
    memcpy(tcp_start.connection.info.lan.mac_address, mac, sizeof tcp_start.connection.info.lan.mac_address);

    connector_transport_t connector_transport = connector_transport_tcp;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, error);
    CHECK(memcmp(ipv6, (*spy_ccapi_data)->transport.tcp->connection.info.lan.ip.address.ipv6, sizeof ipv6) == 0);
    CHECK(memcmp(mac, (*spy_ccapi_data)->transport.tcp->connection.info.lan.mac_address, sizeof mac) == 0);
}

TEST(ccapi_tcp_start_sanity_checks_test, testLANZeroMAC)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    uint8_t mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; /* 000000:000000 */
    uint32_t ipv4 = 0xC0A80101; /* 192.168.1.1 */

    tcp_start.connection.type = CCAPI_CONNECTION_LAN;
    tcp_start.connection.info.lan.ip.type = CCAPI_IPV4;
    tcp_start.connection.info.lan.ip.address.ipv4 = ipv4;
    memcpy(tcp_start.connection.info.lan.mac_address, mac, sizeof tcp_start.connection.info.lan.mac_address);

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_INVALID_MAC, error);
}

TEST(ccapi_tcp_start_sanity_checks_test, testPassword)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    uint8_t mac[] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF}; /* 00049D:ABCDEF */
    uint32_t ipv4 = 0xC0A80101; /* 192.168.1.1 */
    char password[] = "Hello, World!";

    tcp_start.connection.password = password;
    tcp_start.connection.type = CCAPI_CONNECTION_LAN;
    tcp_start.connection.info.lan.ip.type = CCAPI_IPV4;
    tcp_start.connection.info.lan.ip.address.ipv4 = ipv4;
    memcpy(tcp_start.connection.info.lan.mac_address, mac, sizeof tcp_start.connection.info.lan.mac_address);

    connector_transport_t connector_transport = connector_transport_tcp;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, error);
    /* If both pointers are the same, then ccapi_data is holding a pointer to a stack variable */
    CHECK(tcp_start.connection.password != (*spy_ccapi_data)->transport.tcp->connection.password);
    STRCMP_EQUAL(tcp_start.connection.password, (*spy_ccapi_data)->transport.tcp->connection.password);
}

TEST(ccapi_tcp_start_sanity_checks_test, testWAN)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    char phone_number[] = "+54-3644-421921";

    tcp_start.connection.type = CCAPI_CONNECTION_WAN;
    tcp_start.connection.info.wan.phone_number = phone_number;
    tcp_start.connection.info.wan.link_speed = 115200;

    connector_transport_t connector_transport = connector_transport_tcp;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, error);
    CHECK(tcp_start.connection.info.wan.link_speed == (*spy_ccapi_data)->transport.tcp->connection.info.wan.link_speed);
    /* If both pointers are the same, then ccapi_data is holding a pointer to a stack variable */
    CHECK(tcp_start.connection.info.wan.phone_number != (*spy_ccapi_data)->transport.tcp->connection.info.wan.phone_number);
    STRCMP_EQUAL(tcp_start.connection.info.wan.phone_number, (*spy_ccapi_data)->transport.tcp->connection.info.wan.phone_number);
}

TEST(ccapi_tcp_start_sanity_checks_test, testWANEmptyPhone)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    char phone_number[] = "";

    tcp_start.connection.type = CCAPI_CONNECTION_WAN;
    tcp_start.connection.info.wan.phone_number = phone_number;
    tcp_start.connection.info.wan.link_speed = 115200;

    connector_transport_t connector_transport = connector_transport_tcp;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, error);
    CHECK(tcp_start.connection.info.wan.link_speed == (*spy_ccapi_data)->transport.tcp->connection.info.wan.link_speed);
    /* If both pointers are the same, then ccapi_data is holding a pointer to a stack variable */
    CHECK(tcp_start.connection.info.wan.phone_number != (*spy_ccapi_data)->transport.tcp->connection.info.wan.phone_number);
    STRCMP_EQUAL(tcp_start.connection.info.wan.phone_number, (*spy_ccapi_data)->transport.tcp->connection.info.wan.phone_number);
}

TEST(ccapi_tcp_start_sanity_checks_test, testWANPhoneNull)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};

    tcp_start.connection.type = CCAPI_CONNECTION_WAN;
    tcp_start.connection.info.wan.phone_number = NULL;
    tcp_start.connection.info.wan.link_speed = 115200;

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_PHONE, error);
}

TEST(ccapi_tcp_start_sanity_checks_test, testKeepaliveDefaults)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    char phone_number[] = "+54-3644-421921";

    tcp_start.connection.type = CCAPI_CONNECTION_WAN;
    tcp_start.connection.info.wan.phone_number = phone_number;
    tcp_start.connection.info.wan.link_speed = 115200;

    connector_transport_t connector_transport = connector_transport_tcp;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, error);
    CHECK_EQUAL(tcp_start.connection.type, (*spy_ccapi_data)->transport.tcp->connection.type);
    CHECK_EQUAL(CCAPI_KEEPALIVES_RX_DEFAULT, (*spy_ccapi_data)->transport.tcp->keepalives.rx);
    CHECK_EQUAL(CCAPI_KEEPALIVES_TX_DEFAULT, (*spy_ccapi_data)->transport.tcp->keepalives.tx);
    CHECK_EQUAL(CCAPI_KEEPALIVES_WCNT_DEFAULT, (*spy_ccapi_data)->transport.tcp->keepalives.wait_count);
}

TEST(ccapi_tcp_start_sanity_checks_test, testMaxSessions)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    char phone_number[] = "+54-3644-421921";

    tcp_start.connection.type = CCAPI_CONNECTION_WAN;
    tcp_start.connection.info.wan.phone_number = phone_number;
    tcp_start.connection.info.wan.link_speed = 115200;
    tcp_start.connection.max_transactions = 10;

    connector_transport_t connector_transport = connector_transport_tcp;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, error);
    CHECK_EQUAL(tcp_start.connection.max_transactions, (*spy_ccapi_data)->transport.tcp->connection.max_transactions);
}

TEST(ccapi_tcp_start_sanity_checks_test, testTcpInfoNoMemory)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    void * malloc_for_ccapi_tcp = NULL;

    Mock_ccimp_malloc_expectAndReturn(sizeof (ccapi_tcp_info_t), malloc_for_ccapi_tcp);
    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_INSUFFICIENT_MEMORY, error);
}

TEST(ccapi_tcp_start_sanity_checks_test, testPasswordNoMemory)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    uint8_t mac[] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF}; /* 00049D:ABCDEF */
    uint32_t ipv4 = 0xC0A80101; /* 192.168.1.1 */
    char password[] = "Hello, World!";
    void * malloc_for_ccapi_tcp = malloc(sizeof (ccapi_tcp_info_t));
    void * malloc_for_password = NULL;

    tcp_start.connection.password = password;
    tcp_start.connection.type = CCAPI_CONNECTION_LAN;
    tcp_start.connection.info.lan.ip.type = CCAPI_IPV4;
    tcp_start.connection.info.lan.ip.address.ipv4 = ipv4;
    memcpy(tcp_start.connection.info.lan.mac_address, mac, sizeof tcp_start.connection.info.lan.mac_address);

    connector_transport_t connector_transport = connector_transport_tcp;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    Mock_ccimp_malloc_expectAndReturn(sizeof (ccapi_tcp_info_t), malloc_for_ccapi_tcp);
    Mock_ccimp_malloc_expectAndReturn(sizeof password, malloc_for_password);
    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_INSUFFICIENT_MEMORY, error);
}

TEST(ccapi_tcp_start_sanity_checks_test, testPhoneNoMemory)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    char phone_number[] = "+54-3644-421921";
    void * malloc_for_ccapi_tcp = malloc(sizeof (ccapi_tcp_info_t));
    void * malloc_for_phone = NULL;

    tcp_start.connection.password = NULL;
    tcp_start.connection.type = CCAPI_CONNECTION_WAN;
    tcp_start.connection.info.wan.phone_number = phone_number;
    tcp_start.connection.info.wan.link_speed = 115200;

    connector_transport_t connector_transport = connector_transport_tcp;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    Mock_ccimp_malloc_expectAndReturn(sizeof (ccapi_tcp_info_t), malloc_for_ccapi_tcp);
    Mock_ccimp_malloc_expectAndReturn(sizeof phone_number, malloc_for_phone);
    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_INSUFFICIENT_MEMORY, error);
}

static ccapi_bool_t ccapi_tcp_close_cb(void)
{
    return CCAPI_TRUE;
}

static void ccapi_tcp_keepalives_cb(ccapi_keepalive_status_t status)
{
    UNUSED_ARGUMENT(status);
    return;
}

TEST(ccapi_tcp_start_sanity_checks_test, testCallbacksAreCopied)
{
    ccapi_tcp_start_error_t error;
    ccapi_tcp_info_t tcp_start = {{0}};
    char phone_number[] = "+54-3644-421921";

    tcp_start.connection.type = CCAPI_CONNECTION_WAN;
    tcp_start.connection.info.wan.phone_number = phone_number;
    tcp_start.connection.info.wan.link_speed = 115200;

    tcp_start.callback.close = ccapi_tcp_close_cb;
    tcp_start.callback.keepalive = ccapi_tcp_keepalives_cb;

    connector_transport_t connector_transport = connector_transport_tcp;
    Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_transport_start, &connector_transport, connector_success);

    error = ccapi_start_transport_tcp(&tcp_start);
    CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, error);
    CHECK_EQUAL(tcp_start.callback.close, (*spy_ccapi_data)->transport.tcp->callback.close);
    CHECK_EQUAL(tcp_start.callback.keepalive, (*spy_ccapi_data)->transport.tcp->callback.keepalive);
}
