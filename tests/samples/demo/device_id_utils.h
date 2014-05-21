#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void get_device_id_from_mac(uint8_t * const device_id, uint8_t const * const mac_addr);
void get_device_id_from_imei(uint8_t * const device_id, uint8_t const * const imei);
void get_device_id_from_meid(uint8_t * const device_id, uint8_t const * const meid);
void get_device_id_from_esn(uint8_t * const device_id, uint8_t const * const esn);
void get_mac_from_string(uint8_t * const mac_addr, char const * const mac_addr_string);
void get_imei_from_string(uint8_t * const imei, char const * const imei_string);
void get_meid_from_string(uint8_t * const meid, char const * const meid_string);
void get_esn_from_string(uint8_t * const esn, char const * const esn_string);
void get_ipv4_from_string(uint8_t * const ipv4, char const * const ipv4_string);
