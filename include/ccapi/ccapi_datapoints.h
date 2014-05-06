#ifndef _CCAPI_DATAPOINTS_H_
#define _CCAPI_DATAPOINTS_H_

#ifdef CCIMP_DATA_SERVICE_ENABLED

#define CCAPI_NO_ELEVATION   -6378000 /* Negative maximum Earth's radio */

typedef enum {
    CCAPI_DP_ERROR_NONE,
    CCAPI_DP_ERROR_INVALID_ARGUMENT,
    CCAPI_DP_ERROR_INSUFFICIENT_MEMORY,
    CCAPI_DP_ERROR_SYNCR_FAILED
} ccapi_dp_error_t;

typedef enum {
    CCAPI_CSV_DATA,
    CCAPI_CSV_TIMESTAMP,
    CCAPI_CSV_QUALITY,
    CCAPI_CSV_DESCRIPTION,
    CCAPI_CSV_LOCATION,
    CCAPI_CSV_TYPE,
    CCAPI_CSV_UNITS,
    CCAPI_CSV_FORWARD_TO,
    CCAPI_CSV_STREAM_ID,
    CCAPI_CSV_COUNT
} csv_fields_t;

typedef enum {
    CCAPI_TS_CLOUD,
    CCAPI_TS_EPOCH,
    CCAPI_TS_EPOCH_MSEC,
    CCAPI_TS_ISO8601
} ccapi_timestamp_t;

typedef struct {
    float latitude;
    float longitude;
    float elevation;
} ccapi_location_t;

typedef struct {
    char const * stream_id;
    char const * unit;
    char const * forward_to;
    enum {
        INT32,
        INT64,
        FLOAT,
        DOUBLE,
        STRING,
        BINARY
    } type;
    csv_fields_t csv_order[CCAPI_CSV_COUNT];
    size_t csv_fields_count;
    ccapi_timestamp_t timestamp_format;
    ccapi_bool_t location_present;
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
            uint32_t epoch;
            uint32_t msec;
        } epoch;
        uint64_t epoch_msec;
        char const * iso8601;
    } timestamp;

    ccapi_location_t location;
    int quality;

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

ccapi_dp_error_t ccapi_dp_create_data_stream(ccapi_dp_data_stream_t * const stream_info, char const * const stream_id, char const * const format_string);
ccapi_dp_error_t ccapi_dp_destroy_data_stream(ccapi_dp_data_stream_t stream_info);

ccapi_dp_error_t ccapi_dp_add(ccapi_dp_collection_t collection, ccapi_dp_data_stream_t stream_info, ...);
#endif

#endif
