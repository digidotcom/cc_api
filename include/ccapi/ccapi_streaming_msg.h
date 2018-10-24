#ifndef _CCAPI_STREAMING_MSG_H_
#define _CCAPI_STREAMING_MSG_H_

typedef unsigned int (*ccapi_streaming_msg_start_session_t)(connector_streaming_msg_session_start_request_t * request);
typedef unsigned int (*ccapi_streaming_msg_poll_session_t)(connector_streaming_msg_poll_request_t * request);
typedef unsigned int (*ccapi_streaming_msg_send_t)(connector_streaming_msg_session_send_data_t * request);
typedef unsigned int (*ccapi_streaming_msg_receive_t)(connector_streaming_msg_session_receive_data_t * request);
typedef unsigned int (*ccapi_streaming_msg_end_session_t)(connector_streaming_msg_session_end_request_t * request);

typedef struct {
	ccapi_streaming_msg_start_session_t start_session;
	ccapi_streaming_msg_poll_session_t poll_session;
	ccapi_streaming_msg_send_t send_data;
	ccapi_streaming_msg_receive_t receive_data;
	ccapi_streaming_msg_end_session_t end_session;
} ccapi_streaming_msg_service_t;

#endif
