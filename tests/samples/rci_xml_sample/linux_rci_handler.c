#include <stdio.h>
#include "rci_xml_sample.h"

#define GOOD_RESPONSE 	\
       "<set_setting> 	\
          <serial>		\
            <baud/>		\
            <parity/>	\
            <xbreak/>	\
            <databits/>	\
          </serial>		\
          <ethernet>	\
            <ip/>		\
            <dhcp/>		\
          </ethernet>	\
        </set_setting>"

#define BAD_RESPONSE 	\
       "<set_setting> 	\
          <serial>		\
            <baud>		\
		       <error id=\"1\"> \
               <desc>linux_error_desc</desc> \
               <hint>linux_error_hint</hint> \
               </error> \
            </baud>		\
            <parity/>	\
            <xbreak/>	\
            <databits/>	\
          </serial>		\
          <ethernet>	\
            <ip/>		\
            <dhcp/>		\
          </ethernet>	\
        </set_setting>"

void linux_rci_handle_set(void)
{
    FILE * xml_request_fp = NULL;
    FILE * xml_response_fp = NULL;
    static unsigned int rnd_response = 0;

    printf("    Called '%s'\n", __FUNCTION__);

    xml_request_fp = fopen(XML_REQUEST_FILE_NAME, "r");
    if (xml_request_fp == NULL)
    {
        printf("%s: Unable to open %s file\n", __FUNCTION__,  XML_REQUEST_FILE_NAME);
    }

    xml_response_fp = fopen(XML_RESPONSE_FILE_NAME, "w+");
    if (xml_response_fp == NULL)
    {
        printf("%s: Unable to open %s file\n", __FUNCTION__,  XML_RESPONSE_FILE_NAME);
    }

    /* TODO: process the XML_REQUEST_FILE_NAME file and provide a response at XML_RESPONSE_FILE_NAME */

    if (rnd_response++ % 2)
        fprintf(xml_response_fp, "%s", BAD_RESPONSE);
    else
        fprintf(xml_response_fp, "%s", GOOD_RESPONSE);


    if (xml_request_fp != NULL)
        fclose(xml_request_fp);

    if (xml_response_fp != NULL)
        fclose(xml_response_fp);

    return;
}

