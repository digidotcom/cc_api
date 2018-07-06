/*
* Copyright (c) 2017, 2018 Digi International Inc.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
* REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
* AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
* INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
* LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
* OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
* PERFORMANCE OF THIS SOFTWARE.
*
* Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
* =======================================================================
*/

#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

#if (defined CCIMP_RCI_SERVICE_ENABLED)

#define connector_rci_error_none 0

#if (defined RCI_PARSER_USES_COLLECTION_NAMES)
#define CLEAR_GROUP_NAME(rci_info) (rci_info)->group.name = NULL
#define CLEAR_LIST_NAME(rci_info, i) (rci_info)->list.data[(i)].name = NULL
#else
#define CLEAR_GROUP_NAME(rci_info)
#define CLEAR_LIST_NAME(rci_info, i)
#endif

#if (defined RCI_PARSER_USES_ELEMENT_NAMES)
#define CLEAR_ELEMENT_NAME(rci_info) (rci_info)->element.name = NULL
#else
#define CLEAR_ELEMENT_NAME(rci_info)
#endif

#define CLEAR_GROUP_INFO(rci_info) \
do \
{ \
	(rci_info)->group.id = 0; \
	(rci_info)->group.instance = 0; \
	CLEAR_GROUP_NAME(rci_info); \
} while (0)

#define CLEAR_LIST_INFO(rci_info, i) \
do \
{ \
		(rci_info)->list.data[i].id = 0; \
		(rci_info)->list.data[i].instance = 0; \
		CLEAR_LIST_NAME(rci_info, i); \
} while (0)

#define CLEAR_ALL_LIST_INFO(rci_info) \
do \
{ \
	int i; \
	(rci_info)->list.current = 0; \
	for (i = 0; i < RCI_LIST_MAX_DEPTH; i++) \
	{ \
		CLEAR_LIST_INFO(rci_info, i); \
	} \
} while (0)

#define CLEAR_ELEMENT_INFO(rci_info) \
do \
{ \
	(rci_info)->element.id = 0; \
	CLEAR_ELEMENT_NAME(rci_info); \
} while (0)


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
    unsigned int const enum_count = rci_internal_data->group_table[ccfsm_group_type].groups[group_id].collection.item.data[element_id].data.element->enums.count;

    ASSERT(group_id < rci_internal_data->group_table[ccfsm_group_type].count);
    ASSERT(element_id < rci_internal_data->group_table[ccfsm_group_type].groups[group_id].collection.item.count);
    ASSERT(rci_internal_data->group_table[ccfsm_group_type].groups[group_id].collection.item.data[element_id].type == connector_element_type_enum);
    return enum_count;
}

static connector_element_enum_t const * get_ccfsm_element_enum_info(connector_remote_config_data_t const * const rci_internal_data, connector_remote_group_type_t const ccfsm_group_type, unsigned int const group_id, unsigned int const element_id)
{
    connector_element_enum_t const * const element_enum = rci_internal_data->group_table[ccfsm_group_type].groups[group_id].collection.item.data[element_id].data.element->enums.data;

    ASSERT(group_id < rci_internal_data->group_table[ccfsm_group_type].count);
    ASSERT(element_id < rci_internal_data->group_table[ccfsm_group_type].groups[group_id].collection.item.count);
    ASSERT(rci_internal_data->group_table[ccfsm_group_type].groups[group_id].collection.item.data[element_id].type == connector_element_type_enum);
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
                        ccapi_element_value_t element_value = { 0 };
                        int enum_id;
                        int * const actual_value = ccapi_data->service.rci.queued_callback.argument;
                        ccapi_data->service.rci.queued_callback.error = ccapi_data->service.rci.queued_callback.function_cb(rci_info, &element_value);
                        if (ccapi_data->service.rci.queued_callback.error == connector_rci_error_none)
                        {
                            enum_id = string_to_enum(enum_array, enum_element_count, element_value.string_value);
                            if (enum_id == -1)
                            {
                                ccapi_data->service.rci.queued_callback.error = connector_rci_error_bad_value;
                            }
                            *actual_value = enum_id;
                        }
                    }
                    else
                    {
						ccapi_element_value_t element_value;
                        unsigned int const * const actual_value = ((ccapi_element_value_t *) ccapi_data->service.rci.queued_callback.argument)->enum_value;

                        ASSERT(*actual_value < enum_element_count);
                        element_value.string_value = enum_to_string(enum_array, *actual_value);
                        ccapi_data->service.rci.queued_callback.error = ccapi_data->service.rci.queued_callback.function_cb(rci_info, &element_value);
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
                    rci_info->query_setting.matches = CCAPI_FALSE;
                    rci_info->query_setting.attributes.compare_to = CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_COMPARE_TO_NONE;
                    rci_info->query_setting.attributes.source = CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_SOURCE_CURRENT;
                    rci_info->group.type = CCAPI_RCI_GROUP_SETTING;
					CLEAR_GROUP_INFO(rci_info);
					CLEAR_ALL_LIST_INFO(rci_info);
					CLEAR_ELEMENT_INFO(rci_info);
                    rci_info->do_command.target = NULL;
                    rci_info->do_command.request = NULL;
                    rci_info->do_command.response = NULL;
                    ccapi_data->service.rci.queued_callback.function_cb = session_start_cb;
                    break;
                }
                case connector_request_id_remote_config_action_start:
                {
                    connector_remote_group_type_t const group_type = remote_config->group.type;
                    ccapi_rci_function_t const action_start_cb = rci_data->callback.start_action;

                    switch (remote_config->action)
                    {
                        case connector_remote_action_set:
                            rci_info->action = CCAPI_RCI_ACTION_SET;
                            break;
                        case connector_remote_action_query:
                            rci_info->action = CCAPI_RCI_ACTION_QUERY;
                            break;
                        case connector_remote_action_do_command:
                            rci_info->action = CCAPI_RCI_ACTION_DO_COMMAND;
                            break;
                        case connector_remote_action_reboot:
                            rci_info->action = CCAPI_RCI_ACTION_REBOOT;
                            break;
                        case connector_remote_action_set_factory_def:
                            rci_info->action = CCAPI_RCI_ACTION_SET_FACTORY_DEFAULTS;
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

					CLEAR_GROUP_INFO(rci_info);
					CLEAR_ALL_LIST_INFO(rci_info);
					CLEAR_ELEMENT_INFO(rci_info);

                    ccapi_data->service.rci.queued_callback.function_cb = action_start_cb;
                    break;
                }
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
                case connector_request_id_remote_config_group_start:
                {
                    rci_info->group.id = remote_config->group.id;
                    rci_info->group.instance = remote_config->group.index;
#if (defined RCI_PARSER_USES_COLLECTION_NAMES)
                    rci_info->group.name = remote_config->group.name;
#endif
					CLEAR_ALL_LIST_INFO(rci_info);
					CLEAR_ELEMENT_INFO(rci_info);
                    ccapi_data->service.rci.queued_callback.function_cb = rci_data->callback.start_group;
                    break;
                }
				case connector_request_id_remote_config_list_start:
                {
					unsigned int const index = remote_config->list.depth - 1;
					rci_info->list.current = remote_config->list.depth;
                    rci_info->list.data[index].id = remote_config->list.level[index].id;
                    rci_info->list.data[index].instance = remote_config->list.level[index].index;
#if (defined RCI_PARSER_USES_COLLECTION_NAMES)
                    rci_info->list.data[index].name = remote_config->list.level[index].name;
#endif
					CLEAR_ELEMENT_INFO(rci_info);
                    ccapi_data->service.rci.queued_callback.function_cb = rci_data->callback.start_list;
                    break;
                }
                case connector_request_id_remote_config_element_process:
                {
					rci_info->list.current = remote_config->list.depth;
					rci_info->element.id = remote_config->element.id;
#if (defined RCI_PARSER_USES_ELEMENT_NAMES)
					rci_info->element.name = remote_config->element.name;
#endif
					switch (rci_info->action)
					{
						case CCAPI_RCI_ACTION_QUERY:
                            ccapi_data->service.rci.queued_callback.function_cb = rci_data->callback.get_element;
                            ccapi_data->service.rci.queued_callback.argument = remote_config->response.element_value;
							break;
						case CCAPI_RCI_ACTION_SET:
                            ccapi_data->service.rci.queued_callback.function_cb = rci_data->callback.set_element;
							ccapi_data->service.rci.queued_callback.argument = &remote_config->element.value;
							break;
                        case CCAPI_RCI_ACTION_DO_COMMAND:
                        case CCAPI_RCI_ACTION_REBOOT:
                        case CCAPI_RCI_ACTION_SET_FACTORY_DEFAULTS:
                            ASSERT(0);
                            break;
					}
					switch (remote_config->element.type)
					{
						case connector_element_type_string:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_STRING;
							break;
						case connector_element_type_multiline_string:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_MULTILINE_STRING;
							break;
						case connector_element_type_password:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_PASSWORD;
							break;
						case connector_element_type_fqdnv4:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_FQDNV4;
							break;
						case connector_element_type_fqdnv6:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_FQDNV6;
							break;
						case connector_element_type_datetime:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_DATETIME;
							break;
						case connector_element_type_ipv4:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_IPV4;
							break;
						case connector_element_type_mac_addr:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_MAC;
							break;
						case connector_element_type_int32:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_INT32;
							break;
						case connector_element_type_uint32:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_UINT32;
							break;
						case connector_element_type_hex32:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_HEX32;
							break;
						case connector_element_type_0x_hex32:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_0X32;
							break;
						case connector_element_type_float:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_FLOAT;
							break;
						case connector_element_type_enum:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_ENUM;
#if (defined RCI_ENUMS_AS_STRINGS)
                            queue_enum_callback(ccapi_data, remote_config);
#endif
							break;
						case connector_element_type_on_off:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_ON_OFF;
							break;
						case connector_element_type_boolean:
							rci_info->element.type = CCAPI_RCI_ELEMENT_TYPE_BOOL;
							break;
						case connector_element_type_list:
							ASSERT(0);
							break;
					}
                    break;
                }
				case connector_request_id_remote_config_list_end:
				{
					rci_info->list.current = remote_config->list.depth;
					ccapi_data->service.rci.queued_callback.function_cb = rci_data->callback.end_list;
					break;
				}
                case connector_request_id_remote_config_group_end:
                {
					ccapi_data->service.rci.queued_callback.function_cb = rci_data->callback.end_group;
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

            if (ccapi_data->service.rci.queued_callback.function_cb != NULL)
            {
                ccapi_data->service.rci.rci_thread_status = CCAPI_RCI_THREAD_CB_QUEUED;
                ccapi_lock_release(ccapi_data->thread.rci->lock);
            }
            else
            {
                ccapi_data->service.rci.rci_thread_status = CCAPI_RCI_THREAD_CB_PROCESSED;
            }

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
                case connector_request_id_remote_config_do_command:
                case connector_request_id_remote_config_reboot:
                case connector_request_id_remote_config_set_factory_def:
                    break;
                case connector_request_id_remote_config_group_start:
				case connector_request_id_remote_config_list_start:
                {
                    if (rci_info->group.type == CCAPI_RCI_GROUP_SETTING && rci_info->action == CCAPI_RCI_ACTION_QUERY)
                    {
                        remote_config->response.compare_matches = CCAPI_BOOL_TO_CONNECTOR_BOOL(rci_info->query_setting.matches);
                    }
                    break;
                }
                case connector_request_id_remote_config_element_process:
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
                        case CCAPI_RCI_ACTION_DO_COMMAND:
                        case CCAPI_RCI_ACTION_REBOOT:
                        case CCAPI_RCI_ACTION_SET_FACTORY_DEFAULTS:
                            break;
                    }
                    break;
                }
				case connector_request_id_remote_config_list_end:
				{
					CLEAR_LIST_INFO(rci_info, rci_info->list.current);
					CLEAR_ELEMENT_INFO(rci_info);
					break;
				}
                case connector_request_id_remote_config_group_end:
                {
					CLEAR_GROUP_INFO(rci_info);
					CLEAR_ALL_LIST_INFO(rci_info);
					CLEAR_ELEMENT_INFO(rci_info);
                    break;
                }
                case connector_request_id_remote_config_action_end:
                    if (rci_info->action == CCAPI_RCI_ACTION_DO_COMMAND)
                    {
                        rci_info->do_command.target = NULL;
                        rci_info->do_command.request = NULL;
                        rci_info->do_command.response = NULL;
                    }
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
