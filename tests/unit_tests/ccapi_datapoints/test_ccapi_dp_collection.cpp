#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_dp_collection)
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

TEST(test_ccapi_dp_collection, testCreateCollectionInvalidArgument)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_create_collection(NULL);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);
}

TEST(test_ccapi_dp_collection, testClearCollectionInvalidArgument)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_clear_collection(NULL);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);
}

TEST(test_ccapi_dp_collection, testDestroyCollectionInvalidArgument)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_destroy_collection(NULL);
    CHECK_EQUAL(CCAPI_DP_ERROR_INVALID_ARGUMENT, dp_error);
}

TEST(test_ccapi_dp_collection, testCreateCollectionNotEnoughMemory)
{
    void * malloc_for_dp_collection = NULL;
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_dp_collection_t), malloc_for_dp_collection);

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(dp_collection == NULL);
}

TEST(test_ccapi_dp_collection, testCreateCollectionSyncrFailed)
{
    void * malloc_for_dp_collection = malloc(sizeof (ccapi_dp_collection_t));
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_dp_collection_t), malloc_for_dp_collection);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_dp_collection, CCIMP_STATUS_OK);
    Mock_ccimp_os_syncr_create_return(CCIMP_STATUS_ERROR);

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_SYNCR_FAILED, dp_error);
    CHECK(dp_collection == NULL);
}

TEST(test_ccapi_dp_collection, testCreateCollectionOk)
{
    void * malloc_for_dp_collection = malloc(sizeof (ccapi_dp_collection_t));
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_dp_collection_t), malloc_for_dp_collection);
    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection != NULL);
    CHECK(dp_collection->syncr != NULL);
    CHECK(dp_collection->data_points_list == NULL);
}

TEST(test_ccapi_dp_collection, testClearCollectionSyncrFailed)
{
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection != NULL);

    Mock_ccimp_os_syncr_acquire_return(CCIMP_STATUS_ERROR);
    dp_error = ccapi_dp_clear_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_SYNCR_FAILED, dp_error);
}

TEST(test_ccapi_dp_collection, testClearEmptyCollection)
{
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection != NULL);

    dp_error = ccapi_dp_clear_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}

TEST(test_ccapi_dp_collection, testClearCollectionOK)
{
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection != NULL);

    /* Fill collection with dummy values */
    ccapi_data_point_t * first_dp = (ccapi_data_point_t *)malloc(sizeof *first_dp);
    ccapi_data_point_t * second_dp = (ccapi_data_point_t *)malloc(sizeof *second_dp);

    first_dp->next = second_dp;
    second_dp->next = NULL;

    dp_collection->data_points_list = first_dp;

    Mock_ccimp_os_free_expectAndReturn(first_dp, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(second_dp, CCIMP_STATUS_OK);

    dp_error = ccapi_dp_clear_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}

TEST(test_ccapi_dp_collection, testDestroyEmptyCollectionOk)
{
    void * malloc_for_dp_collection = malloc(sizeof (ccapi_dp_collection_t));
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_dp_collection_t), malloc_for_dp_collection);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_dp_collection, CCIMP_STATUS_OK);

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    dp_error = ccapi_dp_destroy_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}

TEST(test_ccapi_dp_collection, testDestroyCollectionAcquireFailed)
{
    void * malloc_for_dp_collection = malloc(sizeof (ccapi_dp_collection_t));
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_dp_collection_t), malloc_for_dp_collection);
    Mock_ccimp_os_syncr_acquire_return(CCIMP_STATUS_ERROR);

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    dp_error = ccapi_dp_destroy_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_SYNCR_FAILED, dp_error);
}

TEST(test_ccapi_dp_collection, testDestroyCollectionOk)
{
    void * malloc_for_dp_collection = malloc(sizeof (ccapi_dp_collection_t));
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    Mock_ccimp_os_malloc_expectAndReturn(sizeof (ccapi_dp_collection_t), malloc_for_dp_collection);

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);

    /* Fill collection with dummy values */
    ccapi_data_point_t * first_dp = (ccapi_data_point_t *)malloc(sizeof *first_dp);
    ccapi_data_point_t * second_dp = (ccapi_data_point_t *)malloc(sizeof *second_dp);
    stream_seen_t * first_stream = (stream_seen_t *)malloc(sizeof *first_stream);
    stream_seen_t * second_stream = (stream_seen_t *)malloc(sizeof *second_stream);

    first_dp->next = second_dp;
    second_dp->next = NULL;

    first_stream->stream_id = (char *)malloc(sizeof "Stream_1");
    second_stream->stream_id =(char *) malloc(sizeof "Stream_2");
    strcpy(first_stream->stream_id, "Stream_1");
    strcpy(second_stream->stream_id, "Stream_2");

    first_stream->next = second_stream;
    second_stream->next = NULL;

    dp_collection->data_points_list = first_dp;

    Mock_ccimp_os_free_expectAndReturn(first_dp, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(second_dp, CCIMP_STATUS_OK);
    Mock_ccimp_os_free_expectAndReturn(malloc_for_dp_collection, CCIMP_STATUS_OK);

    dp_error = ccapi_dp_destroy_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}
