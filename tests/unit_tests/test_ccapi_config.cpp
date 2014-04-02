#include "CppUTest/CommandLineTestRunner.h"
#include "mocks/mocks.h"

extern "C" {
#include "ccapi/ccapi.h"
#include "ccapi_definitions.h"
}

#include "test_helper_functions.h"

using namespace std;

static ccapi_data_t * * spy_ccapi_data = (ccapi_data_t * *) &ccapi_data_single_instance;

TEST_GROUP(ccapi_config_test_basic)
{
    mock_connector_api_info_t * mock_info; 
    ccapi_data_t * ccapi_data;

    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;

        Mock_create_all();

        mock_info = alloc_mock_connector_api_info();
        CHECK(mock_info != NULL);

        fill_start_structure_with_good_parameters(&start);
        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);

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

TEST(ccapi_config_test_basic, testDeviceID)
{
    connector_request_id_t request;
    connector_config_pointer_data_t device_id = {NULL, DEVICE_ID_LENGTH};


    request.config_request = connector_request_id_config_device_id;
    ccapi_connector_callback(connector_class_id_config, request, &device_id, (*spy_ccapi_data));
    CHECK_EQUAL(device_id.data, (*spy_ccapi_data)->config.device_id);
}

TEST(ccapi_config_test_basic, testCloudURL)
{
    connector_request_id_t request;
    connector_config_pointer_string_t device_cloud_url = {0};

    request.config_request = connector_request_id_config_device_cloud_url;
    ccapi_connector_callback(connector_class_id_config, request, &device_cloud_url, (*spy_ccapi_data));
    STRCMP_EQUAL(device_cloud_url.string, (*spy_ccapi_data)->config.device_cloud_url);
    CHECK(strlen((*spy_ccapi_data)->config.device_cloud_url) == device_cloud_url.length);
}

TEST(ccapi_config_test_basic, testVendorID)
{
    connector_request_id_t request;
    connector_config_vendor_id_t vendor_id = {0};

    request.config_request = connector_request_id_config_vendor_id;
    ccapi_connector_callback(connector_class_id_config, request, &vendor_id, (*spy_ccapi_data));
    CHECK(vendor_id.id == (*spy_ccapi_data)->config.vendor_id);
}

TEST(ccapi_config_test_basic, testDeviceType)
{
    connector_request_id_t request;
    connector_config_pointer_string_t device_type = {0};

    request.config_request = connector_request_id_config_device_type;
    ccapi_connector_callback(connector_class_id_config, request, &device_type, (*spy_ccapi_data));
    STRCMP_EQUAL(device_type.string, (*spy_ccapi_data)->config.device_type);
    CHECK(strlen((*spy_ccapi_data)->config.device_type) == device_type.length);
}

TEST(ccapi_config_test_basic, testFirmwareSupport)
{
    connector_request_id_t request;
    connector_config_supported_t firmware_supported = {connector_true}; /* Set to the opposite to test that it actually worked */

    request.config_request = connector_request_id_config_firmware_facility;
    ccapi_connector_callback(connector_class_id_config, request, &firmware_supported, (*spy_ccapi_data));
    CHECK(firmware_supported.supported == connector_false);
}

TEST(ccapi_config_test_basic, testFileSystemSupport)
{
    connector_request_id_t request;
    connector_config_supported_t filesystem_supported = {connector_true}; /* Set to the opposite to test that it actually worked */

    request.config_request = connector_request_id_config_file_system;
    ccapi_connector_callback(connector_class_id_config, request, &filesystem_supported, (*spy_ccapi_data));
    CHECK(filesystem_supported.supported == connector_false);
}

TEST(ccapi_config_test_basic, testRCISupport)
{
    connector_request_id_t request;
    connector_config_supported_t rci_supported = {connector_true}; /* Set to the opposite to test that it actually worked */

    request.config_request = connector_request_id_config_remote_configuration;
    ccapi_connector_callback(connector_class_id_config, request, &rci_supported, (*spy_ccapi_data));
    CHECK(rci_supported.supported == connector_false);
}

TEST(ccapi_config_test_basic, testDataServiceSupport)
{
    connector_request_id_t request;
    connector_config_supported_t dataservice_supported = {connector_false}; /* Set to the opposite to test that it actually worked */

    request.config_request = connector_request_id_config_data_service;
    ccapi_connector_callback(connector_class_id_config, request, &dataservice_supported, (*spy_ccapi_data));
    CHECK(dataservice_supported.supported == connector_true);
}

TEST_GROUP(ccapi_config_test_tcp_start_LAN_1)
{
    mock_connector_api_info_t * mock_info; 
    ccapi_data_t * ccapi_data;

    /* This groups starts with LAN and IPv4, No password */
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        ccapi_tcp_start_error_t tcp_start_error;
        ccapi_tcp_info_t tcp_start = {{0}};
        uint32_t ipv4 = 0xC0A80101; /* 192.168.1.1 */
        uint8_t mac[MAC_ADDR_LENGTH] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF}; /* 00049D:ABCDEF */
        connector_transport_t connector_transport = connector_transport_tcp;

        Mock_create_all();

        mock_info = alloc_mock_connector_api_info();
        CHECK(mock_info != NULL);

        fill_start_structure_with_good_parameters(&start);
        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);

        ccapi_data = *spy_ccapi_data;
        mock_info->ccapi_handle = (ccapi_handle_t)ccapi_data;
        mock_info->connector_handle = ccapi_data->connector_handle;

        tcp_start.connection.type = CCAPI_CONNECTION_LAN;
        tcp_start.connection.info.lan.ip.type = CCAPI_IPV4;
        tcp_start.connection.info.lan.ip.address.ipv4 = ipv4;
        memcpy(tcp_start.connection.info.lan.mac_address, mac, sizeof mac);

        tcp_start.callback.close = NULL;
        tcp_start.callback.keepalive = NULL;

        Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_transport_start, &connector_transport,connector_success);
        tcp_start_error = ccapi_start_transport_tcp(&tcp_start);
        CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, tcp_start_error);
    }

    void teardown()
    {
        ccapi_stop_error_t stop_error;

        Mock_connector_initiate_action_expectAndReturn((*spy_ccapi_data)->connector_handle, connector_initiate_terminate, NULL, connector_success);
        stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
        CHECK_EQUAL(CCAPI_STOP_ERROR_NONE, stop_error);

        free_mock_connector_api_info(mock_info);

        Mock_destroy_all();
    }
};

TEST(ccapi_config_test_tcp_start_LAN_1, testConfigConnectionType)
{
    connector_request_id_t request;
    connector_config_connection_type_t connection_type = { connector_connection_type_wan };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_connection_type;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &connection_type, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(connection_type.type, connector_connection_type_lan);
}

TEST(ccapi_config_test_tcp_start_LAN_1, testConfigMAC)
{
    connector_request_id_t request;
    connector_config_pointer_data_t connector_mac_addr = {NULL, MAC_ADDR_LENGTH};
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_mac_addr;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &connector_mac_addr, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(0, memcmp(connector_mac_addr.data, (*spy_ccapi_data)->transport.tcp->connection.info.lan.mac_address, MAC_ADDR_LENGTH));
}

TEST(ccapi_config_test_tcp_start_LAN_1, testConfigIPv4)
{
    connector_request_id_t request;
    connector_config_ip_address_t connector_ip_addr = {NULL, connector_ip_address_ipv6};
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_ip_addr;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &connector_ip_addr, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(connector_ip_address_ipv4, connector_ip_addr.ip_address_type);
    CHECK_EQUAL(0, memcmp(connector_ip_addr.address, &(*spy_ccapi_data)->transport.tcp->connection.info.lan.ip.address.ipv4, IPV4_LENGTH));
}

TEST(ccapi_config_test_tcp_start_LAN_1, testIdVerificationSimple)
{
    connector_request_id_t request;
    connector_config_identity_verification_t connector_id_verification = {connector_identity_verification_password};
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_identity_verification;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &connector_id_verification, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(connector_identity_verification_simple, connector_id_verification.type);
}

TEST_GROUP(ccapi_config_test_tcp_start_LAN_2)
{
    mock_connector_api_info_t * mock_info; 
    ccapi_data_t * ccapi_data;

    /* This groups starts with LAN and IPv6, Password enabled, max transactions = 10 and Keepalives RX=90, TX=100, WC=10 */
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        ccapi_tcp_start_error_t tcp_error;
        ccapi_tcp_info_t tcp_start = {{0}};
        uint8_t ipv6[IPV6_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0xFE, 0x80, 0x00, 0x00, 0x02, 0x25, 0x64, 0xFF, 0xFE, 0x9B, 0xAF, 0x03}; /* fe80::225:64ff:fe9b:af03 */
        uint8_t mac[MAC_ADDR_LENGTH] = {0x00, 0x04, 0x9D, 0xAB, 0xCD, 0xEF}; /* 00049D:ABCDEF */
        connector_transport_t connector_transport = connector_transport_tcp;
        char password[] = "CCAPI Rules!";

        Mock_create_all();

        mock_info = alloc_mock_connector_api_info();
        CHECK(mock_info != NULL);

        fill_start_structure_with_good_parameters(&start);
        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);

        ccapi_data = *spy_ccapi_data;
        mock_info->ccapi_handle = (ccapi_handle_t)ccapi_data;
        mock_info->connector_handle = ccapi_data->connector_handle;

        tcp_start.keepalives.rx = 90;
        tcp_start.keepalives.tx = 100;
        tcp_start.keepalives.wait_count = 10;

        tcp_start.connection.password = password;
        tcp_start.connection.max_transactions = 10;
        tcp_start.connection.type = CCAPI_CONNECTION_LAN;
        tcp_start.connection.info.lan.ip.type = CCAPI_IPV6;
        memcpy(tcp_start.connection.info.lan.ip.address.ipv6, ipv6, sizeof ipv6);
        memcpy(tcp_start.connection.info.lan.mac_address, mac, sizeof mac);

        tcp_start.callback.close = NULL;
        tcp_start.callback.keepalive = NULL;

        Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_transport_start, &connector_transport,connector_success);
        tcp_error = ccapi_start_transport_tcp(&tcp_start);
        CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, tcp_error);
    }

    void teardown()
    {
        ccapi_stop_error_t stop_error;

        Mock_connector_initiate_action_expectAndReturn((*spy_ccapi_data)->connector_handle, connector_initiate_terminate, NULL, connector_success);
        stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
        CHECK_EQUAL(CCAPI_STOP_ERROR_NONE, stop_error);

        free_mock_connector_api_info(mock_info);

        Mock_destroy_all();
    }
};

TEST(ccapi_config_test_tcp_start_LAN_2, testConfigIPv6)
{
    connector_request_id_t request;
    connector_config_ip_address_t connector_ip_addr = {NULL, connector_ip_address_ipv4};
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_ip_addr;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &connector_ip_addr, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(connector_ip_address_ipv6, connector_ip_addr.ip_address_type);
    CHECK_EQUAL(0, memcmp(connector_ip_addr.address, &(*spy_ccapi_data)->transport.tcp->connection.info.lan.ip.address.ipv6, IPV6_LENGTH));
}

TEST(ccapi_config_test_tcp_start_LAN_2, testIdVerificationPassword)
{
    connector_request_id_t request;
    connector_config_identity_verification_t connector_id_verification = {connector_identity_verification_simple};
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_identity_verification;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &connector_id_verification, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(connector_identity_verification_password, connector_id_verification.type);
}

TEST(ccapi_config_test_tcp_start_LAN_2, testPassword)
{
    connector_request_id_t request;
    connector_config_pointer_string_t password = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_password;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &password, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(strlen((*spy_ccapi_data)->transport.tcp->connection.password), password.length);
    CHECK_EQUAL((*spy_ccapi_data)->transport.tcp->connection.password, password.string);
    STRCMP_EQUAL((*spy_ccapi_data)->transport.tcp->connection.password, password.string);
}

TEST(ccapi_config_test_tcp_start_LAN_2, testMaxTransactions)
{
    connector_request_id_t request;
    connector_config_max_transaction_t max_transaction = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_max_transaction;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &max_transaction, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL((*spy_ccapi_data)->transport.tcp->connection.max_transactions, max_transaction.count);
}

TEST(ccapi_config_test_tcp_start_LAN_2, testRxKeepalives)
{
    connector_request_id_t request;
    connector_config_keepalive_t rx_keepalive = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_rx_keepalive;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &rx_keepalive, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL((*spy_ccapi_data)->transport.tcp->keepalives.rx, rx_keepalive.interval_in_seconds);
}

TEST(ccapi_config_test_tcp_start_LAN_2, testTxKeepalives)
{
    connector_request_id_t request;
    connector_config_keepalive_t tx_keepalive = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_tx_keepalive;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &tx_keepalive, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL((*spy_ccapi_data)->transport.tcp->keepalives.tx, tx_keepalive.interval_in_seconds);
}

TEST(ccapi_config_test_tcp_start_LAN_2, testWcKeepalives)
{
    connector_request_id_t request;
    connector_config_wait_count_t wait_count = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_wait_count;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &wait_count, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL((*spy_ccapi_data)->transport.tcp->keepalives.wait_count, wait_count.count);
}

TEST_GROUP(ccapi_config_test_tcp_start_WAN)
{
    mock_connector_api_info_t * mock_info; 
    ccapi_data_t * ccapi_data;

    /* This groups starts with WAN, linkspeed = 1000 and phone number != "", password disabled, max transactions = 10 and Keepalives RX=90, TX=100, WC=10 */
    void setup()
    {
        ccapi_start_t start = {0};
        ccapi_start_error_t error;
        ccapi_tcp_start_error_t tcp_error;
        ccapi_tcp_info_t tcp_start = {{0}};
        connector_transport_t connector_transport = connector_transport_tcp;
        char phone_number[] = "+34 941 27 00 60";
        Mock_create_all();

        mock_info = alloc_mock_connector_api_info();
        CHECK(mock_info != NULL);

        fill_start_structure_with_good_parameters(&start);

        error = ccapi_start(&start);
        CHECK(error == CCAPI_START_ERROR_NONE);

        ccapi_data = *spy_ccapi_data;
        mock_info->ccapi_handle = (ccapi_handle_t)ccapi_data;
        mock_info->connector_handle = ccapi_data->connector_handle;

        tcp_start.keepalives.rx = 90;
        tcp_start.keepalives.tx = 100;
        tcp_start.keepalives.wait_count = 10;

        tcp_start.connection.max_transactions = 10;
        tcp_start.connection.type = CCAPI_CONNECTION_WAN;
        tcp_start.connection.info.wan.link_speed = 1000;
        tcp_start.connection.info.wan.phone_number = phone_number;

        tcp_start.callback.close = NULL;
        tcp_start.callback.keepalive = NULL;

        Mock_connector_initiate_action_expectAndReturn(ccapi_data->connector_handle, connector_initiate_transport_start, &connector_transport,connector_success);
        tcp_error = ccapi_start_transport_tcp(&tcp_start);
        CHECK_EQUAL(CCAPI_TCP_START_ERROR_NONE, tcp_error);
    }

    void teardown()
    {
        ccapi_stop_error_t stop_error;

        Mock_connector_initiate_action_expectAndReturn((*spy_ccapi_data)->connector_handle, connector_initiate_terminate, NULL, connector_success);
        stop_error = ccapi_stop(CCAPI_STOP_IMMEDIATELY);
        CHECK_EQUAL(CCAPI_STOP_ERROR_NONE, stop_error);

        free_mock_connector_api_info(mock_info);

        Mock_destroy_all();
    }
};

TEST(ccapi_config_test_tcp_start_WAN, testLinkSpeed)
{
    connector_request_id_t request;
    connector_config_link_speed_t link_speed = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_link_speed;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &link_speed, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL((*spy_ccapi_data)->transport.tcp->connection.info.wan.link_speed, link_speed.speed);
}

TEST(ccapi_config_test_tcp_start_WAN, testPhoneNumber)
{
    connector_request_id_t request;
    connector_config_pointer_string_t phone_number = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_phone_number;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &phone_number, (*spy_ccapi_data));
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(strlen((*spy_ccapi_data)->transport.tcp->connection.info.wan.phone_number), phone_number.length);
    CHECK_EQUAL((*spy_ccapi_data)->transport.tcp->connection.info.wan.phone_number, phone_number.string);
    STRCMP_EQUAL((*spy_ccapi_data)->transport.tcp->connection.info.wan.phone_number, phone_number.string);
}
