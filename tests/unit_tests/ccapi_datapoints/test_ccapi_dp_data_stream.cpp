#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_dp_data_stream)
{
    ccapi_dp_collection_handle_t dp_collection;
    ccapi_dp_collection_t * dp_collection_spy;
    void setup()
    {
        ccapi_dp_error_t dp_error;

        Mock_create_all();
        dp_error = ccapi_dp_create_collection(&dp_collection);
        dp_collection_spy = (ccapi_dp_collection_t *)dp_collection;
        CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_dp_data_stream, testDataStreamInvalidArguments)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_add_data_stream_to_collection(NULL, "stream_1", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);
}

TEST(test_ccapi_dp_data_stream, testDataStreamInvalidStreamID)
{
    ccapi_dp_error_t dp_error;
    ccapi_dp_data_stream_t * data_stream = NULL;

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, NULL, "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_STREAM_ID, dp_error);
    CHECK(data_stream == NULL);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_STREAM_ID, dp_error);
    CHECK(data_stream == NULL);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "s p a c e s", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_STREAM_ID, dp_error);
    CHECK(data_stream == NULL);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "s&mb0/5", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_STREAM_ID, dp_error);
    CHECK(data_stream == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamInvalidFormatString)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", NULL);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", "");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", " int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", "int32, ts_epoch / loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamExtraInvalidForwardTo)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_add_data_stream_to_collection_extra(dp_collection, "stream_1", "int32 ts_epoch loc", NULL, "");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORWARD_TO, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);

    dp_error = ccapi_dp_add_data_stream_to_collection_extra(dp_collection, "stream_1", "int32 ts_epoch loc", NULL, "s p a c e s");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORWARD_TO, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamExtraInvalidUnits)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_add_data_stream_to_collection_extra(dp_collection, "stream_1", "int32 ts_epoch loc", "", "stream_2");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_UNITS, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamNoMemory4FormatString)
{
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof "int32 ts_epoch loc", TH_MALLOC_RETURN_NULL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamNoMemory4CcapiDataStream)
{
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof "int32 ts_epoch loc", TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(3 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NULL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamNoMemory4CcfsmDataStream)
{
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof "int32 ts_epoch loc", TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(3 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NULL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamFormatStringInvalidKeyWord)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", "int8");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamNoMemory4ArgList)
{
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof "int32", TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(1 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NULL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", "int32");
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamDataInt32)
{
    char const * const stream_id = "stream_1";
    char const * const format_string = CCAPI_DP_KEY_DATA_INT32;
    ccapi_dp_argument_t const expected_arg = CCAPI_DP_ARG_DATA_INT32;
    connector_data_point_type_t const expected_type = connector_data_point_type_integer;
    size_t const expected_arg_count = 1;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(1 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->next == NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->point == NULL);

    CHECK_EQUAL(expected_arg, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);
    CHECK_EQUAL(expected_type, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->type);
}

TEST(test_ccapi_dp_data_stream, testDataStreamDataInt64)
{
    char const * const stream_id = "stream_1";
    char const * const format_string = CCAPI_DP_KEY_DATA_INT64;
    ccapi_dp_argument_t const expected_arg = CCAPI_DP_ARG_DATA_INT64;
    connector_data_point_type_t const expected_type = connector_data_point_type_long;
    size_t const expected_arg_count = 1;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(1 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->next == NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->point == NULL);

    CHECK_EQUAL(expected_arg, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);
    CHECK_EQUAL(expected_type, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->type);
}

TEST(test_ccapi_dp_data_stream, testDataStreamDataFloat)
{
    char const * const stream_id = "stream_1";
    char const * const format_string = CCAPI_DP_KEY_DATA_FLOAT;
    ccapi_dp_argument_t const expected_arg = CCAPI_DP_ARG_DATA_FLOAT;
    connector_data_point_type_t const expected_type = connector_data_point_type_float;
    size_t const expected_arg_count = 1;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(1 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->next == NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->point == NULL);

    CHECK_EQUAL(expected_arg, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);
    CHECK_EQUAL(expected_type, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->type);
}

TEST(test_ccapi_dp_data_stream, testDataStreamDataDouble)
{
    char const * const stream_id = "stream_1";
    char const * const format_string = CCAPI_DP_KEY_DATA_DOUBLE;
    ccapi_dp_argument_t const expected_arg = CCAPI_DP_ARG_DATA_DOUBLE;
    connector_data_point_type_t const expected_type = connector_data_point_type_double;
    size_t const expected_arg_count = 1;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(1 * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->next == NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->point == NULL);

    CHECK_EQUAL(expected_arg, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);
    CHECK_EQUAL(expected_type, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->type);
}

TEST(test_ccapi_dp_data_stream, testDataStreamDataString)
{
    char const * const stream_id = "stream_1";
    char const * const format_string = CCAPI_DP_KEY_DATA_STRING;
    ccapi_dp_argument_t const expected_arg = CCAPI_DP_ARG_DATA_STRING;
    connector_data_point_type_t const expected_type = connector_data_point_type_string;
    size_t const expected_arg_count = 1;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->next == NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->point == NULL);

    CHECK_EQUAL(expected_arg, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);
    CHECK_EQUAL(expected_type, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->type);
}

TEST(test_ccapi_dp_data_stream, testDataStreamTimeStampEpoch)
{
    char const * const stream_id = "stream_1";
    char const * const format_string = CCAPI_DP_KEY_DATA_STRING " " CCAPI_DP_KEY_TS_EPOCH;
    connector_data_point_type_t const expected_type = connector_data_point_type_string;
    ccapi_dp_argument_t const expected_arg_1 = CCAPI_DP_ARG_DATA_STRING;
    ccapi_dp_argument_t const expected_arg_2 = CCAPI_DP_ARG_TIME_EPOCH;
    size_t const expected_arg_count = 2;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->next == NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->point == NULL);

    CHECK_EQUAL(expected_arg_1, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[1]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);
    CHECK_EQUAL(expected_type, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->type);

    CHECK_EQUAL(expected_arg_1, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[1]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);

}

TEST(test_ccapi_dp_data_stream, testDataStreamTimeStampEpochMsec)
{
    char const * const stream_id = "stream_1";
    char const * const format_string = CCAPI_DP_KEY_DATA_STRING " " CCAPI_DP_KEY_TS_EPOCHMS;
    connector_data_point_type_t const expected_type = connector_data_point_type_string;
    ccapi_dp_argument_t const expected_arg_1 = CCAPI_DP_ARG_DATA_STRING;
    ccapi_dp_argument_t const expected_arg_2 = CCAPI_DP_ARG_TIME_EPOCH_MSEC;
    size_t const expected_arg_count = 2;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->next == NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->point == NULL);

    CHECK_EQUAL(expected_arg_1, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[1]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);
    CHECK_EQUAL(expected_type, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->type);

    CHECK_EQUAL(expected_arg_1, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[1]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);
}

TEST(test_ccapi_dp_data_stream, testDataStreamTimeStampISO8601)
{
    char const * const stream_id = "myISO8601-stream";
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_STRING;
    connector_data_point_type_t const expected_type = connector_data_point_type_string;
    ccapi_dp_argument_t const expected_arg_1 = CCAPI_DP_ARG_TIME_ISO8601;
    ccapi_dp_argument_t const expected_arg_2 = CCAPI_DP_ARG_DATA_STRING;
    size_t const expected_arg_count = 2;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->next == NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->point == NULL);

    CHECK_EQUAL(expected_arg_1, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[1]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);
    CHECK_EQUAL(expected_type, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->type);

    CHECK_EQUAL(expected_arg_1, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[1]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);
}

TEST(test_ccapi_dp_data_stream, testDataStreamLocation)
{
    char const * const stream_id = "myISO8601-stream";
    char const * const format_string = CCAPI_DP_KEY_DATA_INT32 " " CCAPI_DP_KEY_LOCATION;
    connector_data_point_type_t const expected_type = connector_data_point_type_integer;
    ccapi_dp_argument_t const expected_arg_1 = CCAPI_DP_ARG_DATA_INT32;
    ccapi_dp_argument_t const expected_arg_2 = CCAPI_DP_ARG_LOC;
    size_t const expected_arg_count = 2;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->next == NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->point == NULL);

    CHECK_EQUAL(expected_arg_1, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[1]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);
    CHECK_EQUAL(expected_type, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->type);

    CHECK_EQUAL(expected_arg_1, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[1]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);
}

TEST(test_ccapi_dp_data_stream, testDataStreamQuality)
{
    char const * const stream_id = "qualitystream[1]";
    char const * const format_string = CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    connector_data_point_type_t const expected_type = connector_data_point_type_float;
    ccapi_dp_argument_t const expected_arg_1 = CCAPI_DP_ARG_DATA_FLOAT;
    ccapi_dp_argument_t const expected_arg_2 = CCAPI_DP_ARG_LOC;
    ccapi_dp_argument_t const expected_arg_3 = CCAPI_DP_ARG_QUAL;
    size_t const expected_arg_count = 3;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->next == NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->point == NULL);

    CHECK_EQUAL(expected_arg_1, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[1]);
    CHECK_EQUAL(expected_arg_3, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[2]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);
    CHECK_EQUAL(expected_type, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->type);
}

TEST(test_ccapi_dp_data_stream, testDataStreamExtraFull)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    char const * const stream_id = "temperature";
    char const * const units = "temperature_math";
    char const * const forward_to = "Kelvin";

    connector_data_point_type_t const expected_type = connector_data_point_type_float;
    ccapi_dp_argument_t const expected_arg_1 = CCAPI_DP_ARG_TIME_ISO8601;
    ccapi_dp_argument_t const expected_arg_2 = CCAPI_DP_ARG_DATA_FLOAT;
    ccapi_dp_argument_t const expected_arg_3 = CCAPI_DP_ARG_LOC;
    ccapi_dp_argument_t const expected_arg_4 = CCAPI_DP_ARG_QUAL;
    size_t const expected_arg_count = 4;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(units) + 1, TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(forward_to) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection_extra(dp_collection, stream_id, format_string, units, forward_to);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->next == NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->point == NULL);

    CHECK_EQUAL(expected_arg_1, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[1]);
    CHECK_EQUAL(expected_arg_3, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[2]);
    CHECK_EQUAL(expected_arg_4, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[3]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);
    CHECK_EQUAL(expected_type, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->type);

    STRCMP_EQUAL(stream_id, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->stream_id);
    STRCMP_EQUAL(units, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->unit);
    STRCMP_EQUAL(forward_to, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->forward_to);
}

TEST(test_ccapi_dp_data_stream, testDataStreamFull)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    char const * const stream_id = "temperature";

    connector_data_point_type_t const expected_type = connector_data_point_type_float;
    ccapi_dp_argument_t const expected_arg_1 = CCAPI_DP_ARG_TIME_ISO8601;
    ccapi_dp_argument_t const expected_arg_2 = CCAPI_DP_ARG_DATA_FLOAT;
    ccapi_dp_argument_t const expected_arg_3 = CCAPI_DP_ARG_LOC;
    ccapi_dp_argument_t const expected_arg_4 = CCAPI_DP_ARG_QUAL;
    size_t const expected_arg_count = 4;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, stream_id, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream != NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->next == NULL);
    CHECK(dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->point == NULL);

    CHECK_EQUAL(expected_arg_1, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[0]);
    CHECK_EQUAL(expected_arg_2, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[1]);
    CHECK_EQUAL(expected_arg_3, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[2]);
    CHECK_EQUAL(expected_arg_4, dp_collection_spy->ccapi_data_stream_list[0].arguments.list[3]);
    CHECK_EQUAL(expected_arg_count, dp_collection_spy->ccapi_data_stream_list[0].arguments.count);
    CHECK_EQUAL(expected_type, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->type);

    STRCMP_EQUAL(stream_id, dp_collection_spy->ccapi_data_stream_list[0].ccfsm_data_stream->stream_id);
}

TEST(test_ccapi_dp_data_stream, testDataStreamExtraNoMemory4StreamID)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    char const * const stream_id = "temperature";
    char const * const units = "temperature_math";
    char const * const forward_to = "Kelvin";
    size_t const expected_arg_count = 4;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NULL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection_extra(dp_collection, stream_id, format_string, units, forward_to);
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamExtraNoMemory4Units)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    char const * const stream_id = "temperature";
    char const * const units = "temperature_math";
    char const * const forward_to = "Kelvin";
    size_t const expected_arg_count = 4;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(units) + 1, TH_MALLOC_RETURN_NULL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection_extra(dp_collection, stream_id, format_string, units, forward_to);
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamExtraNoMemory4ForwardTo)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    char const * const stream_id = "temperature";
    char const * const units = "temperature_math";
    char const * const forward_to = "Kelvin";
    size_t const expected_arg_count = 4;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(units) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(forward_to) + 1, TH_MALLOC_RETURN_NULL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection_extra(dp_collection, stream_id, format_string, units, forward_to);
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamInvalidKeyword)
{
    char const * const format_string = "pepito" " " CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamTooLong)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY " " CCAPI_DP_KEY_QUALITY;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamDataTwoDataTypes)
{
    char const * const format_string = CCAPI_DP_KEY_DATA_STRING " " CCAPI_DP_KEY_DATA_INT32;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamTwoTimeStamp)
{
    char const * const format_string = CCAPI_DP_KEY_DATA_STRING " " CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_TS_EPOCH;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamTwoLocation)
{
    char const * const format_string = CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_DATA_STRING " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_TS_EPOCH;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamTwoQuality)
{
    char const * const format_string = CCAPI_DP_KEY_QUALITY " " CCAPI_DP_KEY_DATA_DOUBLE " " CCAPI_DP_KEY_QUALITY " " CCAPI_DP_KEY_TS_EPOCH;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_FORMAT, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamIDOK)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream-1", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "relative+absolute", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "parent/child", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream[1]", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream.1", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "!stream", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}

TEST(test_ccapi_dp_data_stream, testDataStreamAddTwice)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", "int32 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, "stream_1", "int64 ts_epoch loc");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_STREAM_ID, dp_error);
}

TEST(test_ccapi_dp_data_stream, testDataStreamRemoveNullArgument)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_remove_data_stream_from_collection(NULL, "stream1");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);

    dp_error = ccapi_dp_remove_data_stream_from_collection(dp_collection, NULL);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);

    dp_error = ccapi_dp_remove_data_stream_from_collection(dp_collection, "");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);
}

TEST(test_ccapi_dp_data_stream, testDataStreamRemoveEmptyCollection)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_remove_data_stream_from_collection(dp_collection, "inexistent_id");
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_STREAM_ID, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamRemove1of1)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    char const * const stream_id = "temperature";
    char const * const units = "temperature_math";
    char const * const forward_to = "Kelvin";
    size_t const expected_arg_count = 4;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(units) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(forward_to) + 1, TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_add_data_stream_to_collection_extra(dp_collection, stream_id, format_string, units, forward_to);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);

    dp_error = ccapi_dp_remove_data_stream_from_collection(dp_collection, stream_id);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list == NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamRemoveInexistent)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    char const * const stream_id = "temperature";
    char const * const inexistent_stream_id = "inexistent_stream_id";
    char const * const units = "temperature_math";
    char const * const forward_to = "Kelvin";
    size_t const expected_arg_count = 4;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id) + 1, TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(units) + 1, TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(forward_to) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection_extra(dp_collection, stream_id, format_string, units, forward_to);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);

    dp_error = ccapi_dp_remove_data_stream_from_collection(dp_collection, inexistent_stream_id);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_STREAM_ID, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);
}

TEST(test_ccapi_dp_data_stream, testDataStreamRemove1of3)
{
    char const * const format_string = CCAPI_DP_KEY_TS_ISO8601 " " CCAPI_DP_KEY_DATA_FLOAT " " CCAPI_DP_KEY_LOCATION " " CCAPI_DP_KEY_QUALITY;
    char const * const stream_id_1 = "fulanito";
    char const * const stream_id_2 = "menganito";
    char const * const stream_id_3 = "el-de-la-moto";
    char const * const units = "temperature_math";
    char const * const forward_to = "Kelvin";
    size_t const expected_arg_count = 4;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id_1) + 1, TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(units) + 1, TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(forward_to) + 1, TH_MALLOC_RETURN_NORMAL, false);

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(strlen(stream_id_2) + 1, TH_MALLOC_RETURN_NORMAL, true);

    th_expect_malloc(strlen(format_string) + 1, TH_MALLOC_RETURN_NORMAL, true);
    th_expect_malloc(sizeof (ccapi_dp_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(sizeof (connector_data_stream_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(expected_arg_count * sizeof (ccapi_dp_argument_t), TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(stream_id_3) + 1, TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(units) + 1, TH_MALLOC_RETURN_NORMAL, false);
    th_expect_malloc(strlen(forward_to) + 1, TH_MALLOC_RETURN_NORMAL, false);

    dp_error = ccapi_dp_add_data_stream_to_collection_extra(dp_collection, stream_id_1, format_string, units, forward_to);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_add_data_stream_to_collection(dp_collection, stream_id_2, format_string);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    dp_error = ccapi_dp_add_data_stream_to_collection_extra(dp_collection, stream_id_3, format_string, units, forward_to);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    STRCMP_EQUAL(stream_id_3, dp_collection_spy->ccapi_data_stream_list->ccfsm_data_stream->stream_id);
    STRCMP_EQUAL(stream_id_2, dp_collection_spy->ccapi_data_stream_list->next->ccfsm_data_stream->stream_id);
    STRCMP_EQUAL(stream_id_1, dp_collection_spy->ccapi_data_stream_list->next->next->ccfsm_data_stream->stream_id);

    dp_error = ccapi_dp_remove_data_stream_from_collection(dp_collection, stream_id_2);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection_spy->ccapi_data_stream_list != NULL);
    STRCMP_EQUAL(stream_id_3, dp_collection_spy->ccapi_data_stream_list->ccfsm_data_stream->stream_id);
    CHECK(dp_collection_spy->ccapi_data_stream_list->next != NULL);
    STRCMP_EQUAL(stream_id_1, dp_collection_spy->ccapi_data_stream_list->next->ccfsm_data_stream->stream_id);
}
