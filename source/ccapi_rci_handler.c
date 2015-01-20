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

#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

#if (defined CCIMP_RCI_SERVICE_ENABLED)

static ccapi_rci_query_setting_attribute_compare_to_t connector_to_ccapi_compare_to_attribute(rci_query_setting_attribute_compare_to_t const compare_to)
{
    ccapi_rci_query_setting_attribute_compare_to_t retval;

    switch (compare_to)
    {
        case rci_query_setting_attribute_compare_to_none:
            retval = CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_COMPARE_TO_NONE;
            break;
        case rci_query_setting_attribute_compare_to_current:
            retval = CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_COMPARE_TO_CURRENT;
            break;
        case rci_query_setting_attribute_compare_to_stored:
            retval = CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_COMPARE_TO_STORED;
            break;
        case rci_query_setting_attribute_compare_to_defaults:
            retval = CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_COMPARE_TO_DEFAULTS;
            break;
    }

    return retval;
}

static ccapi_rci_query_setting_attribute_source_t connector_to_ccapi_source_attribute(rci_query_setting_attribute_source_t const source)
{
    ccapi_rci_query_setting_attribute_source_t retval;

    switch (source)
    {
        case rci_query_setting_attribute_source_current:
            retval = CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_SOURCE_CURRENT;
            break;
        case rci_query_setting_attribute_source_stored:
            retval = CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_SOURCE_STORED;
            break;
        case rci_query_setting_attribute_source_defaults:
            retval = CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_SOURCE_DEFAULTS;
            break;
    }

    return retval;
}

connector_callback_status_t ccapi_rci_handler(connector_request_id_remote_config_t const request_id, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t status;
    connector_remote_config_t * const remote_config = data;
    ccapi_rci_data_t const * const rci_data = ccapi_data->service.rci.rci_data;
    ccapi_rci_info_t * const rci_info = &ccapi_data->service.rci.rci_info;
    unsigned int error;

    rci_info->user_context = remote_config->user_context;
    switch (request_id)
    {
        case connector_request_id_remote_config_session_start:
        {
            ccapi_rci_function_t const session_start_cb = rci_data->callbacks.start_session;

            rci_info->action = CCAPI_RCI_ACTION_QUERY;
            rci_info->error_hint = NULL;
            rci_info->group.instance = 0;
            rci_info->group.type = CCAPI_RCI_GROUP_SETTING;
            rci_info->query_setting.matches = CCAPI_FALSE;
            rci_info->query_setting.attributes.compare_to = CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_COMPARE_TO_NONE;
            rci_info->query_setting.attributes.source = CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_SOURCE_CURRENT;
            error = session_start_cb(rci_info);
            break;
        }
        case connector_request_id_remote_config_action_start:
        {
            connector_remote_group_type_t const group_type = remote_config->group.type;
            ccapi_rci_function_t const action_start_cb = rci_data->callbacks.start_action;

            switch (remote_config->action)
            {
                case connector_remote_action_set:
                    rci_info->action = CCAPI_RCI_ACTION_SET;
                    break;
                case connector_remote_action_query:
                    rci_info->action = CCAPI_RCI_ACTION_QUERY;
                    break;
            }

            switch (group_type)
            {
                case connector_remote_group_setting:
                    rci_info->group.type = CCAPI_RCI_GROUP_SETTING;
                    break;
                case connector_remote_group_state:
                    rci_info->group.type = CCAPI_RCI_GROUP_STATE;
                    break;
            }

            if (rci_info->action == CCAPI_RCI_ACTION_QUERY && rci_info->group.type == CCAPI_RCI_GROUP_SETTING)
            {
                rci_info->query_setting.attributes.compare_to = connector_to_ccapi_compare_to_attribute(remote_config->attribute.compare_to);
                rci_info->query_setting.attributes.source = connector_to_ccapi_source_attribute(remote_config->attribute.source);
                rci_info->query_setting.matches = CCAPI_FALSE;
            }
#if (defined RCI_PARSER_USES_GROUP_NAMES)
            rci_info->group.name = NULL;
#endif
#if (defined RCI_PARSER_USES_ELEMENT_NAMES)
            rci_info->element.name = NULL;
#endif
            error = action_start_cb(rci_info);
            break;
        }
        case connector_request_id_remote_config_group_start:
        {
            connector_remote_group_type_t const group_type = remote_config->group.type;
            unsigned int const group_id = remote_config->group.id;
            ccapi_rci_group_table_t const * group_table;
            ccapi_rci_group_t const * group;
            ccapi_rci_function_t start_callback;

            switch (group_type)
            {
                case connector_remote_group_setting:
                    group_table = &rci_data->settings;
                    break;
                case connector_remote_group_state:
                    group_table = &rci_data->state;
                    break;
            }
            rci_info->group.instance = remote_config->group.index;
#if (defined RCI_PARSER_USES_GROUP_NAMES)
            rci_info->group.name = remote_config->group.name;
#endif
            ASSERT(group_id < group_table->count);
            group = &group_table->groups[group_id];
            start_callback = group->callbacks.start;
            error = start_callback(rci_info);

            if (rci_info->group.type == CCAPI_RCI_GROUP_SETTING && rci_info->action == CCAPI_RCI_ACTION_QUERY)
            {
                remote_config->response.compare_matches = CCAPI_BOOL_TO_CONNECTOR_BOOL(rci_info->query_setting.matches);
            }
            break;
        }
        case connector_request_id_remote_config_group_process:
        {
            connector_remote_group_type_t const group_type = remote_config->group.type;
            unsigned int const group_id = remote_config->group.id;
            unsigned int const element_id = remote_config->element.id;
            ccapi_rci_group_table_t const * group_table;
            ccapi_rci_group_t const * group;
            ccapi_rci_element_t const * element;

            switch (group_type)
            {
                case connector_remote_group_setting:
                    rci_info->group.type = CCAPI_RCI_GROUP_SETTING;
                    group_table = &rci_data->settings;
                    break;
                case connector_remote_group_state:
                    rci_info->action = CCAPI_RCI_GROUP_STATE;
                    group_table = &rci_data->state;
                    break;
            }

            rci_info->group.instance = remote_config->group.index;
            ASSERT(group_id < group_table->count);
            group = &group_table->groups[group_id];

            ASSERT(element_id < group->count);
            element = &group->elements[element_id];

            switch (rci_info->action)
            {
                case CCAPI_RCI_ACTION_QUERY:
                {
                    ccapi_rci_function_t const process_callback = element->get;
                    void * p_element = NULL;

                    if (rci_info->group.type == CCAPI_RCI_GROUP_SETTING)
                    {
                        remote_config->response.compare_matches = CCAPI_FALSE;
                    }

                    switch (remote_config->element.type)
                    {
#if defined RCI_PARSER_USES_STRINGS
#if defined RCI_PARSER_USES_STRING
                        case connector_element_type_string:
#endif

#if defined RCI_PARSER_USES_MULTILINE_STRING
                        case connector_element_type_multiline_string:
#endif

#if defined RCI_PARSER_USES_PASSWORD
                        case connector_element_type_password:
#endif


#if defined RCI_PARSER_USES_FQDNV4
                        case connector_element_type_fqdnv4:
#endif

#if defined RCI_PARSER_USES_FQDNV6
                        case connector_element_type_fqdnv6:
#endif

#if defined RCI_PARSER_USES_DATETIME
                        case connector_element_type_datetime:
#endif
#if defined RCI_PARSER_USES_IPV4
                        case connector_element_type_ipv4:
#endif

#if defined RCI_PARSER_USES_MAC_ADDR
                        case connector_element_type_mac_addr:
#endif
                            p_element = &remote_config->response.element_value->string_value;
                            break;
#endif

#if defined RCI_PARSER_USES_INT32
                        case connector_element_type_int32:
                        {
                            p_element = &remote_config->response.element_value->signed_integer_value;
                            break;
                        }
#endif

#if (defined RCI_PARSER_USES_UNSIGNED_INTEGER)
#if defined RCI_PARSER_USES_UINT32
                        case connector_element_type_uint32:
#endif

#if defined RCI_PARSER_USES_HEX32
                        case connector_element_type_hex32:
#endif

#if defined RCI_PARSER_USES_0X_HEX32
                        case connector_element_type_0x_hex32:
#endif
                        {
                            p_element = &remote_config->response.element_value->unsigned_integer_value;
                            break;
                        }
#endif

#if defined RCI_PARSER_USES_FLOAT
                        case connector_element_type_float:
                        {
                            p_element = &remote_config->response.element_value->float_value;
                            break;
                        }
#endif

#if defined RCI_PARSER_USES_ENUM
                        case connector_element_type_enum:
                        {
                            p_element = &remote_config->response.element_value->enum_value;
                            break;
                        }
#endif

#if defined RCI_PARSER_USES_ON_OFF
                        case connector_element_type_on_off:
                        {
                            p_element = &remote_config->response.element_value->on_off_value;
                            break;
                        }
#endif

#if defined RCI_PARSER_USES_BOOLEAN
                        case connector_element_type_boolean:
                        {
                            p_element = &remote_config->response.element_value->boolean_value;
                            break;
                        }
#endif
                        default:
                            assert(0);
                    }
#if (defined RCI_PARSER_USES_ELEMENT_NAMES)
                    rci_info->element.name = remote_config->element.name;
#endif
                    error = process_callback(rci_info, p_element);

                    if (rci_info->group.type == CCAPI_RCI_GROUP_SETTING)
                    {
                        remote_config->response.compare_matches = CCAPI_BOOL_TO_CONNECTOR_BOOL(rci_info->query_setting.matches);
                        rci_info->query_setting.matches = CCAPI_FALSE;
                    }
                    break;
                }
                case CCAPI_RCI_ACTION_SET:
                {
                    ccapi_rci_function_t const process_callback = element->set;
                    switch (remote_config->element.type)
                    {
#if defined RCI_PARSER_USES_STRINGS
#if defined RCI_PARSER_USES_STRING
                        case connector_element_type_string:
#endif

#if defined RCI_PARSER_USES_MULTILINE_STRING
                        case connector_element_type_multiline_string:
#endif

#if defined RCI_PARSER_USES_PASSWORD
                        case connector_element_type_password:
#endif


#if defined RCI_PARSER_USES_FQDNV4
                        case connector_element_type_fqdnv4:
#endif

#if defined RCI_PARSER_USES_FQDNV6
                        case connector_element_type_fqdnv6:
#endif

#if defined RCI_PARSER_USES_DATETIME
                        case connector_element_type_datetime:
#endif
#if defined RCI_PARSER_USES_IPV4
                        case connector_element_type_ipv4:
#endif

#if defined RCI_PARSER_USES_MAC_ADDR
                        case connector_element_type_mac_addr:
#endif
                            error = process_callback(rci_info, remote_config->element.value->string_value);
                            break;
#endif

#if defined RCI_PARSER_USES_INT32
                        case connector_element_type_int32:
                        {
                            error = process_callback(rci_info, remote_config->element.value->signed_integer_value);
                            break;
                        }
#endif

#if (defined RCI_PARSER_USES_UNSIGNED_INTEGER)
#if defined RCI_PARSER_USES_UINT32
                        case connector_element_type_uint32:
#endif

#if defined RCI_PARSER_USES_HEX32
                        case connector_element_type_hex32:
#endif

#if defined RCI_PARSER_USES_0X_HEX32
                        case connector_element_type_0x_hex32:
#endif
                        {
                            error = process_callback(rci_info, remote_config->element.value->unsigned_integer_value);
                            break;
                        }
#endif

#if defined RCI_PARSER_USES_FLOAT
                        case connector_element_type_float:
                        {
                            error = process_callback(rci_info, remote_config->element.value->float_value);
                            break;
                        }
#endif

#if defined RCI_PARSER_USES_ENUM
                        case connector_element_type_enum:
                        {
                            error = process_callback(rci_info, remote_config->element.value->enum_value);
                            break;
                        }
#endif

#if defined RCI_PARSER_USES_ON_OFF
                        case connector_element_type_on_off:
                        {
                            error = process_callback(rci_info, remote_config->element.value->on_off_value);
                            break;
                        }
#endif

#if defined RCI_PARSER_USES_BOOLEAN
                        case connector_element_type_boolean:
                        {
                            error = process_callback(rci_info, remote_config->element.value->boolean_value);
                            break;
                        }
#endif
                    }
                    break;
                }
            }
            break;
        }
        case connector_request_id_remote_config_group_end:
        {
            unsigned int const group_id = remote_config->group.id;
            ccapi_rci_group_table_t const * group_table;
            ccapi_rci_group_t const * group;
            ccapi_rci_function_t end_callback;

            switch (rci_info->group.type)
            {
                case CCAPI_RCI_GROUP_SETTING:
                    group_table = &rci_data->settings;
                    break;
                case CCAPI_RCI_GROUP_STATE:
                    group_table = &rci_data->state;
                    break;
            }

            rci_info->group.instance = remote_config->group.index;
            ASSERT(group_id < group_table->count);
            group = &group_table->groups[group_id];
            ASSERT(group_id < group_table->count);
            group = &group_table->groups[group_id];
            end_callback = group->callbacks.end;
            error = end_callback(rci_info);

#if (defined RCI_PARSER_USES_GROUP_NAMES)
            rci_info->group.name = NULL;
#endif
#if (defined RCI_PARSER_USES_ELEMENT_NAMES)
            rci_info->element.name = NULL;
#endif
            break;
        }
        case connector_request_id_remote_config_action_end:
        {
            ccapi_rci_function_t const action_end_cb = rci_data->callbacks.end_action;

            error = action_end_cb(rci_info);
            break;
        }
        case connector_request_id_remote_config_session_end:
        {
            ccapi_rci_function_t const session_end_cb = rci_data->callbacks.end_session;

            error = session_end_cb(rci_info);
            break;
        }
        case connector_request_id_remote_config_session_cancel:
            ASSERT(connector_false);
            break;
    }

    if (error != 0)
    {
        remote_config->error_id = error;
    }
    remote_config->user_context = rci_info->user_context;
    status = connector_callback_continue;

    return status;
}

#endif
