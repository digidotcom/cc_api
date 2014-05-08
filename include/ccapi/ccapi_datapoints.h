#ifndef _CCAPI_DATAPOINTS_H_
#define _CCAPI_DATAPOINTS_H_

#ifdef CCIMP_DATA_SERVICE_ENABLED

#define CCAPI_NO_ELEVATION   -6378000 /* Negative maximum Earth's radio */

#define CCAPI_DP_KEY_DATA_INT32     "int32"
#define CCAPI_DP_KEY_DATA_INT64     "int64"
#define CCAPI_DP_KEY_DATA_FLOAT     "float"
#define CCAPI_DP_KEY_DATA_DOUBLE    "double"
#define CCAPI_DP_KEY_DATA_STRING    "string"

#define CCAPI_DP_KEY_TS_EPOCH       "ts_epoch"
#define CCAPI_DP_KEY_TS_EPOCHMS     "ts_epoch_ms"
#define CCAPI_DP_KEY_TS_ISO8601     "ts_iso"

#define CCAPI_DP_KEY_LOCATION       "loc"
#define CCAPI_DP_KEY_QUALITY        "qual"

typedef enum {
    CCAPI_DP_ERROR_NONE,
    CCAPI_DP_ERROR_INVALID_ARGUMENT,
    CCAPI_DP_ERROR_INVALID_STREAM_ID,
    CCAPI_DP_ERROR_INVALID_FORMAT,
    CCAPI_DP_ERROR_INVALID_UNITS,
    CCAPI_DP_ERROR_INVALID_FORWARD_TO,
    CCAPI_DP_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_DP_ERROR_SYNCR_FAILED
} ccapi_dp_error_t;

typedef enum {
    CCAPI_DP_ARG_DATA_INT32,
    CCAPI_DP_ARG_DATA_INT64,
    CCAPI_DP_ARG_DATA_FLOAT,
    CCAPI_DP_ARG_DATA_DOUBLE,
    CCAPI_DP_ARG_DATA_STRING,
    CCAPI_DP_ARG_TIME_EPOCH,
    CCAPI_DP_ARG_TIME_EPOCH_MSEC,
    CCAPI_DP_ARG_TIME_ISO8601,
    CCAPI_DP_ARG_LOC,
    CCAPI_DP_ARG_QUAL,
    CCAPI_DP_ARG_INVALID
} ccapi_dp_argument_t;

typedef struct {
    float latitude;
    float longitude;
    float elevation;
} ccapi_location_t;

typedef struct {
    char const * stream_id;
    char const * units;
    char const * forward_to;
    struct {
        ccapi_dp_argument_t * list;
        size_t count;
    } arguments;
} ccapi_dp_data_stream_t;

typedef struct ccapi_data_point {
    union {
        int32_t int32;
        int64_t int64;
        char const * string;
        float flt;
        double dbl;
    } data;

    union {
        struct {
            uint32_t seconds;
            uint32_t milliseconds;
        } epoch;
        uint64_t epoch_msec;
        char const * iso8601;
    } timestamp;

    ccapi_location_t location;
    int32_t quality;

    struct ccapi_data_point * next;
} ccapi_data_point_t;

typedef struct stream_seen {
    char * stream_id;
    struct stream_seen * next;
} stream_seen_t;

typedef struct {
    ccapi_data_point_t * data_points_list;
    void * syncr;
} ccapi_dp_collection_t;

ccapi_dp_error_t ccapi_dp_create_collection(ccapi_dp_collection_t * * const dp_collection);
ccapi_dp_error_t ccapi_dp_clear_collection(ccapi_dp_collection_t * const dp_collection);
ccapi_dp_error_t ccapi_dp_destroy_collection(ccapi_dp_collection_t * const dp_collection);
ccapi_dp_error_t ccapi_dp_send_collection(ccapi_transport_t transport, ccapi_dp_collection_t * const dp_collection);

ccapi_dp_error_t ccapi_dp_create_data_stream(ccapi_dp_data_stream_t * * const stream_info, char const * const stream_id, char const * const format_string);
ccapi_dp_error_t ccapi_dp_create_data_stream_extra(ccapi_dp_data_stream_t * * const stream_info, char const * const stream_id, char const * const format_string, char const * const units, char const * const forward_to);
ccapi_dp_error_t ccapi_dp_destroy_data_stream(ccapi_dp_data_stream_t * const stream_info);

ccapi_dp_error_t ccapi_dp_add(ccapi_dp_collection_t collection, ccapi_dp_data_stream_t stream_info, ...);
#endif

#endif
