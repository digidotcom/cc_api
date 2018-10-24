#define CCAPI_CONST_PROTECTION_UNLOCK

#include "ccapi_definitions.h"

#if (defined CCIMP_STREAMING_MSG_SERVICE_ENABLED)

#define run_callback_if_available(cb, arg)	(cb) != NULL ? (cb)((arg)) : connector_callback_continue

connector_callback_status_t ccapi_streaming_msg_handler(connector_request_id_streaming_msg_service_t const request, void * const data, ccapi_data_t * const ccapi_data)
{
	connector_callback_status_t status;
	ccapi_streaming_msg_service_t * callbacks = &ccapi_data->service.streaming_msg.user_callback;

	switch (request)
	{
		case connector_request_id_streaming_msg_session_start:
			status = run_callback_if_available(callbacks->start_session, data);
			break;
		case connector_request_id_streaming_msg_poll:
			status = run_callback_if_available(callbacks->poll_session, data);
			break;
		case connector_request_id_streaming_msg_send:
			status = run_callback_if_available(callbacks->send_data, data);
			break;
		case connector_request_id_streaming_msg_receive:
			status = run_callback_if_available(callbacks->receive_data, data);
			break;
		case connector_request_id_streaming_msg_session_end:
			status = run_callback_if_available(callbacks->end_session, data);
			break;
	}

	return status;
}

#endif
