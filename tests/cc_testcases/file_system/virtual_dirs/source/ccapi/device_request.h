
/* Device request */
extern void ccapi_device_request_status_callback(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t * const response_buffer_info, ccapi_receive_error_t receive_error);
extern void ccapi_device_request_data_callback(char const * const target, ccapi_transport_t const transport, ccapi_buffer_info_t const * const request_buffer_info, ccapi_buffer_info_t * const response_buffer_info, ccapi_receive_error_t receive_error);
extern ccapi_bool_t ccapi_device_request_accept_callback(char const * const target, ccapi_transport_t const transport);
extern void fill_device_request_service(ccapi_start_t * start);