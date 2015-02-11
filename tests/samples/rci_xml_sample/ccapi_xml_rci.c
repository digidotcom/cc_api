#include  <stdio.h>
#include  <sys/stat.h>
#include  <unistd.h>
#include  <stdarg.h>

#include  "ccapi_xml_rci.h"
#include  "ccapi_xml_rci_handler.h"

#define xstr(s) str(s)
#define str(s) #s

static FILE * xml_request_fp = NULL;
static char * xml_query_response_buffer = NULL;

static ccapi_global_error_id_t process_xml_error(ccapi_rci_info_t * const info, char * xml_response_buffer)
{
    ccapi_global_error_id_t error_id = CCAPI_GLOBAL_ERROR_NONE;

    char * error_ptr = NULL;

    /* Parse xml_response looking for an 'error' tag */
    error_ptr = strstr(xml_response_buffer, "<error id=");
    if (error_ptr != NULL)
    {
        int scanf_ret;
        unsigned int response_error_id;

        char * response_error_desc = malloc(XML_MAX_ERROR_DESC_LENGTH + 1);
        char * response_error_hint = malloc(XML_MAX_ERROR_HINT_LENGTH + 1);

        error_id = CCAPI_GLOBAL_ERROR_XML_RESPONSE_FAIL;

        if (response_error_desc == NULL || response_error_hint == NULL)
        {
            goto done;
        }

        #define XML_MAX_ERROR_DESC_LENGTH_STR  xstr(XML_MAX_ERROR_DESC_LENGTH)
        #define XML_MAX_ERROR_HINT_LENGTH_STR  xstr(XML_MAX_ERROR_HINT_LENGTH)

        #define XML_ERROR_FORMAT "<error id=\"%u\">%*[^<]<desc>%" XML_MAX_ERROR_DESC_LENGTH_STR "[^<]</desc>%*[^<]<hint>%" XML_MAX_ERROR_HINT_LENGTH_STR "[^<]</hint>"

        scanf_ret = sscanf(error_ptr, XML_ERROR_FORMAT, &response_error_id, response_error_desc, response_error_hint);
        if (scanf_ret > 0)
        {
            static char const brci_error_prefix[] = "XML Error: id='%d', desc='%s', hint='%s'";
            static char response_error[sizeof(brci_error_prefix) + XML_MAX_ERROR_DESC_LENGTH + 1 + XML_MAX_ERROR_HINT_LENGTH + 1];

            sprintf(response_error, brci_error_prefix, response_error_id, response_error_desc, response_error_hint);

            /* Take the xml error 'id' + 'desc' + 'hint' as hint */
            info->error_hint = response_error;

            printf("%s\n", response_error);
        }

        free(response_error_desc);
        free(response_error_hint);
    }

done:
    return error_id;
}

static void write_group(ccapi_rci_info_t * const info)
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
 
ccapi_global_error_id_t ccapi_xml_rci_action_start(ccapi_rci_info_t * const info)
{
    ccapi_global_error_id_t error_id = CCAPI_GLOBAL_ERROR_NONE;

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
            write_group(info);
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

    /* printf("response:\n%s\n", *xml_response_buffer); */

    fclose(xml_response_fp);

done:
    return error_id;
}

ccapi_global_error_id_t ccapi_xml_rci_action_end(ccapi_rci_info_t * const info)
{
    ccapi_global_error_id_t error_id = CCAPI_GLOBAL_ERROR_NONE;

    switch(info->action)
    {
        case CCAPI_RCI_ACTION_SET:
        {
            fprintf(xml_request_fp, "\n</set_");
            write_group(info);
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

            xml_rci_handler();

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

            error_id = process_xml_error(info, xml_set_response_buffer);

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

int ccapi_xml_rci_group_start(ccapi_rci_info_t * const info)
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
            write_group(info);

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
            write_group(info);
            fprintf(xml_request_fp, ">\n");

            fclose(xml_request_fp);

            {
                xml_rci_handler();

                assert(xml_query_response_buffer == NULL);

                error_id = get_response_buffer(&xml_query_response_buffer, info);
                if (error_id != CCAPI_GLOBAL_ERROR_NONE)
                {
                    goto done;
                }

                error_id = process_xml_error(info, xml_query_response_buffer);
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

int ccapi_xml_rci_group_end(ccapi_rci_info_t * const info)
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

int ccapi_xml_rci_group_set(ccapi_rci_info_t * const info, ...)
{

    va_list arg_list;

    va_start(arg_list, info);

    switch(info->element.type)
    {
        case CCAPI_RCI_ELEMENT_TYPE_NOT_SET:
            break;
#if (defined RCI_PARSER_USES_STRINGS)
#if (defined RCI_PARSER_USES_STRING)
        case CCAPI_RCI_ELEMENT_TYPE_STRING:
#endif
#if (defined RCI_PARSER_USES_MULTILINE_STRING)
        case CCAPI_RCI_ELEMENT_TYPE_MULTILINE_STRING:
#endif
#if (defined RCI_PARSER_USES_PASSWORD)
        case CCAPI_RCI_ELEMENT_TYPE_PASSWORD:
#endif
#if (defined RCI_PARSER_USES_FQDNV4)
        case CCAPI_RCI_ELEMENT_TYPE_FQDNV4:
#endif
#if (defined RCI_PARSER_USES_FQDNV6)
        case CCAPI_RCI_ELEMENT_TYPE_FQDNV6:
#endif
#if (defined RCI_PARSER_USES_DATETIME)
        case CCAPI_RCI_ELEMENT_TYPE_DATETIME:
#endif
#if (defined RCI_PARSER_USES_IPV4)
        case CCAPI_RCI_ELEMENT_TYPE_IPV4:
#endif
#if (defined RCI_PARSER_USES_MAC_ADDR)
        case CCAPI_RCI_ELEMENT_TYPE_MAC:
#endif
#if (defined RCI_PARSER_USES_ENUM)
        case CCAPI_RCI_ELEMENT_TYPE_ENUM:
#endif
        {
            char const * const string_value = va_arg(arg_list, char const * const);

            fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, string_value, info->element.name);

            break;
        }
#endif
#if (defined RCI_PARSER_USES_INT32)
        case CCAPI_RCI_ELEMENT_TYPE_INT32:
        {
            int32_t const * const p_int32_t = va_arg(arg_list, int32_t const * const);

            fprintf(xml_request_fp, "<%s>%d</%s>", info->element.name, *p_int32_t, info->element.name);    

            break;
        }
#endif
#if (defined RCI_PARSER_USES_UNSIGNED_INTEGER)
#if (defined RCI_PARSER_USES_UINT32)
        case CCAPI_RCI_ELEMENT_TYPE_UINT32:
#endif
#if (defined RCI_PARSER_USES_HEX32)
        case CCAPI_RCI_ELEMENT_TYPE_HEX32:
#endif
#if (defined RCI_PARSER_USES_0X_HEX32)
        case CCAPI_RCI_ELEMENT_TYPE_0X32:
#endif
        {
            uint32_t const * const p_uint32_t = va_arg(arg_list, uint32_t const * const);

            fprintf(xml_request_fp, "<%s>%d</%s>", info->element.name, *p_uint32_t, info->element.name);    

            break;
        }
#endif
#if (defined RCI_PARSER_USES_FLOAT)
        case CCAPI_RCI_ELEMENT_TYPE_FLOAT:
            /* TODO */
            break;
#endif
#if (defined RCI_PARSER_USES_ON_OFF)
        case CCAPI_RCI_ELEMENT_TYPE_ON_OFF:
        {
            ccapi_on_off_t const * const p_ccapi_on_off_t = va_arg(arg_list, ccapi_on_off_t const * const);

            fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, *p_ccapi_on_off_t ? "on":"off", info->element.name);    

            break;
        }    
#endif
#if (defined RCI_PARSER_USES_BOOLEAN)
        case CCAPI_RCI_ELEMENT_TYPE_BOOL:
        {
            ccapi_bool_t const * const p_ccapi_bool_t = va_arg(arg_list, ccapi_bool_t const * const);

            fprintf(xml_request_fp, "<%s>%s</%s>", info->element.name, *p_ccapi_bool_t ? "true":"false", info->element.name);    

            break;
        }        
#endif
    }

    va_end(arg_list);

    return CCAPI_GLOBAL_ERROR_NONE;
}

#define BRACKET_SIZE 2 /* Size of '<' + '>' */
#define NULL_TERM_SIZE 1

static int get_xml_value(ccapi_rci_info_t * const info, char const * * xml_value)
{
    int error_id = CCAPI_GLOBAL_ERROR_XML_RESPONSE_FAIL;
    char * element_ptr = NULL;
    char element_name[RCI_ELEMENT_NAME_MAX_SIZE + BRACKET_SIZE + NULL_TERM_SIZE];

    assert(xml_query_response_buffer != NULL);

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
        int scanf_ret;
        static char value[XML_MAX_VALUE_LENGTH + 1];

        #define XML_MAX_VALUE_LENGTH_STR  xstr(XML_MAX_VALUE_LENGTH)

        #define XML_VALUE_FORMAT "%*[^>]>%" XML_MAX_VALUE_LENGTH_STR "[^<]</"
        scanf_ret = sscanf(element_ptr, XML_VALUE_FORMAT, value);
        if (scanf_ret > 0)
        {
            /* printf("element xml_value='%s'\n", value); */
            *xml_value = value;

            error_id = CCAPI_GLOBAL_ERROR_NONE;
            goto done;
        }
    }

done:
    return error_id;
}

int ccapi_xml_rci_group_get_string(ccapi_rci_info_t * const info, char const * * const value)
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

int ccapi_xml_rci_group_get_unsigned_integer(ccapi_rci_info_t * const info, uint32_t * const value)
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

int ccapi_xml_rci_group_get_integer(ccapi_rci_info_t * const info, int32_t * const value)
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

int ccapi_xml_rci_group_get_ccapi_on_off(ccapi_rci_info_t * const info, ccapi_on_off_t * const value)
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

int ccapi_xml_rci_group_get_ccapi_bool(ccapi_rci_info_t * const info, ccapi_bool_t * const value)
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

/* TODO: add ccapi_xml_rci_group_get_*() functions for types not present in this example */

