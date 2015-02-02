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

#ifndef _CCAPI_RCI_H_
#define _CCAPI_RCI_H_

#if (defined CCAPI_CONST_PROTECTION_UNLOCK)
#define CONST
#else
#define CONST   const
#endif

typedef enum {
    CCAPI_OFF,
    CCAPI_ON
} ccapi_on_off_t;

typedef enum {
    CCAPI_RCI_ACTION_SET,
    CCAPI_RCI_ACTION_QUERY
#if (defined RCI_LEGACY_COMMANDS)
    ,CCAPI_RCI_ACTION_DO_COMMAND,
    CCAPI_RCI_ACTION_REBOOT,
    CCAPI_RCI_ACTION_SET_FACTORY_DEFAULTS
#endif
} ccapi_rci_action_t;

typedef enum {
    CCAPI_RCI_GROUP_SETTING,
    CCAPI_RCI_GROUP_STATE
} ccapi_rci_group_type_t;

typedef enum {
    CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_SOURCE_CURRENT,
    CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_SOURCE_STORED,
    CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_SOURCE_DEFAULTS
} ccapi_rci_query_setting_attribute_source_t;

typedef enum {
    CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_COMPARE_TO_NONE,
    CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_COMPARE_TO_CURRENT,
    CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_COMPARE_TO_STORED,
    CCAPI_RCI_QUERY_SETTING_ATTRIBUTE_COMPARE_TO_DEFAULTS
} ccapi_rci_query_setting_attribute_compare_to_t;

typedef struct {
    ccapi_rci_query_setting_attribute_source_t source;
    ccapi_rci_query_setting_attribute_compare_to_t compare_to;
} ccapi_rci_query_setting_attributes_t;

typedef struct {
    struct {
        unsigned int CONST instance;
        ccapi_rci_group_type_t CONST type;
#if (defined RCI_PARSER_USES_GROUP_NAMES)
        char const * CONST name;
#endif
    } group;
#if (defined RCI_PARSER_USES_ELEMENT_NAMES)
    struct {
        char const * CONST name;
    } element;
#endif
    struct {
        ccapi_rci_query_setting_attributes_t CONST attributes;
        ccapi_bool_t matches;
    } query_setting;
#if (defined RCI_LEGACY_COMMANDS)
    struct {
        char const * CONST target;
        char const * CONST request;
        char const * * CONST response;
    } do_command;
#endif
    ccapi_rci_action_t CONST action;
    char const * error_hint;
    void * user_context;
} ccapi_rci_info_t;

typedef unsigned int (*ccapi_rci_function_t)(ccapi_rci_info_t * const info, ...);

typedef struct {
    ccapi_rci_function_t set;
    ccapi_rci_function_t get;
} ccapi_rci_element_t;

typedef struct {
    ccapi_rci_element_t const * const elements;
    unsigned int count;
    struct {
        ccapi_rci_function_t start;
        ccapi_rci_function_t end;
    } callback;
} ccapi_rci_group_t;

typedef struct {
    ccapi_rci_group_t const * const groups;
    unsigned int count;
} ccapi_rci_group_table_t;

typedef struct {
    ccapi_rci_group_table_t settings;
    ccapi_rci_group_table_t state;
    struct {
        ccapi_rci_function_t start_session;
        ccapi_rci_function_t end_session;
        ccapi_rci_function_t start_action;
        ccapi_rci_function_t end_action;
#if (defined RCI_LEGACY_COMMANDS)
        ccapi_rci_function_t do_command;
        ccapi_rci_function_t set_factory_defaults;
        ccapi_rci_function_t reboot;
#endif
    } callback;
    struct connector_remote_config_data const * rci_desc;
} ccapi_rci_data_t;

typedef struct {
    ccapi_rci_data_t const * rci_data;
} ccapi_rci_service_t;
#endif
