#include  <stdio.h>
#include  <sys/stat.h>
#include  <unistd.h>
#include  "ccapi/ccapi.h"
#include  "ccapi_rci_functions.h"

#include  "rci_xml_sample.h"

FILE * xml_request_fp = NULL;
char * xml_query_response_buffer = NULL;
char * value_end_ptr = NULL;

ccapi_global_error_id_t rci_session_start_cb(ccapi_rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_global_error_id_t rci_session_end_cb(ccapi_rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return CCAPI_GLOBAL_ERROR_NONE;
}

static void print_group(ccapi_rci_info_t * const info)
{
    switch (info->group.type)
    {
        case CCAPI_RCI_GROUP_SETTING:
            fprintf(xml_request_fp, "setting");
            break;
        case CCAPI_RCI_GROUP_STATE:
            fprintf(xml_request_fp, "state");
            break;
    }

    return;
}
 
ccapi_global_error_id_t rci_action_start_cb(ccapi_rci_info_t * const info)
{
    ccapi_global_error_id_t error_id = CCAPI_GLOBAL_ERROR_NONE;

    printf("    Called '%s'\n", __FUNCTION__);

    switch(info->action)
    {
        case CCAPI_RCI_ACTION_SET:
        {
            xml_request_fp = fopen(XML_REQUEST_FILE_NAME, "w+");
            if (xml_request_fp == NULL)
            {
                printf("%s: Unable to create %s file\n", __FUNCTION__,  XML_REQUEST_FILE_NAME);
                error_id = CCAPI_GLOBAL_ERROR_XML_REQUEST_FAIL;
                info->error_hint = "Unable to create xml request file";
                goto done;
            }

            fprintf(xml_request_fp, "<set_");
            print_group(info);
            fprintf(xml_request_fp, ">");

            break;
        }
        case CCAPI_RCI_ACTION_QUERY:
        {
            break;
        }
#if (defined RCI_LEGACY_COMMANDS)
        case CCAPI_RCI_ACTION_DO_COMMAND:
        case CCAPI_RCI_ACTION_REBOOT:
        case CCAPI_RCI_ACTION_SET_FACTORY_DEFAULTS:
        {
            /* TODO */
            break;
        }
#endif
    }

done:
    return error_id;
}

static int get_response_buffer(char * * xml_response_buffer, ccapi_rci_info_t * const info)
{
    int error_id = CCAPI_GLOBAL_ERROR_NONE;
    struct stat response_info;
    FILE * xml_response_fp = NULL;

    stat(XML_RESPONSE_FILE_NAME, &response_info);

    *xml_response_buffer = malloc(response_info.st_size + 1);

    if (*xml_response_buffer == NULL)
    {
        error_id = CCAPI_GLOBAL_ERROR_MEMORY_FAIL;
        info->error_hint = "Couldn't malloc to proccess xml response";
        goto done;
    }

    xml_response_fp = fopen(XML_RESPONSE_FILE_NAME, "r");
    if (xml_response_fp == NULL)
    {
        printf("%s: Unable to open %s file\n", __FUNCTION__,  XML_RESPONSE_FILE_NAME);
        error_id = CCAPI_GLOBAL_ERROR_XML_RESPONSE_FAIL;
        info->error_hint = "Unable to open xml response file";
        goto done;
    }          

    fread(*xml_response_buffer, 1, response_info.st_size, xml_response_fp);
    (*xml_response_buffer)[response_info.st_size] = '\0';
    if (ferror(xml_response_fp) != 0)
    {
        printf("%s: Failed to read %s file\n", __FUNCTION__, XML_RESPONSE_FILE_NAME);

        error_id = CCAPI_GLOBAL_ERROR_XML_RESPONSE_FAIL;
        info->error_hint = "Error reading xml response file";
        goto done;
    }
    else
    {
       printf("response='%s'\n", *xml_response_buffer);
    }          

    fclose(xml_response_fp);

done:
    return error_id;
}

ccapi_global_error_id_t rci_action_end_cb(ccapi_rci_info_t * const info)
{
    ccapi_global_error_id_t error_id = CCAPI_GLOBAL_ERROR_NONE;

    printf("    Called '%s'\n", __FUNCTION__);

    switch(info->action)
    {
        case CCAPI_RCI_ACTION_SET:
        {
            fprintf(xml_request_fp, "\n</set_");
            print_group(info);
            fprintf(xml_request_fp, ">\n");

            fclose(xml_request_fp);

            break;
        }
        case CCAPI_RCI_ACTION_QUERY:
        {
            break;
        }
#if (defined RCI_LEGACY_COMMANDS)
        case CCAPI_RCI_ACTION_DO_COMMAND:
        case CCAPI_RCI_ACTION_REBOOT:
        case CCAPI_RCI_ACTION_SET_FACTORY_DEFAULTS:
        {
            /* TODO */
            break;
        }
#endif
    }

    switch(info->action)
    {
        case CCAPI_RCI_ACTION_SET:
        {
            char * xml_set_response_buffer = NULL;
            char * error_ptr = NULL;

            linux_rci_handle();

            error_id = get_response_buffer(&xml_set_response_buffer, info);
            if (error_id != CCAPI_GLOBAL_ERROR_NONE)
            {
                goto done;
            }

            if (strlen(xml_set_response_buffer) == 0)
            {
                error_id = CCAPI_GLOBAL_ERROR_XML_RESPONSE_FAIL;
                info->error_hint = "empty xml response";

                free(xml_set_response_buffer);
                goto done;
            }

            /* Parse xml_response looking for an 'error' tag */
            error_ptr = strstr(xml_set_response_buffer, "<error id=");
            if (error_ptr != NULL)
            {
                error_id = CCAPI_GLOBAL_ERROR_XML_RESPONSE_FAIL;
#if 0
                {
                    int scanf_ret;
                    unsigned int response_error_id;

                    printf("error_ptr='%s'\n", error_ptr);

                    scanf_ret = scanf(error_ptr, "<error id=\"%u\">", &response_error_id);
                    printf("scanf_ret=%d, error_id=%d\n", scanf_ret, response_error_id);

                    if (scanf_ret > 0)
                    {
                        /* TODO: take the linux 'desc' or 'hint' as hint */
                        info->error_hint = "xml response has error tag";
                    }
                }
#else
                info->error_hint = "xml response has error tag";
#endif
            }

            if (xml_set_response_buffer != NULL)
            {
                free(xml_set_response_buffer);
            }

            break;
        }
        case CCAPI_RCI_ACTION_QUERY:
        {
            break;
        }
#if (defined RCI_LEGACY_COMMANDS)
        case CCAPI_RCI_ACTION_DO_COMMAND:
        case CCAPI_RCI_ACTION_REBOOT:
        case CCAPI_RCI_ACTION_SET_FACTORY_DEFAULTS:
        {
            /* TODO */
            break;
        }
#endif
    }

done:
    return error_id;
}

static int handle_group_start(ccapi_rci_info_t * const info)
{
    int error_id = CCAPI_GLOBAL_ERROR_NONE;

    switch(info->action)
    {
        case CCAPI_RCI_ACTION_SET:
        {
            fprintf(xml_request_fp, "\n   <%s index=\"%d\">\n      ", info->group.name, info->group.instance);    
            break;
        }
        case CCAPI_RCI_ACTION_QUERY:
        {
            xml_request_fp = fopen(XML_REQUEST_FILE_NAME, "w+");
            if (xml_request_fp == NULL)
            {
                printf("%s: Unable to create %s file\n", __FUNCTION__,  XML_REQUEST_FILE_NAME);
                return CCAPI_GLOBAL_ERROR_XML_REQUEST_FAIL;
            }

            fprintf(xml_request_fp, "<query_");
            print_group(info);

            if (info->group.type == CCAPI_RCI_GROUP_SETTING)
            {
                static char const * const attribute_source[] = {"current", "stored", "defaults"};
                static char const * const attribute_compare_to[] = {"none", "current", "stored", "defaults"};

                fprintf(xml_request_fp, " source=\"%s\"", attribute_source[info->query_setting.attributes.source]);
                fprintf(xml_request_fp, " compare_to=\"%s\"", attribute_compare_to[info->query_setting.attributes.compare_to]);
            }
            fprintf(xml_request_fp, ">");
            fprintf(xml_request_fp, "\n   <%s index=\"%d\"/>", info->group.name, info->group.instance);    
            fprintf(xml_request_fp, "\n</query_");
            print_group(info);
            fprintf(xml_request_fp, ">\n");

            fclose(xml_request_fp);

            {
                char * error_ptr = NULL;

                linux_rci_handle();

                assert(xml_query_response_buffer == NULL);

                error_id = get_response_buffer(&xml_query_response_buffer, info);
                if (error_id != CCAPI_GLOBAL_ERROR_NONE)
                {
                    goto done;
                }

                /* Parse xml_response looking for an 'error' tag */
                error_ptr = strstr(xml_query_response_buffer, "<error id=");
                if (error_ptr != NULL)
                {
                    error_id = CCAPI_GLOBAL_ERROR_XML_RESPONSE_FAIL;
#if 0
                    {
                        int scanf_ret;
                        unsigned int response_error_id;

                        printf("error_ptr='%s'\n", error_ptr);

                        scanf_ret = scanf(error_ptr, "<error id=\"%u\">", &response_error_id);
                        printf("scanf_ret=%d, error_id=%d\n", scanf_ret, response_error_id);

                        if (scanf_ret > 0)
                        {
                            /* TODO: take the linux 'desc' or 'hint' as hint */
                            info->error_hint = "xml response has error tag";
                        }
                    }
#else
                    info->error_hint = "xml response has error tag";
#endif
                }
            }
            break;
        }
#if (defined RCI_LEGACY_COMMANDS)
        case CCAPI_RCI_ACTION_DO_COMMAND:
        case CCAPI_RCI_ACTION_REBOOT:
        case CCAPI_RCI_ACTION_SET_FACTORY_DEFAULTS:
        {
            /* TODO */
            break;
        }
#endif
    }

done:
    return error_id;
}

static int handle_group_end(ccapi_rci_info_t * const info)
{
    switch(info->action)
    {
        case CCAPI_RCI_ACTION_SET:
        {
            fprintf(xml_request_fp, "\n   </%s>", info->group.name);    
            break;
        }
        case CCAPI_RCI_ACTION_QUERY:
        {
            if (xml_query_response_buffer != NULL)
            {
                free(xml_query_response_buffer);
                xml_query_response_buffer = NULL;
            }

            value_end_ptr = NULL;

            break;
        }
#if (defined RCI_LEGACY_COMMANDS)
        case CCAPI_RCI_ACTION_DO_COMMAND:
        case CCAPI_RCI_ACTION_REBOOT:
        case CCAPI_RCI_ACTION_SET_FACTORY_DEFAULTS:
        {
            /* TODO */
            break;
        }
#endif
    }

    return CCAPI_GLOBAL_ERROR_NONE;
}

#if 0
// missing info->element.type !!
static void handle_group_set(ccapi_rci_info_t * const info, ...)
{

    va_list args;

    va_start(args, info);

    switch(info->element.type)
    {
        case connector_element_type_int32:
        {
            int32_t const int_value = va_arg(arg_list, int32_t);

            fprintf(xml_request_fp, "<%s>%d</%s>", info->element.name, int_value, info->element.name);    

            break;
        }
        case connector_element_type_enum:
        case connector_element_type_string:
        {
            char const * const string_value = va_arg(arg_list, char const * const);

            fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, string_value, info->element.name);

            break;
        }
        /* TODO: other types */
    }

    va_end(args);

    return;
}
#endif

#define BRACKET_SIZE 2 /* Size of '<' + '>' */
#define NULL_TERM_SIZE 1

static int get_xml_value(ccapi_rci_info_t * const info, char const * * xml_value)
{
    int error_id = CCAPI_GLOBAL_ERROR_XML_RESPONSE_FAIL;
    char * element_ptr = NULL;
    char * value_start_ptr = NULL;
    char element_name[RCI_ELEMENT_NAME_MAX_SIZE + BRACKET_SIZE + NULL_TERM_SIZE];

    assert(xml_query_response_buffer != NULL);

    if (value_end_ptr != NULL)
    {
        /* Restore previous value_end_ptr */
        *value_end_ptr = '<';
    }

    /* Firs look for empty answer */
    sprintf(element_name, "<%s/>", info->element.name);
    element_ptr = strstr(xml_query_response_buffer, element_name);
    if (element_ptr != NULL)
    {
        *xml_value = "";

        error_id = CCAPI_GLOBAL_ERROR_NONE;
        goto done;
    }

    sprintf(element_name, "<%s>", info->element.name);

    element_ptr = strstr(xml_query_response_buffer, element_name);
    if (element_ptr != NULL)
    {
        value_start_ptr = element_ptr + strlen(info->element.name) + BRACKET_SIZE;
        value_end_ptr = strstr(value_start_ptr, "</");
        if (value_end_ptr != NULL)
        {
            *value_end_ptr = '\0';
            //printf("element xml_value='%s'\n", value_start_ptr);
            *xml_value = value_start_ptr;

            error_id = CCAPI_GLOBAL_ERROR_NONE;
            goto done;
        }
    }

done:
    return error_id;
}

ccapi_setting_serial_error_id_t handle_group_get_string(ccapi_rci_info_t * const info, char const * * const value)
{
    int error_id;
    char const * xml_value = NULL;

    error_id = get_xml_value(info, &xml_value);
    if (error_id != CCAPI_GLOBAL_ERROR_NONE)
    {
        goto done;
    }

    *value = xml_value;

    printf("string_value='%s'\n", *value);

done:
    return error_id;
}

ccapi_setting_serial_error_id_t handle_group_get_unsigned_integer(ccapi_rci_info_t * const info, uint32_t * const value)
{
    int error_id;
    char const * xml_value = NULL;

    error_id = get_xml_value(info, &xml_value);
    if (error_id != CCAPI_GLOBAL_ERROR_NONE)
    {
        goto done;
    }

    *value = (uint32_t)atoi(xml_value);

    printf("unsigned_integer_value='%u'\n", *value);

done:
    return error_id;
}

ccapi_setting_serial_error_id_t handle_group_get_integer(ccapi_rci_info_t * const info, int32_t * const value)
{
    int error_id;
    char const * xml_value = NULL;

    error_id = get_xml_value(info, &xml_value);
    if (error_id != CCAPI_GLOBAL_ERROR_NONE)
    {
        goto done;
    }

    *value = (int32_t)atoi(xml_value);

    printf("integer_value='%d'\n", *value);

done:
    return error_id;
}

ccapi_setting_serial_error_id_t handle_group_get_ccapi_on_off(ccapi_rci_info_t * const info, ccapi_on_off_t * const value)
{
    int error_id;
    char const * xml_value = NULL;

    error_id = get_xml_value(info, &xml_value);
    if (error_id != CCAPI_GLOBAL_ERROR_NONE)
    {
        goto done;
    }

    /* boolean value should be 0, 1, on, off, true or false */
    if (!strcmp(xml_value, "0") || !strcmp(xml_value, "off") || !strcmp(xml_value, "false"))
    {
        *value = CCAPI_OFF;
    }
    else if (!strcmp(xml_value, "1") || !strcasecmp(xml_value, "on") || !strcasecmp(xml_value, "true"))
    {
        *value = CCAPI_ON;
    }
    else
    {
        error_id = CCAPI_GLOBAL_ERROR_XML_RESPONSE_FAIL;
        goto done;
    }

    printf("ccapi_on_off_value='%u'\n", *value);

done:
    return error_id;
}

ccapi_setting_serial_error_id_t handle_group_get_ccapi_bool(ccapi_rci_info_t * const info, ccapi_bool_t * const value)
{
    int error_id;
    char const * xml_value = NULL;

    error_id = get_xml_value(info, &xml_value);
    if (error_id != CCAPI_GLOBAL_ERROR_NONE)
    {
        goto done;
    }

    /* boolean value should be 0, 1, on, off, true or false */
    if (!strcmp(xml_value, "0") || !strcmp(xml_value, "off") || !strcmp(xml_value, "false"))
    {
        *value = CCAPI_FALSE;
    }
    else if (!strcmp(xml_value, "1") || !strcasecmp(xml_value, "on") || !strcasecmp(xml_value, "true"))
    {
        *value = CCAPI_TRUE;
    }
    else
    {
        error_id = CCAPI_GLOBAL_ERROR_XML_RESPONSE_FAIL;
        goto done;
    }

    printf("ccapi_bool_value='%u'\n", *value);

done:
    return error_id;
}

/* TODO: add handle_group_get_*() functions for types not present in this example */

ccapi_setting_serial_error_id_t rci_setting_serial_start(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_start(info);
}

ccapi_setting_serial_error_id_t rci_setting_serial_end(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_end(info);
}

ccapi_setting_serial_error_id_t rci_setting_serial_baud_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_serial_error_id_t rci_setting_serial_baud_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_serial_error_id_t rci_setting_serial_parity_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_serial_error_id_t rci_setting_serial_parity_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_serial_error_id_t rci_setting_serial_databits_get(ccapi_rci_info_t * const info, uint32_t * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_unsigned_integer(info, value);
}

ccapi_setting_serial_error_id_t rci_setting_serial_databits_set(ccapi_rci_info_t * const info, uint32_t const *const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%u</%s>", info->element.name, *value, info->element.name);    
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_serial_error_id_t rci_setting_serial_xbreak_get(ccapi_rci_info_t * const info, ccapi_on_off_t * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_ccapi_on_off(info, value);
}

ccapi_setting_serial_error_id_t rci_setting_serial_xbreak_set(ccapi_rci_info_t * const info, ccapi_on_off_t const *const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, *value ? "on":"off", info->element.name);    
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_serial_error_id_t rci_setting_serial_txbytes_get(ccapi_rci_info_t * const info, uint32_t * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_unsigned_integer(info, value);
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_start(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_start(info);
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_end(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_end(info);
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_ip_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_ip_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_subnet_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_subnet_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_gateway_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_gateway_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_dhcp_get(ccapi_rci_info_t * const info, ccapi_bool_t * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
     return handle_group_get_ccapi_bool(info, value);
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_dhcp_set(ccapi_rci_info_t * const info, ccapi_bool_t const *const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, *value ? "true":"false", info->element.name);    
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_dns_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_dns_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_mac_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_mac_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_duplex_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_ethernet_error_id_t rci_setting_ethernet_duplex_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_device_time_error_id_t rci_setting_device_time_start(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_start(info);
}

ccapi_setting_device_time_error_id_t rci_setting_device_time_end(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_end(info);
}

ccapi_setting_device_time_error_id_t rci_setting_device_time_curtime_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_device_time_error_id_t rci_setting_device_time_curtime_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_device_info_error_id_t rci_setting_device_info_start(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_start(info);
}

ccapi_setting_device_info_error_id_t rci_setting_device_info_end(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_end(info);
}

ccapi_setting_device_info_error_id_t rci_setting_device_info_version_get(ccapi_rci_info_t * const info, uint32_t * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_unsigned_integer(info, value);
}

ccapi_setting_device_info_error_id_t rci_setting_device_info_product_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_device_info_error_id_t rci_setting_device_info_product_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_device_info_error_id_t rci_setting_device_info_model_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_device_info_error_id_t rci_setting_device_info_model_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_device_info_error_id_t rci_setting_device_info_company_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_device_info_error_id_t rci_setting_device_info_company_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_device_info_error_id_t rci_setting_device_info_desc_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_device_info_error_id_t rci_setting_device_info_desc_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_system_error_id_t rci_setting_system_start(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_start(info);
}

ccapi_setting_system_error_id_t rci_setting_system_end(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_end(info);
}

ccapi_setting_system_error_id_t rci_setting_system_description_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_system_error_id_t rci_setting_system_description_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_system_error_id_t rci_setting_system_contact_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_system_error_id_t rci_setting_system_contact_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_system_error_id_t rci_setting_system_location_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_system_error_id_t rci_setting_system_location_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_devicesecurity_error_id_t rci_setting_devicesecurity_start(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_start(info);
}

ccapi_setting_devicesecurity_error_id_t rci_setting_devicesecurity_end(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_end(info);
}

ccapi_setting_devicesecurity_error_id_t rci_setting_devicesecurity_identityVerificationForm_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_setting_devicesecurity_error_id_t rci_setting_devicesecurity_identityVerificationForm_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_setting_devicesecurity_error_id_t rci_setting_devicesecurity_password_set(ccapi_rci_info_t * const info, char const * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, value, info->element.name);
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_state_device_state_error_id_t rci_state_device_state_start(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_start(info);
}

ccapi_state_device_state_error_id_t rci_state_device_state_end(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_end(info);
}

ccapi_state_device_state_error_id_t rci_state_device_state_system_up_time_get(ccapi_rci_info_t * const info, uint32_t * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_unsigned_integer(info, value);
}

ccapi_state_device_state_error_id_t rci_state_device_state_signed_integer_get(ccapi_rci_info_t * const info, int32_t * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_integer(info, value);
}

ccapi_state_device_state_error_id_t rci_state_device_state_signed_integer_set(ccapi_rci_info_t * const info, int32_t const *const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    fprintf(xml_request_fp, "<%s>%d</%s>", info->element.name, *value, info->element.name);    
    return CCAPI_GLOBAL_ERROR_NONE;
}

ccapi_state_gps_stats_error_id_t rci_state_gps_stats_start(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_start(info);
}

ccapi_state_gps_stats_error_id_t rci_state_gps_stats_end(ccapi_rci_info_t * const info)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_end(info);
}

ccapi_state_gps_stats_error_id_t rci_state_gps_stats_latitude_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}

ccapi_state_gps_stats_error_id_t rci_state_gps_stats_longitude_get(ccapi_rci_info_t * const info, char const * * const value)
{
    printf("    Called '%s'\n", __FUNCTION__);
    return handle_group_get_string(info, value);
}
