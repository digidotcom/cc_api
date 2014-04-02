#ifndef _CCAPI_TRANSPORT_TCP_H_
#define _CCAPI_TRANSPORT_TCP_H_


#define CCAPI_KEEPALIVES_RX_MIN         5
#define CCAPI_KEEPALIVES_RX_MAX         7200
#define CCAPI_KEEPALIVES_RX_DEFAULT     75

#define CCAPI_KEEPALIVES_TX_MIN         5
#define CCAPI_KEEPALIVES_TX_MAX         7200
#define CCAPI_KEEPALIVES_TX_DEFAULT     75

#define CCAPI_KEEPALIVES_WCNT_MIN       2
#define CCAPI_KEEPALIVES_WCNT_MAX       64
#define CCAPI_KEEPALIVES_WCNT_DEFAULT   5

typedef enum {
    CCAPI_CONNECTION_LAN,
    CCAPI_CONNECTION_WAN
} ccapi_connection_type_t;

typedef enum {
    CCAPI_TCP_START_ERROR_NONE,
    CCAPI_TCP_START_ERROR_CCAPI_STOPPED,
    CCAPI_TCP_START_ERROR_NULL_POINTER,
    CCAPI_TCP_START_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_TCP_START_ERROR_KEEPALIVES,
    CCAPI_TCP_START_ERROR_IP,
    CCAPI_TCP_START_ERROR_INVALID_MAC,
    CCAPI_TCP_START_ERROR_PHONE,
    CCAPI_TCP_START_ERROR_INIT
} ccapi_tcp_start_error_t;

typedef enum {
    CCAPI_IPV4,
    CCAPI_IPV6
} ccapi_ip_address_type_t;

typedef struct {
    union {
        uint32_t ipv4;
        uint8_t ipv6[IPV6_LENGTH];
    } address;
    ccapi_ip_address_type_t type;
} ccapi_ip_address_t;

typedef enum {
    CCAPI_KEEPALIVE_MISSED,
    CCAPI_KEEPALIVE_RESTORED
} ccapi_keepalive_status_t;

typedef ccapi_bool_t (* ccapi_tcp_close_cb_t)(void);
typedef void (* ccapi_tcp_keepalives_cb_t)(ccapi_keepalive_status_t status);

typedef struct {
    struct {
        uint16_t tx;
        uint16_t rx;
        uint16_t wait_count;
    } keepalives;

    struct {
        uint8_t max_transactions; /* connector_request_id_config_max_transaction */
        char * password;
        ccapi_connection_type_t type;
        union {
            struct {
                uint32_t link_speed;
                char * phone_number;
            } wan;

            struct {
                ccapi_ip_address_t ip;
                uint8_t mac_address[MAC_ADDR_LENGTH];
            } lan;
        } info;
    } connection;

    struct {
        ccapi_tcp_close_cb_t close;
        ccapi_tcp_keepalives_cb_t keepalive;
    } callback;
} ccapi_tcp_info_t;

ccapi_tcp_start_error_t ccapi_start_transport_tcp(ccapi_tcp_info_t const * const tcp_start);

#endif
