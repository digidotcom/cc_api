#include "test_helper_functions.h"

TEST_GROUP(test_ccapi_dp_collection)
{
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
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof (ccapi_dp_collection_t), TH_MALLOC_RETURN_NULL, false);

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_INSUFFICIENT_MEMORY, dp_error);
    CHECK(dp_collection == NULL);
}

TEST(test_ccapi_dp_collection, testCreateCollectionSyncrFailed)
{
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof (ccapi_dp_collection_t), TH_MALLOC_RETURN_NORMAL, true);
    Mock_ccimp_os_syncr_create_return(CCIMP_STATUS_ERROR);

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_SYNCR_FAILED, dp_error);
    CHECK(dp_collection == NULL);
}

TEST(test_ccapi_dp_collection, testCreateCollectionOk)
{
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof (ccapi_dp_collection_t), TH_MALLOC_RETURN_NORMAL, false);
    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    CHECK(dp_collection != NULL);
    CHECK(dp_collection->syncr != NULL);
    CHECK(dp_collection->ccapi_data_stream_list == NULL);
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

TEST(test_ccapi_dp_collection, testDestroyEmptyCollectionOk)
{
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof (ccapi_dp_collection_t), TH_MALLOC_RETURN_NORMAL, true);

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    dp_error = ccapi_dp_destroy_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}

TEST(test_ccapi_dp_collection, testDestroyCollectionAcquireFailed)
{
    ccapi_dp_collection_t * dp_collection = (ccapi_dp_collection_t *)&dp_collection;
    ccapi_dp_error_t dp_error;

    th_expect_malloc(sizeof (ccapi_dp_collection_t), TH_MALLOC_RETURN_NORMAL, false);
    Mock_ccimp_os_syncr_acquire_return(CCIMP_STATUS_ERROR);

    dp_error = ccapi_dp_create_collection(&dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
    dp_error = ccapi_dp_destroy_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_SYNCR_FAILED, dp_error);
}

TEST_GROUP(test_ccapi_dp_collection_destroy_and_clear)
{
    ccapi_dp_collection_t * dp_collection;
    void setup()
    {
        Mock_create_all();
        ccapi_dp_error_t dp_error;

        th_expect_malloc(sizeof (ccapi_dp_collection_t), TH_MALLOC_RETURN_NORMAL, false);
        dp_error = ccapi_dp_create_collection(&dp_collection);
        CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
        CHECK(dp_collection != NULL);

        /* Fill collection with dummy values */
        connector_data_stream_t * data_stream_1 = (connector_data_stream_t *)malloc(sizeof *data_stream_1);
        connector_data_stream_t * data_stream_2 = (connector_data_stream_t *)malloc(sizeof *data_stream_2);
        connector_data_point_t * ds_1_dp1 = (connector_data_point_t *)malloc(sizeof *ds_1_dp1);
        connector_data_point_t * ds_1_dp2 = (connector_data_point_t *)malloc(sizeof *ds_1_dp2);
        connector_data_point_t * ds_2_dp1 = (connector_data_point_t *)malloc(sizeof *ds_2_dp1);

        ccapi_dp_data_stream_t * ccapi_ds_1 = (ccapi_dp_data_stream_t *)malloc(sizeof *ccapi_ds_1);
        ccapi_dp_argument_t * ccapi_ds_1_args = (ccapi_dp_argument_t *)malloc(sizeof * ccapi_ds_1_args);
        ccapi_dp_data_stream_t * ccapi_ds_2 = (ccapi_dp_data_stream_t *)malloc(sizeof *ccapi_ds_2);
        ccapi_dp_argument_t * ccapi_ds_2_args = (ccapi_dp_argument_t *)malloc(sizeof * ccapi_ds_2_args);

        Mock_ccimp_os_free_expectAndReturn(data_stream_1, CCIMP_STATUS_OK);
        Mock_ccimp_os_free_expectAndReturn(data_stream_2, CCIMP_STATUS_OK);
        Mock_ccimp_os_free_expectAndReturn(ds_1_dp1, CCIMP_STATUS_OK);
        Mock_ccimp_os_free_expectAndReturn(ds_1_dp2, CCIMP_STATUS_OK);
        Mock_ccimp_os_free_expectAndReturn(ds_2_dp1, CCIMP_STATUS_OK);
        Mock_ccimp_os_free_expectAndReturn(ccapi_ds_1, CCIMP_STATUS_OK);
        Mock_ccimp_os_free_expectAndReturn(ccapi_ds_1_args, CCIMP_STATUS_OK);
        Mock_ccimp_os_free_expectAndReturn(ccapi_ds_2, CCIMP_STATUS_OK);
        Mock_ccimp_os_free_expectAndReturn(ccapi_ds_2_args, CCIMP_STATUS_OK);

        ccapi_ds_1->arguments.list = ccapi_ds_1_args;
        ccapi_ds_1->ccfsm_data_stream = data_stream_1;
        ccapi_ds_1->next = ccapi_ds_2;

        ccapi_ds_2->arguments.list = ccapi_ds_2_args;
        ccapi_ds_2->ccfsm_data_stream = data_stream_2;
        ccapi_ds_2->next = NULL;

        data_stream_1->point = ds_1_dp1;
        ds_1_dp1->next = ds_1_dp2;
        ds_1_dp2->next = NULL;

        data_stream_2->point = ds_2_dp1;
        ds_2_dp1->next = NULL;

        data_stream_1->next = NULL;
        data_stream_2->next = NULL;

        ccapi_ds_1->ccfsm_data_stream = data_stream_1;
        ccapi_ds_2->ccfsm_data_stream = data_stream_2;
        dp_collection->ccapi_data_stream_list = ccapi_ds_1;

    }

    void teardown()
    {
        Mock_destroy_all();
    }
};

TEST(test_ccapi_dp_collection_destroy_and_clear, testClearCollectionOK)
{
    ccapi_dp_error_t dp_error;

    dp_error = ccapi_dp_clear_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}

TEST(test_ccapi_dp_collection_destroy_and_clear, testDestroyCollectionOk)
{
    ccapi_dp_error_t dp_error;

    Mock_ccimp_os_free_expectAndReturn((void*)dp_collection, CCIMP_STATUS_OK);

    dp_error = ccapi_dp_destroy_collection(dp_collection);
    CHECK_EQUAL(CCAPI_DP_ERROR_NONE, dp_error);
}
