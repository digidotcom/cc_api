/*
* Copyright (c) 2014 Etherios, a Division of Digi International, Inc.
* All rights not expressly granted are reserved.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Etherios 11001 Bren Road East, Minnetonka, MN 55343
* =======================================================================
*/

#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_config_tcp_start_wan)
{
    /* This groups starts with WAN, linkspeed = 1000 and phone number != "", password disabled, max transactions = 10 and Keepalives RX=90, TX=100, WC=10 */
    void setup()
    {
        Mock_create_all();

        th_start_ccapi();
        th_start_tcp_wan_ipv4_with_callbacks();
    }

    void teardown()
    {
        th_stop_ccapi(ccapi_data_single_instance);

        Mock_destroy_all();
    }
};

TEST(test_ccapi_config_tcp_start_wan, testLinkSpeed)
{
    connector_request_id_t request;
    connector_config_link_speed_t link_speed = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_link_speed;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &link_speed, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(ccapi_data_single_instance->transport_tcp.info->connection.info.wan.link_speed, link_speed.speed);
}

TEST(test_ccapi_config_tcp_start_wan, testPhoneNumber)
{
    connector_request_id_t request;
    connector_config_pointer_string_t phone_number = { 0 };
    connector_callback_status_t callback_status;

    request.config_request = connector_request_id_config_phone_number;
    callback_status = ccapi_connector_callback(connector_class_id_config, request, &phone_number, ccapi_data_single_instance);
    CHECK_EQUAL(connector_callback_continue, callback_status);
    CHECK_EQUAL(strlen(ccapi_data_single_instance->transport_tcp.info->connection.info.wan.phone_number), phone_number.length);
    CHECK_EQUAL(ccapi_data_single_instance->transport_tcp.info->connection.info.wan.phone_number, phone_number.string);
    STRCMP_EQUAL(ccapi_data_single_instance->transport_tcp.info->connection.info.wan.phone_number, phone_number.string);
}
