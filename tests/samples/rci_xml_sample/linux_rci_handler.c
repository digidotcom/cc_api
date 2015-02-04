#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "ccapi_xml_rci.h"

#define SET_GOOD_RESPONSE \
       "<set_setting>     \
          <serial>        \
            <baud/>       \
            <parity/>     \
            <xbreak/>     \
            <databits/>   \
          </serial>       \
          <ethernet>      \
            <ip/>         \
            <dhcp/>       \
          </ethernet>     \
        </set_setting>"

#define SET_BAD_RESPONSE  \
       "<set_setting>     \
          <serial>        \
            <baud>        \
               <error id=\"1\"> \
               <desc>linux_error_desc</desc> \
               <hint>linux_error_hint</hint> \
               </error>   \
            </baud>       \
            <parity/>     \
            <xbreak/>     \
            <databits/>   \
          </serial>       \
          <ethernet>      \
            <ip/>         \
            <dhcp/>       \
          </ethernet>     \
        </set_setting>"

#define QUERY_SETTING_SERIAL_RESPONSE \
       "<query_setting>            \
          <serial index=\"2\">     \
            <baud>2400</baud>      \
            <parity>none</parity>  \
            <databits>5</databits> \
            <xbreak>on</xbreak>    \
            <txbytes>123</txbytes> \
          </serial>                \
        </query_setting>"

#define QUERY_SETTING_ETHERNET_RESPONSE \
       "<query_setting>            \
          <ethernet index=\"1\">   \
            <ip>0.0.0.0</ip>       \
            <subnet>0.0.0.0</subnet>     \
            <gateway>0.0.0.0</gateway>   \
            <dhcp>true</dhcp>            \
            <dns/>                       \
            <mac>00:00:00:00:00:00</mac> \
            <duplex>auto</duplex>        \
          </ethernet>                    \
        </query_setting>"

#define QUERY_SETTING_DEVICE_TIME_RESPONSE \
       "<query_setting>            \
          <device_time> \
            <curtime>2015-02-04T11:41:24+-100</curtime> \
          </device_time> \
        </query_setting>"

#define QUERY_SETTING_DEVICE_INFO_RESPONSE \
       "<query_setting>            \
          <device_info> \
            <version>0x2020000</version> \
            <product>Cloud Connector Product</product> \
            <model/> \
            <company>Digi International Inc.</company> \
            <desc>Cloud Connector Demo on Linux \
              with firmware upgrade, and remote configuration supports</desc> \
          </device_info> \
        </query_setting>"

#define QUERY_SETTING_SYSTEM_RESPONSE \
       "<query_setting>            \
          <system> \
            <description/> \
            <contact/> \
            <location/> \
          </system> \
        </query_setting>"

#define QUERY_SETTING_DEVICESECURITY_RESPONSE \
       "<query_setting>            \
          <devicesecurity> \
            <identityVerificationForm>simple</identityVerificationForm> \
          </devicesecurity> \
        </query_setting>"

#define QUERY_STATE_DEVICE_STATE_RESPONSE \
       "<query_state>            \
          <device_state> \
            <system_up_time>9</system_up_time> \
            <signed_integer>-10</signed_integer> \
          </device_state> \
        </query_state>"

#define QUERY_STATE_GPS_STATS_RESPONSE \
       "<query_state>            \
          <gps_stats> \
            <latitude>44.932017</latitude> \
            <longitude>-93.461594</longitude> \
          </gps_stats> \
        </query_state>"

#define QUERY_BAD_RESPONSE     \
       "<query_setting>        \
          <serial>             \
             <error id=\"1\">  \
             <desc>linux_error_desc</desc> \
             <hint>linux_error_hint</hint> \
             </error>          \
          </serial>            \
        </query_setting>"

static unsigned int rnd_set_response = 0;
static unsigned int rnd_query_response = 0;

static int get_request_buffer(char * * xml_request_buffer)
{
    int error_id = 0;
    struct stat request_info;
    FILE * xml_request_fp = NULL;

    stat(XML_REQUEST_FILE_NAME, &request_info);

    *xml_request_buffer = malloc(request_info.st_size + 1);

    if (*xml_request_buffer == NULL)
    {
        error_id = -1;
        goto done;
    }

    xml_request_fp = fopen(XML_REQUEST_FILE_NAME, "r");
    if (xml_request_fp == NULL)
    {
        printf("%s: Unable to open %s file\n", __FUNCTION__,  XML_REQUEST_FILE_NAME);
        error_id = -1;
        goto done;
    }          

    fread(*xml_request_buffer, 1, request_info.st_size, xml_request_fp);
    (*xml_request_buffer)[request_info.st_size] = '\0';
    if (ferror(xml_request_fp) != 0)
    {
        printf("%s: Failed to read %s file\n", __FUNCTION__, XML_REQUEST_FILE_NAME);

        error_id = -2;
        goto done;
    }
    else
    {
       printf("request='%s'\n", *xml_request_buffer);
    }          

    fclose(xml_request_fp);

done:
    return error_id;
}


void linux_rci_handle(void)
{
    char * xml_request_buffer = NULL;
    FILE * xml_response_fp = NULL;
    char * group_ptr = NULL;

    printf("    Called '%s'\n", __FUNCTION__);

    if (get_request_buffer(&xml_request_buffer) != 0)
    {
        goto done;
    }

    xml_response_fp = fopen(XML_RESPONSE_FILE_NAME, "w+");
    if (xml_response_fp == NULL)
    {
        printf("%s: Unable to open %s file\n", __FUNCTION__,  XML_RESPONSE_FILE_NAME);
        goto done;
    }

    /* process the XML_REQUEST_FILE_NAME file and provide a response at XML_RESPONSE_FILE_NAME */
    if (strncmp(xml_request_buffer, "<query_setting", sizeof("<query_setting") - 1) == 0)
    {
        group_ptr = strstr(xml_request_buffer, "   <");
        if (group_ptr != NULL)
        {
            group_ptr += sizeof("   ") - 1;
            if (strncmp(group_ptr, "<serial", sizeof("<serial") - 1) == 0)
            {
                fprintf(xml_response_fp, "%s", QUERY_SETTING_SERIAL_RESPONSE);
            }
            else if (strncmp(group_ptr, "<ethernet", sizeof("<ethernet") -1 ) == 0)
            {
                /* Just a test: every two query request for the group 'ethernet' return an error */
                if (rnd_query_response++ % 2)
                {
                    fprintf(xml_response_fp, "%s", QUERY_BAD_RESPONSE);
                }
                else
                {
                    fprintf(xml_response_fp, "%s", QUERY_SETTING_ETHERNET_RESPONSE);
                }
            }
            else if (strncmp(group_ptr, "<device_time", sizeof("<device_time") -1 ) == 0)
            {
                fprintf(xml_response_fp, "%s", QUERY_SETTING_DEVICE_TIME_RESPONSE);
            }
            else if (strncmp(group_ptr, "<device_info", sizeof("<device_info") -1 ) == 0)
            {
                fprintf(xml_response_fp, "%s", QUERY_SETTING_DEVICE_INFO_RESPONSE);
            }
            else if (strncmp(group_ptr, "<system", sizeof("<system") -1 ) == 0)
            {
                fprintf(xml_response_fp, "%s", QUERY_SETTING_SYSTEM_RESPONSE);
            }
            else if (strncmp(group_ptr, "<devicesecurity", sizeof("<devicesecurity") -1 ) == 0)
            {
                fprintf(xml_response_fp, "%s", QUERY_SETTING_DEVICESECURITY_RESPONSE);
            }
        }
    }
    else if (strncmp(xml_request_buffer, "<query_state", sizeof("<query_state") - 1) == 0)
    {
        group_ptr = strstr(xml_request_buffer, "   <");
        if (group_ptr != NULL)
        {
            group_ptr += sizeof("   ") - 1;
            if (strncmp(group_ptr, "<device_state", sizeof("<device_state") - 1) == 0)
            {
                fprintf(xml_response_fp, "%s", QUERY_STATE_DEVICE_STATE_RESPONSE);
            }
            else if (strncmp(group_ptr, "<gps_stats", sizeof("<gps_stats") -1 ) == 0)
            {
                fprintf(xml_response_fp, "%s", QUERY_STATE_GPS_STATS_RESPONSE);
            }
        }
    }
    else if (strncmp(xml_request_buffer, "<set_setting", sizeof("<set_setting") - 1) == 0)
    {
        /* Don't mind the group set in the request... just provide a response (for 'ethernet' group for example)
           with or without 'error' tag randomly */
        if (rnd_set_response++ % 2)
            fprintf(xml_response_fp, "%s", SET_BAD_RESPONSE);
        else
            fprintf(xml_response_fp, "%s", SET_GOOD_RESPONSE);
    }
    else if (strncmp(xml_request_buffer, "<set_state", sizeof("<set_state") - 1) == 0)
    {
        /* Don't mind the group set in the request... just provide a response (for 'ethernet' group for example) */
        fprintf(xml_response_fp, "%s", SET_GOOD_RESPONSE);
    }
    else
    {
    }

done:
    if (xml_response_fp != NULL)
        fclose(xml_response_fp);

    if (xml_request_buffer != NULL)
        free(xml_request_buffer);

    return;
}

