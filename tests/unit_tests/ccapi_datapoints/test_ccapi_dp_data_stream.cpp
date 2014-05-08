#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_dp_data_stream)
{
    static ccapi_send_error_t error;

    void setup()
    {
        Mock_create_all();
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_dp_data_stream, testDataStreamInvalidArguments)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_create_data_stream(NULL, "stream_1", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);
}

TEST(test_ccapi_dp_data_stream, testDataStreamInvalidStreamID)
{
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    dp_error = ccapi_dp_create_data_stream(&data_stream, NULL, "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_STREAM_ID, dp_error);
    CHECK(data_stream == NULL);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_STREAM_ID, dp_error);
    CHECK(data_stream == NULL);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "s p a c e s", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_STREAM_ID, dp_error);
    CHECK(data_stream == NULL);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "s&mb0/5", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_STREAM_ID, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamInvalidFormatString)
{
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream_1", NULL);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(data_stream == NULL);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream_1", "");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(data_stream == NULL);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream_1", " int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(data_stream == NULL);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream_1", "int32, ts_epoch / loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamExtraInvalidForwardTo)
{
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    dp_error = ccapi_dp_create_data_stream_extra(&data_stream, "stream_1", "int32 ts_epoch loc", NULL, "");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORWARD_TO, dp_error);
    CHECK(data_stream == NULL);

    dp_error = ccapi_dp_create_data_stream_extra(&data_stream, "stream_1", "int32 ts_epoch loc", NULL, "s p a c e s");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORWARD_TO, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamExtraInvalidUnits)
{
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    dp_error = ccapi_dp_create_data_stream_extra(&data_stream, "stream_1", "int32 ts_epoch loc", "", "stream_2");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_UNITS, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamNoMemory4FormatString)
{
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(sizeof "int32 ts_epoch loc", TH_MALLOC_RETURN_NULL, false);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream_1", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamNoMemory4DataStream)
{
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(sizeof "int32 ts_epoch loc", TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(3 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NULL, false);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream_1", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamIDOK)
{
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream_1", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream-1", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "relative+absolute", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "parent/child", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream[1]", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream.1", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "!stream", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}

TEST(test_ccapi_dp_data_stream, testDataStreamFormatStringInvalidKeyWord)
{
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream_1", "int8");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamNoMemory4ArgList)
{
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(sizeof "int32", TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(1 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NULL, false);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream_1", "int32");
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamDataInt32)
{
    char const * const stream_id = "stream_1";
    char const * const format_string = CCAPI_DP_KEY_DATA_INT32;
    ccapi_dp_argument_t const expected_type = CCAPI_DP_ARG_DATA_INT32;
    int const expected_arg_count = 1;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(1 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_create_data_stream(&data_stream, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(data_stream != NULL);
    CHECK_EQUAL(expected_type, get_dp_type_from_arg_list(data_stream->arguments.list, data_stream->arguments.count));
    CHECK_EQUAL(expected_arg_count, data_stream->arguments.count);
    CHECK_EQUAL(CCAPI_DP_ARG_DATA_INT32, data_stream->arguments.list[0]);
    STRCMP_EQUAL(stream_id, data_stream->stream_id);
    CHECK(NULL == data_stream->forward_to);
    CHECK(NULL == data_stream->units);
}

TEST(test_ccapi_dp_data_stream, testDataStreamDataInt64)
{
    char const * const stream_id = "stream_1";
    char const * const format_string = CCAPI_DP_KEY_DATA_INT64;
    ccapi_dp_argument_t const expected_type = CCAPI_DP_ARG_DATA_INT64;
    int const expected_arg_count = 1;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(1 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_create_data_stream(&data_stream, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(data_stream != NULL);
    CHECK_EQUAL(expected_type, get_dp_type_from_arg_list(data_stream->arguments.list, data_stream->arguments.count));
    CHECK_EQUAL(expected_arg_count, data_stream->arguments.count);
    CHECK_EQUAL(CCAPI_DP_ARG_DATA_INT64, data_stream->arguments.list[0]);
    STRCMP_EQUAL(stream_id, data_stream->stream_id);
    CHECK(NULL == data_stream->forward_to);
    CHECK(NULL == data_stream->units);
}

TEST(test_ccapi_dp_data_stream, testDataStreamDataFloat)
{
    char const * const stream_id = "stream_1";
    char const * const format_string = CCAPI_DP_KEY_DATA_FLOAT;
    ccapi_dp_argument_t const expected_type = CCAPI_DP_ARG_DATA_FLOAT;
    int const expected_arg_count = 1;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(1 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_create_data_stream(&data_stream, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(data_stream != NULL);
    CHECK_EQUAL(expected_type, get_dp_type_from_arg_list(data_stream->arguments.list, data_stream->arguments.count));
    CHECK_EQUAL(expected_arg_count, data_stream->arguments.count);
    CHECK_EQUAL(CCAPI_DP_ARG_DATA_FLOAT, data_stream->arguments.list[0]);
    STRCMP_EQUAL(stream_id, data_stream->stream_id);
    CHECK(NULL == data_stream->forward_to);
    CHECK(NULL == data_stream->units);
}

TEST(test_ccapi_dp_data_stream, testDataStreamDataDouble)
{
    char const * const stream_id = "stream_1";
    char const * const format_string = CCAPI_DP_KEY_DATA_DOUBLE;
    ccapi_dp_argument_t const expected_type = CCAPI_DP_ARG_DATA_DOUBLE;
    int const expected_arg_count = 1;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(1 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_create_data_stream(&data_stream, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(data_stream != NULL);
    CHECK_EQUAL(expected_type, get_dp_type_from_arg_list(data_stream->arguments.list, data_stream->arguments.count));
    CHECK_EQUAL(expected_arg_count, data_stream->arguments.count);
    CHECK_EQUAL(CCAPI_DP_ARG_DATA_DOUBLE, data_stream->arguments.list[0]);
    STRCMP_EQUAL(stream_id, data_stream->stream_id);
    CHECK(NULL == data_stream->forward_to);
    CHECK(NULL == data_stream->units);
}

TEST(test_ccapi_dp_data_stream, testDataStreamDataString)
{
    char const * const stream_id = "stream_1";
    char const * const format_string = CCAPI_DP_KEY_DATA_STRING;
    ccapi_dp_argument_t const expected_type = CCAPI_DP_ARG_DATA_STRING;
    int const expected_arg_count = 1;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(1 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_create_data_stream(&data_stream, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(data_stream != NULL);
    CHECK_EQUAL(expected_type, get_dp_type_from_arg_list(data_stream->arguments.list, data_stream->arguments.count));
    CHECK_EQUAL(expected_arg_count, data_stream->arguments.count);
    CHECK_EQUAL(CCAPI_DP_ARG_DATA_STRING, data_stream->arguments.list[0]);
    STRCMP_EQUAL(stream_id, data_stream->stream_id);
    CHECK(NULL == data_stream->forward_to);
    CHECK(NULL == data_stream->units);
}

TEST(test_ccapi_dp_data_stream, testDataStreamTimeStampEpoch)
{
    char const * const stream_id = "stream_1";
    char const * const format_string = CCAPI_DP_KEY_DATA_STRING " " CCAPI_DP_KEY_TS_EPOCH;
    ccapi_dp_argument_t const expected_type = CCAPI_DP_ARG_DATA_STRING;
    ccapi_dp_argument_t const expected_arg_1 = CCAPI_DP_ARG_DATA_STRING;
    ccapi_dp_argument_t const expected_arg_2 = CCAPI_DP_ARG_TIME_EPOCH;
    int const expected_arg_count = 2;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(2 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_create_data_stream(&data_stream, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(data_stream != NULL);
    CHECK_EQUAL(expected_type, get_dp_type_from_arg_list(data_stream->arguments.list, data_stream->arguments.count));
    CHECK_EQUAL(expected_arg_count, data_stream->arguments.count);
    CHECK_EQUAL(expected_arg_1, data_stream->arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, data_stream->arguments.list[1]);
    STRCMP_EQUAL(stream_id, data_stream->stream_id);
    CHECK(NULL == data_stream->forward_to);
    CHECK(NULL == data_stream->units);
}

TEST(test_ccapi_dp_data_stream, testDataStreamTimeStampEpochMsec)
{
    char const * const stream_id = "stream_1";
    char const * const format_string = CCAPI_DP_KEY_DATA_STRING " " CCAPI_DP_KEY_TS_EPOCHMS;
    ccapi_dp_argument_t const expected_type = CCAPI_DP_ARG_DATA_STRING;
    ccapi_dp_argument_t const expected_arg_1 = CCAPI_DP_ARG_DATA_STRING;
    ccapi_dp_argument_t const expected_arg_2 = CCAPI_DP_ARG_TIME_EPOCH_MSEC;
    int const expected_arg_count = 2;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(2 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_create_data_stream(&data_stream, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(data_stream != NULL);
    CHECK_EQUAL(expected_type, get_dp_type_from_arg_list(data_stream->arguments.list, data_stream->arguments.count));
    CHECK_EQUAL(expected_arg_count, data_stream->arguments.count);
    CHECK_EQUAL(expected_arg_1, data_stream->arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, data_stream->arguments.list[1]);
    STRCMP_EQUAL(stream_id, data_stream->stream_id);
    CHECK(NULL == data_stream->forward_to);
    CHECK(NULL == data_stream->units);
}

TEST(test_ccapi_dp_data_stream, testDataStreamTimeStampISO8601)
{
    char const * const stream_id = "myISO8601-stream";
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_STRING;
    ccapi_dp_argument_t const expected_type = CCAPI_DP_ARG_DATA_STRING;
    ccapi_dp_argument_t const expected_arg_1 = CCAPI_DP_ARG_TIME_ISO8601;
    ccapi_dp_argument_t const expected_arg_2 = CCAPI_DP_ARG_DATA_STRING;
    int const expected_arg_count = 2;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(2 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_create_data_stream(&data_stream, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(data_stream != NULL);
    CHECK_EQUAL(expected_type, get_dp_type_from_arg_list(data_stream->arguments.list, data_stream->arguments.count));
    CHECK_EQUAL(expected_arg_count, data_stream->arguments.count);
    CHECK_EQUAL(expected_arg_1, data_stream->arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, data_stream->arguments.list[1]);
    STRCMP_EQUAL(stream_id, data_stream->stream_id);
    CHECK(NULL == data_stream->forward_to);
    CHECK(NULL == data_stream->units);
}

TEST(test_ccapi_dp_data_stream, testDataStreamLocation)
{
    char const * const stream_id = "myISO8601-stream";
    char const * const format_string = CCAPI_DP_KEY_DATA_INT32 " " CCAPI_DP_KEY_LOCATION;
    ccapi_dp_argument_t const expected_type = CCAPI_DP_ARG_DATA_INT32;
    ccapi_dp_argument_t const expected_arg_1 = CCAPI_DP_ARG_DATA_INT32;
    ccapi_dp_argument_t const expected_arg_2 = CCAPI_DP_ARG_LOC;
    int const expected_arg_count = 2;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(2 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_create_data_stream(&data_stream, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(data_stream != NULL);
    CHECK_EQUAL(expected_type, get_dp_type_from_arg_list(data_stream->arguments.list, data_stream->arguments.count));
    CHECK_EQUAL(expected_arg_count, data_stream->arguments.count);
    CHECK_EQUAL(expected_arg_1, data_stream->arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, data_stream->arguments.list[1]);
    STRCMP_EQUAL(stream_id, data_stream->stream_id);
    CHECK(NULL == data_stream->forward_to);
    CHECK(NULL == data_stream->units);
}

TEST(test_ccapi_dp_data_stream, testDataStreamQuality)
{
    char const * const stream_id = "qualitystream[1]";
    char const * const format_string = CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    ccapi_dp_argument_t const expected_type = CCAPI_DP_ARG_DATA_FLOAT;
    ccapi_dp_argument_t const expected_arg_1 = CCAPI_DP_ARG_DATA_FLOAT;
    ccapi_dp_argument_t const expected_arg_2 = CCAPI_DP_ARG_LOC;
    ccapi_dp_argument_t const expected_arg_3 = CCAPI_DP_ARG_QUAL;
    int const expected_arg_count = 3;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_create_data_stream(&data_stream, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(data_stream != NULL);
    CHECK_EQUAL(expected_type, get_dp_type_from_arg_list(data_stream->arguments.list, data_stream->arguments.count));
    CHECK_EQUAL(expected_arg_count, data_stream->arguments.count);
    CHECK_EQUAL(expected_arg_1, data_stream->arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, data_stream->arguments.list[1]);
    CHECK_EQUAL(expected_arg_3, data_stream->arguments.list[2]);
    STRCMP_EQUAL(stream_id, data_stream->stream_id);
    CHECK(NULL == data_stream->forward_to);
    CHECK(NULL == data_stream->units);
}

TEST(test_ccapi_dp_data_stream, testDataStreamExtraFull)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    char const * const stream_id = "temperature";
    char const * const units = "temperature_math";
    char const * const forward_to = "Kelvin";

    ccapi_dp_argument_t const expected_type = CCAPI_DP_ARG_DATA_FLOAT;
    ccapi_dp_argument_t const expected_arg_1 = CCAPI_DP_ARG_TIME_ISO8601;
    ccapi_dp_argument_t const expected_arg_2 = CCAPI_DP_ARG_DATA_FLOAT;
    ccapi_dp_argument_t const expected_arg_3 = CCAPI_DP_ARG_LOC;
    ccapi_dp_argument_t const expected_arg_4 = CCAPI_DP_ARG_QUAL;
    int const expected_arg_count = 4;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(units) + 1, TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(forward_to) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_create_data_stream_extra(&data_stream, stream_id, format_string, units, forward_to);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(data_stream != NULL);
    CHECK_EQUAL(expected_type, get_dp_type_from_arg_list(data_stream->arguments.list, data_stream->arguments.count));
    CHECK_EQUAL(expected_arg_count, data_stream->arguments.count);
    CHECK_EQUAL(expected_arg_1, data_stream->arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, data_stream->arguments.list[1]);
    CHECK_EQUAL(expected_arg_3, data_stream->arguments.list[2]);
    CHECK_EQUAL(expected_arg_4, data_stream->arguments.list[3]);

    STRCMP_EQUAL(stream_id, data_stream->stream_id);
    STRCMP_EQUAL(units, data_stream->units);
    STRCMP_EQUAL(forward_to, data_stream->forward_to);
}

TEST(test_ccapi_dp_data_stream, testDataStreamFull)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    char const * const stream_id = "temperature";

    ccapi_dp_argument_t const expected_type = CCAPI_DP_ARG_DATA_FLOAT;
    ccapi_dp_argument_t const expected_arg_1 = CCAPI_DP_ARG_TIME_ISO8601;
    ccapi_dp_argument_t const expected_arg_2 = CCAPI_DP_ARG_DATA_FLOAT;
    ccapi_dp_argument_t const expected_arg_3 = CCAPI_DP_ARG_LOC;
    ccapi_dp_argument_t const expected_arg_4 = CCAPI_DP_ARG_QUAL;
    int const expected_arg_count = 4;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_create_data_stream(&data_stream, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(data_stream != NULL);
    CHECK_EQUAL(expected_type, get_dp_type_from_arg_list(data_stream->arguments.list, data_stream->arguments.count));
    CHECK_EQUAL(expected_arg_count, data_stream->arguments.count);
    CHECK_EQUAL(expected_arg_1, data_stream->arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, data_stream->arguments.list[1]);
    CHECK_EQUAL(expected_arg_3, data_stream->arguments.list[2]);
    CHECK_EQUAL(expected_arg_4, data_stream->arguments.list[3]);

    STRCMP_EQUAL(stream_id, data_stream->stream_id);
    CHECK(NULL == data_stream->forward_to);
    CHECK(NULL == data_stream->units);
}

TEST(test_ccapi_dp_data_stream, testDataStreamExtraNoMemory4StreamID)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    char const * const stream_id = "temperature";
    char const * const units = "temperature_math";
    char const * const forward_to = "Kelvin";
    int const expected_arg_count = 4;

    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NULL, false);

    dp_error = ccapi_dp_create_data_stream_extra(&data_stream, stream_id, format_string, units, forward_to);
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamExtraNoMemory4Units)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    char const * const stream_id = "temperature";
    char const * const units = "temperature_math";
    char const * const forward_to = "Kelvin";
    int const expected_arg_count = 4;

    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(units) + 1, TH_MALLOC_RETURN_NULL, false);

    dp_error = ccapi_dp_create_data_stream_extra(&data_stream, stream_id, format_string, units, forward_to);
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamExtraNoMemory4ForwardTo)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    char const * const stream_id = "temperature";
    char const * const units = "temperature_math";
    char const * const forward_to = "Kelvin";
    int const expected_arg_count = 4;

    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(units) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(forward_to) + 1, TH_MALLOC_RETURN_NULL, false);

    dp_error = ccapi_dp_create_data_stream_extra(&data_stream, stream_id, format_string, units, forward_to);
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamInvalidKeyword)
{
    char const * const format_string = "pepito" " " CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream_1", format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamTooLong)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY " " CCAPI_DP_KEY_QUALITY;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream_1", format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamDataTwoDataTypes)
{
    char const * const format_string = CCAPI_DP_KEY_DATA_STRING " " CCAPI_DP_KEY_DATA_INT32;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream_1", format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamTwoTimeStamp)
{
    char const * const format_string = CCAPI_DP_KEY_DATA_STRING " " CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_TS_EPOCH;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream_1", format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamTwoLocation)
{
    char const * const format_string = CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_DATA_STRING " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_TS_EPOCH;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream_1", format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamTwoQuality)
{
    char const * const format_string = CCAPI_DP_KEY_QUALITY " " CCAPI_DP_KEY_DATA_DOUBLE " " CCAPI_DP_KEY_QUALITY " " CCAPI_DP_KEY_TS_EPOCH;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_create_data_stream(&data_stream, "stream_1", format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamDestroyNullArgument)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_destroy_data_stream(NULL);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);
}

TEST(test_ccapi_dp_data_stream, testDataStreamDestroyCorruptedArgument)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    char const * const stream_id = "temperature";
    char const * const units = "temperature_math";
    char const * const forward_to = "Kelvin";
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;
    char const * aux_string;

    dp_error = ccapi_dp_create_data_stream_extra(&data_stream, stream_id, format_string, units, forward_to);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    Mock_ccimp_os_free_notExpected();

    aux_string = data_stream->stream_id;
    data_stream->stream_id = NULL;

    dp_error = ccapi_dp_destroy_data_stream(data_stream);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);

    data_stream->stream_id = aux_string;
    data_stream->arguments.list = NULL;

    dp_error = ccapi_dp_destroy_data_stream(data_stream);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);
}

TEST(test_ccapi_dp_data_stream, testDataStreamDestroyOk)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    char const * const stream_id = "temperature";
    char const * const units = "temperature_math";
    char const * const forward_to = "Kelvin";
    int const expected_arg_count = 4;
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(units) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(forward_to) + 1, TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_create_data_stream_extra(&data_stream, stream_id, format_string, units, forward_to);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_destroy_data_stream(data_stream);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}
