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

#define connector_rci_error_none 0

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

#if (defined RCI_ENUMS_AS_STRINGS)
static unsigned int get_ccfsm_element_enum_count(connector_remote_config_data_t const * const rci_internal_data, connector_remote_group_type_t const ccfsm_group_type, unsigned int const group_id, unsigned int const element_id)
{
    unsigned int const enum_count = rci_internal_data->group_table[ccfsm_group_type].groups[group_id].elements.data[element_id].enums.count;

    ASSERT(group_id < rci_internal_data->group_table[ccfsm_group_type].count);
    ASSERT(element_id < rci_internal_data->group_table[ccfsm_group_type].groups[group_id].elements.count);
    ASSERT(rci_internal_data->group_table[ccfsm_group_type].groups[group_id].elements.data[element_id].type == connector_element_type_enum);
    return enum_count;
}

static connector_element_enum_t const * get_ccfsm_element_enum_info(connector_remote_config_data_t const * const rci_internal_data, connector_remote_group_type_t const ccfsm_group_type, unsigned int const group_id, unsigned int const element_id)
{
    connector_element_enum_t const * const element_enum = rci_internal_data->group_table[ccfsm_group_type].groups[group_id].elements.data[element_id].enums.data;

    ASSERT(group_id < rci_internal_data->group_table[ccfsm_group_type].count);
    ASSERT(element_id < rci_internal_data->group_table[ccfsm_group_type].groups[group_id].elements.count);
    ASSERT(rci_internal_data->group_table[ccfsm_group_type].groups[group_id].elements.data[element_id].type == connector_element_type_enum);
    return element_enum;
}

static char const * enum_to_string(connector_element_enum_t const * const element_enum_info, unsigned int enum_id)
{
    return element_enum_info[enum_id].name;
}

static int string_to_enum(connector_element_enum_t const * const element_enum_info, unsigned int const enum_count, char const * const enum_string)
{
    int enum_id = -1;
    unsigned int i;

    ASSERT(enum_string != NULL);
    for (i = 0; i < enum_count; i++)
    {
        if (strcmp(element_enum_info[i].name, enum_string) == 0)
        {
            enum_id = i;
            break;
        }
    }

    return enum_id;
}

static void queue_enum_callback(ccapi_data_t * const ccapi_data, connector_remote_config_t const * const remote_config)
{
    connector_remote_config_data_t const * const rci_desc = ccapi_data->service.rci.rci_data->rci_desc;
    unsigned int const group_id = remote_config->group.id;
    unsigned int const element_id = remote_config->element.id;
    connector_remote_group_type_t const group_type = remote_config->group.type;

    ccapi_data->service.rci.queued_callback.enum_data.array = get_ccfsm_element_enum_info(rci_desc, group_type, group_id, element_id);
    ccapi_data->service.rci.queued_callback.enum_data.element_count = get_ccfsm_element_enum_count(rci_desc, group_type, group_id, element_id);
}
#endif

void ccapi_rci_thread(void * const argument)
{
    ccapi_data_t * const ccapi_data = argument;

    /* ccapi_data is corrupted, it's likely the implementer made it wrong passing argument to the new thread */
    ASSERT_MSG_GOTO(ccapi_data != NULL, done);

    ccapi_data->thread.rci->status = CCAPI_THREAD_RUNNING;
    while (ccapi_data->thread.rci->status == CCAPI_THREAD_RUNNING)
    {
        ccapi_lock_acquire(ccapi_data->thread.rci->lock);

        if (ccapi_data->thread.rci->status != CCAPI_THREAD_REQUEST_STOP)
        {
            ASSERT_MSG_GOTO(ccapi_data->service.rci.rci_thread_status == CCAPI_RCI_THREAD_CB_QUEUED, done);
            ASSERT_MSG_GOTO(ccapi_data->service.rci.queued_callback.function_cb != NULL, done);

            /* Pass data to the user */ 
            if (ccapi_data->service.rci.queued_callback.argument == NULL)
            {
                ccapi_data->service.rci.queued_callback.error = ccapi_data->service.rci.queued_callback.function_cb(&ccapi_data->service.rci.rci_info);
            }
            else
            {
#if (defined RCI_ENUMS_AS_STRINGS)
                connector_element_enum_t const * const enum_array = ccapi_data->service.rci.queued_callback.enum_data.array;

                if (enum_array != NULL)
                {
                    ccapi_rci_info_t * const rci_info = &ccapi_data->service.rci.rci_info;
                    unsigned int const enum_element_count = ccapi_data->service.rci.queued_callback.enum_data.element_count;

                    if (rci_info->action == CCAPI_RCI_ACTION_QUERY)
                    {
                        char * enum_string = NULL;
                        int enum_id;
                        int * const actual_value = ccapi_data->service.rci.queued_callback.argument;
                        ccapi_data->service.rci.queued_callback.error = ccapi_data->service.rci.queued_callback.function_cb(rci_info, &enum_string);
                        if (ccapi_data->service.rci.queued_callback.error == connector_rci_error_none)
                        {
                            enum_id = string_to_enum(enum_array, enum_element_count, enum_string);
                            if (enum_id == -1)
                            {
                                ccapi_data->service.rci.queued_callback.error = connector_rci_error_bad_value;
                            }
                            *actual_value = enum_id;
                        }
                    }
                    else
                    {
                        char const * enum_string;
                        unsigned int const * const actual_value = ccapi_data->service.rci.queued_callback.argument;

                        ASSERT(*actual_value < enum_element_count);
                        enum_string = enum_to_string(enum_array, *actual_value);
                        ccapi_data->service.rci.queued_callback.error = ccapi_data->service.rci.queued_callback.function_cb(rci_info, enum_string);
                    }
                }
                else
#endif
                {
                    ccapi_data->service.rci.queued_callback.error = ccapi_data->service.rci.queued_callback.function_cb(&ccapi_data->service.rci.rci_info, ccapi_data->service.rci.queued_callback.argument);
                }
            }
                   
            /* Check if ccfsm has called cancel callback while we were waiting for the user */
            if (ccapi_data->service.rci.rci_thread_status == CCAPI_RCI_THREAD_CB_QUEUED)
            {
                ccapi_data->service.rci.rci_thread_status = CCAPI_RCI_THREAD_CB_PROCESSED;
            }
        }
    }
    ASSERT_MSG_GOTO(ccapi_data->thread.rci->status == CCAPI_THREAD_REQUEST_STOP, done);

done:
    ccapi_data->thread.rci->status = CCAPI_THREAD_NOT_STARTED;
    return;
}

static void clear_queued_callback(ccapi_data_t * const ccapi_data)
{
    ccapi_data->service.rci.queued_callback.function_cb = NULL;
    ccapi_data->service.rci.queued_callback.argument = NULL;
    ccapi_data->service.rci.queued_callback.error = connector_rci_error_none;
#if (defined RCI_ENUMS_AS_STRINGS)
    ccapi_data->service.rci.queued_callback.enum_data.array = NULL;
    ccapi_data->service.rci.queued_callback.enum_data.element_count = 0;
#endif
}

static ccapi_rci_element_type_t connector_rci_element_type_to_ccapi(connector_element_value_type_t const ccfsm_type)
{
    ccapi_rci_element_type_t ccapi_type;
    switch (ccfsm_type)
    {
#if defined RCI_PARSER_USES_STRING
        case connector_element_type_string:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_STRING;
            break;
#endif
#if defined RCI_PARSER_USES_MULTILINE_STRING
        case connector_element_type_multiline_string:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_MULTILINE_STRING;
            break;
#endif
#if defined RCI_PARSER_USES_PASSWORD
        case connector_element_type_password:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_PASSWORD;
            break;
#endif
#if defined RCI_PARSER_USES_FQDNV4
        case connector_element_type_fqdnv4:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_FQDNV4;
            break;
#endif
#if defined RCI_PARSER_USES_FQDNV6
        case connector_element_type_fqdnv6:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_FQDNV6;
            break;
#endif
#if defined RCI_PARSER_USES_DATETIME
        case connector_element_type_datetime:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_DATETIME;
            break;
#endif
#if defined RCI_PARSER_USES_IPV4
        case connector_element_type_ipv4:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_IPV4;
            break;
#endif
#if defined RCI_PARSER_USES_MAC_ADDR
        case connector_element_type_mac_addr:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_MAC;
            break;
#endif
#if defined RCI_PARSER_USES_INT32
        case connector_element_type_int32:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_INT32;
            break;
#endif
#if defined RCI_PARSER_USES_UINT32
        case connector_element_type_uint32:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_UINT32;
            break;
#endif
#if defined RCI_PARSER_USES_HEX32
        case connector_element_type_hex32:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_HEX32;
            break;
#endif
#if defined RCI_PARSER_USES_0X_HEX32
        case connector_element_type_0x_hex32:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_0X32;
            break;
#endif
#if defined RCI_PARSER_USES_FLOAT
        case connector_element_type_float:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_FLOAT;
            break;
#endif
#if defined RCI_PARSER_USES_ENUM
        case connector_element_type_enum:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_ENUM;
            break;
#endif
#if defined RCI_PARSER_USES_ON_OFF
        case connector_element_type_on_off:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_ON_OFF;
            break;
#endif
#if defined RCI_PARSER_USES_BOOLEAN
        case connector_element_type_boolean:
            ccapi_type = CCAPI_RCI_ELEMENT_TYPE_BOOL;
            break;
#endif
    }
    return ccapi_type;
}

connector_callback_status_t ccapi_rci_handler(connector_request_id_remote_config_t const request_id, void * const data, ccapi_data_t * const ccapi_data)
{
    connector_callback_status_t status;
    connector_remote_config_t * const remote_config = data;
    ccapi_rci_data_t const * const rci_data = ccapi_data->service.rci.rci_data;
    ccapi_rci_info_t * const rci_info = &ccapi_data->service.rci.rci_info;

    if (request_id == connector_request_id_remote_config_session_cancel)
    {
        clear_queued_callback(ccapi_data);

        ccapi_data->service.rci.rci_thread_status = CCAPI_RCI_THREAD_IDLE;

        status = connector_callback_continue;

        goto done;
    }

    switch (ccapi_data->service.rci.rci_thread_status)
    {
        case CCAPI_RCI_THREAD_IDLE:
        {
            ASSERT(ccapi_data->service.rci.queued_callback.function_cb == NULL);
            clear_queued_callback(ccapi_data);

            rci_info->user_context = remote_config->user_context;
            rci_info->error_hint = remote_config->response.error_hint;

            switch (request_id)
            {
                case connector_request_id_remote_config_session_start:
                {
                    ccapi_rci_function_t const session_start_cb = rci_data->callback.start_session;

                    rci_info->action = CCAPI_RCI_ACTION_QUERY;
                    rci_info->error_hint = NULL;
                    rci_info->group.instance = 0;
                    rci_info->group.type = CCAPI_RCI_GROUP_SETTING;
                    rci_info->query_setting.matches = CCAPI_FALSE;
                    rci_info->query_setting.attributes.compare_to = CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_COMPARE_TO_NONE;
                    rci_info->query_setting.attributes.source = CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_SOURCE_CURRENT;
                    rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_NOT_SET;
#if (defined RCI_LEGACY_COMMANDS)
                    rci_info->do_command.target = NULL;
                    rci_info->do_command.request = NULL;
                    rci_info->do_command.response = NULL;
#endif
                    ccapi_data->service.rci.queued_callback.function_cb = session_start_cb;
                    break;
                }
                case connector_request_id_remote_config_action_start:
                {
                    connector_remote_group_type_t const group_type = remote_config->group.type;
                    ccapi_rci_function_t const action_start_cb = rci_data->callback.start_action;

                    rci_info->group.instance = 0;
                    rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_NOT_SET;
                    switch (remote_config->action)
                    {
                        case connector_remote_action_set:
                            rci_info->action = CCAPI_RCI_ACTION_SET;
                            break;
                        case connector_remote_action_query:
                            rci_info->action = CCAPI_RCI_ACTION_QUERY;
                            break;
#if (defined RCI_LEGACY_COMMANDS)
                        case connector_remote_action_do_command:
                            rci_info->action = CCAPI_RCI_ACTION_DO_COMMAND;
                            break;
                        case connector_remote_action_reboot:
                            rci_info->action = CCAPI_RCI_ACTION_REBOOT;
                            break;
                        case connector_remote_action_set_factory_def:
                            rci_info->action = CCAPI_RCI_ACTION_SET_FACTORY_DEFAULTS;
                            break;
#endif
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
                    ccapi_data->service.rci.queued_callback.function_cb = action_start_cb;
                    break;
                }
#if (defined RCI_LEGACY_COMMANDS)
                case connector_request_id_remote_config_do_command:
                    ccapi_data->service.rci.queued_callback.function_cb = rci_data->callback.do_command;
                    ccapi_data->service.rci.queued_callback.argument = NULL;
                    ccapi_data->service.rci.rci_info.do_command.target = remote_config->attribute.target;
                    ccapi_data->service.rci.rci_info.do_command.request = remote_config->element.value->string_value;
                    ccapi_data->service.rci.rci_info.do_command.response = &remote_config->response.element_value->string_value;
                    break;
                case connector_request_id_remote_config_reboot:
                    ccapi_data->service.rci.queued_callback.function_cb = rci_data->callback.reboot;
                    ccapi_data->service.rci.queued_callback.argument = NULL;
                    break;
                    break;
                case connector_request_id_remote_config_set_factory_def:
                    ccapi_data->service.rci.queued_callback.function_cb = rci_data->callback.set_factory_defaults;
                    ccapi_data->service.rci.queued_callback.argument = NULL;
                    break;
#endif
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
                    rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_NOT_SET;
#if (defined RCI_PARSER_USES_GROUP_NAMES)
                    rci_info->group.name = remote_config->group.name;
#endif
                    ASSERT(group_id < group_table->count);
                    group = &group_table->groups[group_id];
                    start_callback = group->callback.start;
                    ccapi_data->service.rci.queued_callback.function_cb = start_callback;
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
                            rci_info->group.type = CCAPI_RCI_GROUP_STATE;
                            group_table = &rci_data->state;
                            break;
                    }

                    ASSERT(group_id < group_table->count);
                    group = &group_table->groups[group_id];

                    ASSERT(element_id < group->count);
                    element = &group->elements[element_id];

                    rci_info->element.type = connector_rci_element_type_to_ccapi(remote_config->element.type);
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
#if (defined RCI_ENUMS_AS_STRINGS)
                                    queue_enum_callback(ccapi_data, remote_config);
#endif
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
                            }
#if (defined RCI_PARSER_USES_ELEMENT_NAMES)
                            rci_info->element.name = remote_config->element.name;
#endif
                            ccapi_data->service.rci.queued_callback.function_cb = process_callback;
                            ccapi_data->service.rci.queued_callback.argument = p_element;
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
                                    ccapi_data->service.rci.queued_callback.function_cb = process_callback;
                                    ccapi_data->service.rci.queued_callback.argument = (void *)remote_config->element.value->string_value;
                                    break;
#endif

#if defined RCI_PARSER_USES_INT32
                                case connector_element_type_int32:
                                {
                                    ccapi_data->service.rci.queued_callback.function_cb = process_callback;
                                    ccapi_data->service.rci.queued_callback.argument = (void *)&remote_config->element.value->signed_integer_value;
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
                                    ccapi_data->service.rci.queued_callback.function_cb = process_callback;
                                    ccapi_data->service.rci.queued_callback.argument = (void *)&remote_config->element.value->unsigned_integer_value;
                                    break;
                                }
#endif

#if defined RCI_PARSER_USES_FLOAT
                                case connector_element_type_float:
                                {
                                    ccapi_data->service.rci.queued_callback.function_cb = process_callback;
                                    ccapi_data->service.rci.queued_callback.argument = (void *)&remote_config->element.value->float_value;
                                    break;
                                }
#endif

#if defined RCI_PARSER_USES_ENUM
                                case connector_element_type_enum:
                                {
#if (defined RCI_ENUMS_AS_STRINGS)
                                    queue_enum_callback(ccapi_data, remote_config);
#endif
                                    ccapi_data->service.rci.queued_callback.function_cb = process_callback;
                                    ccapi_data->service.rci.queued_callback.argument = (void *)&remote_config->element.value->enum_value;
                                    break;
                                }
#endif

#if defined RCI_PARSER_USES_ON_OFF
                                case connector_element_type_on_off:
                                {
                                    ccapi_data->service.rci.queued_callback.function_cb = process_callback;
                                    ccapi_data->service.rci.queued_callback.argument = (void *)&remote_config->element.value->on_off_value;
                                    break;
                                }
#endif

#if defined RCI_PARSER_USES_BOOLEAN
                                case connector_element_type_boolean:
                                {
                                    ccapi_data->service.rci.queued_callback.function_cb = process_callback;
                                    ccapi_data->service.rci.queued_callback.argument = (void *)&remote_config->element.value->boolean_value;
                                    break;
                                }
#endif
                            }
#if (defined RCI_PARSER_USES_ELEMENT_NAMES)
                            rci_info->element.name = remote_config->element.name;
#endif
                            break;
                        }
#if (defined RCI_LEGACY_COMMANDS)
                        case CCAPI_RCI_ACTION_DO_COMMAND:
                        case CCAPI_RCI_ACTION_REBOOT:
                        case CCAPI_RCI_ACTION_SET_FACTORY_DEFAULTS:
                            ASSERT(rci_info->action == CCAPI_RCI_ACTION_QUERY || rci_info->action == CCAPI_RCI_ACTION_SET);
                            break;
#endif
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
                    rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_NOT_SET;
                    ASSERT(group_id < group_table->count);
                    group = &group_table->groups[group_id];
                    end_callback = group->callback.end;
                    ccapi_data->service.rci.queued_callback.function_cb = end_callback;
                    break;
                }
                case connector_request_id_remote_config_action_end:
                {
                    ccapi_rci_function_t const action_end_cb = rci_data->callback.end_action;
                    rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_NOT_SET;
                    ccapi_data->service.rci.queued_callback.function_cb = action_end_cb;
                    break;
                }
                case connector_request_id_remote_config_session_end:
                {
                    ccapi_rci_function_t const session_end_cb = rci_data->callback.end_session;
                    rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_NOT_SET;
                    ccapi_data->service.rci.queued_callback.function_cb = session_end_cb;
                    break;
                }
                case connector_request_id_remote_config_session_cancel:
                    ASSERT(connector_false);
                    break;
            }

            ccapi_data->service.rci.rci_thread_status = CCAPI_RCI_THREAD_CB_QUEUED;

            ccapi_lock_release(ccapi_data->thread.rci->lock);

            status = connector_callback_busy;

            break;
        }
        case CCAPI_RCI_THREAD_CB_QUEUED:
        {
            status = connector_callback_busy;

            break;
        }    
        case CCAPI_RCI_THREAD_CB_PROCESSED:
        {
            switch (request_id)
            {
                case connector_request_id_remote_config_session_start:
                case connector_request_id_remote_config_action_start:
                    break;
#if (defined RCI_LEGACY_COMMANDS)
                case connector_request_id_remote_config_do_command:
                case connector_request_id_remote_config_reboot:
                case connector_request_id_remote_config_set_factory_def:
                    break;
#endif
                case connector_request_id_remote_config_group_start:
                {
                    if (rci_info->group.type == CCAPI_RCI_GROUP_SETTING && rci_info->action == CCAPI_RCI_ACTION_QUERY)
                    {
                        remote_config->response.compare_matches = CCAPI_BOOL_TO_CONNECTOR_BOOL(rci_info->query_setting.matches);
                    }
                    break;
                }
                case connector_request_id_remote_config_group_process:
                {
                    switch (rci_info->action)
                    {
                        case CCAPI_RCI_ACTION_QUERY:
                        {
                            if (rci_info->group.type == CCAPI_RCI_GROUP_SETTING)
                            {
                                remote_config->response.compare_matches = CCAPI_BOOL_TO_CONNECTOR_BOOL(rci_info->query_setting.matches);
                                rci_info->query_setting.matches = CCAPI_FALSE;
                            }
                            break;
                        }
                        case CCAPI_RCI_ACTION_SET:
#if (defined RCI_LEGACY_COMMANDS)
                        case CCAPI_RCI_ACTION_DO_COMMAND:
                        case CCAPI_RCI_ACTION_REBOOT:
                        case CCAPI_RCI_ACTION_SET_FACTORY_DEFAULTS:
#endif
                            break;
                    }
                    break;
                }
                case connector_request_id_remote_config_group_end:
                {
#if (defined RCI_PARSER_USES_GROUP_NAMES)
                    rci_info->group.name = NULL;
#endif
#if (defined RCI_PARSER_USES_ELEMENT_NAMES)
                    rci_info->element.name = NULL;
#endif
                    break;
                }
                case connector_request_id_remote_config_action_end:
#if (defined RCI_LEGACY_COMMANDS)
                    if (rci_info->action == CCAPI_RCI_ACTION_DO_COMMAND)
                    {
                        rci_info->do_command.target = NULL;
                        rci_info->do_command.request = NULL;
                        rci_info->do_command.response = NULL;
                    }
#endif
                    break;
                case connector_request_id_remote_config_session_end:
                    break;
                case connector_request_id_remote_config_session_cancel:
                    ASSERT(connector_false);
                    break;
            }

            remote_config->error_id = ccapi_data->service.rci.queued_callback.error;
            remote_config->user_context = rci_info->user_context;
            remote_config->response.error_hint = rci_info->error_hint;

            clear_queued_callback(ccapi_data);

            ccapi_data->service.rci.rci_thread_status = CCAPI_RCI_THREAD_IDLE;

            status = connector_callback_continue;

            break;
        }
    }

done:
    return status;
}

#endif
