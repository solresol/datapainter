#include <gtest/gtest.h>
#include "database.h"
#include "metadata.h"
#include "data_table.h"

using namespace datapainter;

// Test fixture for data table tests
class DataTableTest : public ::testing::Test {
protected:
    void SetUp() override {
        db = std::make_unique<Database>(":memory:");
        ASSERT_TRUE(db->is_open());
        ASSERT_TRUE(db->ensure_metadata_table());

        // Create a test data table
        mgr = std::make_unique<MetadataManager>(*db);
        ASSERT_TRUE(mgr->create_data_table("test_data"));

        data_table = std::make_unique<DataTable>(*db, "test_data");
    }

    std::unique_ptr<Database> db;
    std::unique_ptr<MetadataManager> mgr;
    std::unique_ptr<DataTable> data_table;
};

// Test inserting a point
TEST_F(DataTableTest, InsertPoint) {
    auto id = data_table->insert_point(1.5, 2.5, "x");
    ASSERT_TRUE(id.has_value());
    EXPECT_GT(id.value(), 0);
}

// Test inserting multiple points
TEST_F(DataTableTest, InsertMultiplePoints) {
    auto id1 = data_table->insert_point(1.0, 2.0, "x");
    auto id2 = data_table->insert_point(3.0, 4.0, "o");
    auto id3 = data_table->insert_point(5.0, 6.0, "x");

    ASSERT_TRUE(id1.has_value());
    ASSERT_TRUE(id2.has_value());
    ASSERT_TRUE(id3.has_value());

    // IDs should be different
    EXPECT_NE(id1.value(), id2.value());
    EXPECT_NE(id2.value(), id3.value());
    EXPECT_NE(id1.value(), id3.value());
}

// Test deleting a point
TEST_F(DataTableTest, DeletePoint) {
    auto id = data_table->insert_point(1.0, 2.0, "x");
    ASSERT_TRUE(id.has_value());

    EXPECT_TRUE(data_table->delete_point(id.value()));
}

// Test deleting non-existent point fails
TEST_F(DataTableTest, DeleteNonExistentPointFails) {
    EXPECT_FALSE(data_table->delete_point(99999));
}

// Test updating point target
TEST_F(DataTableTest, UpdatePointTarget) {
    auto id = data_table->insert_point(1.0, 2.0, "x");
    ASSERT_TRUE(id.has_value());

    EXPECT_TRUE(data_table->update_point_target(id.value(), "o"));
}

// Test updating non-existent point fails
TEST_F(DataTableTest, UpdateNonExistentPointFails) {
    EXPECT_FALSE(data_table->update_point_target(99999, "x"));
}

// Test querying viewport with no points
TEST_F(DataTableTest, QueryViewportEmpty) {
    auto points = data_table->query_viewport(-10.0, 10.0, -10.0, 10.0);
    EXPECT_TRUE(points.empty());
}

// Test querying viewport with points inside
TEST_F(DataTableTest, QueryViewportWithPointsInside) {
    auto id1 = data_table->insert_point(1.0, 2.0, "x");
    auto id2 = data_table->insert_point(3.0, 4.0, "o");
    auto id3 = data_table->insert_point(5.0, 6.0, "x");

    ASSERT_TRUE(id1.has_value());
    ASSERT_TRUE(id2.has_value());
    ASSERT_TRUE(id3.has_value());

    // Query viewport that contains all points
    auto points = data_table->query_viewport(0.0, 10.0, 0.0, 10.0);
    EXPECT_EQ(points.size(), 3);
}

// Test querying viewport with points outside
TEST_F(DataTableTest, QueryViewportWithPointsOutside) {
    data_table->insert_point(1.0, 2.0, "x");
    data_table->insert_point(3.0, 4.0, "o");
    data_table->insert_point(15.0, 20.0, "x");

    // Query viewport that excludes third point
    auto points = data_table->query_viewport(0.0, 10.0, 0.0, 10.0);
    EXPECT_EQ(points.size(), 2);
}

// Test querying viewport at boundaries (inclusive)
TEST_F(DataTableTest, QueryViewportBoundariesInclusive) {
    data_table->insert_point(1.0, 1.0, "x");
    data_table->insert_point(5.0, 5.0, "o");
    data_table->insert_point(10.0, 10.0, "x");

    // Query with exact boundaries
    auto points = data_table->query_viewport(1.0, 10.0, 1.0, 10.0);
    EXPECT_EQ(points.size(), 3);

    // Query with tighter boundaries
    auto points2 = data_table->query_viewport(2.0, 9.0, 2.0, 9.0);
    EXPECT_EQ(points2.size(), 1);  // Only middle point
}

// Test query returns correct point data
TEST_F(DataTableTest, QueryReturnsCorrectData) {
    auto id = data_table->insert_point(1.5, 2.5, "x");
    ASSERT_TRUE(id.has_value());

    auto points = data_table->query_viewport(0.0, 10.0, 0.0, 10.0);
    ASSERT_EQ(points.size(), 1);

    EXPECT_EQ(points[0].id, id.value());
    EXPECT_DOUBLE_EQ(points[0].x, 1.5);
    EXPECT_DOUBLE_EQ(points[0].y, 2.5);
    EXPECT_EQ(points[0].target, "x");
}

// Test getting distinct targets from empty table
TEST_F(DataTableTest, GetDistinctTargetsEmpty) {
    auto targets = data_table->get_distinct_targets();
    EXPECT_TRUE(targets.empty());
}

// Test getting distinct targets with one value
TEST_F(DataTableTest, GetDistinctTargetsSingle) {
    data_table->insert_point(1.0, 2.0, "x");
    data_table->insert_point(3.0, 4.0, "x");

    auto targets = data_table->get_distinct_targets();
    EXPECT_EQ(targets.size(), 1);
    EXPECT_EQ(targets[0], "x");
}

// Test getting distinct targets with multiple values
TEST_F(DataTableTest, GetDistinctTargetsMultiple) {
    data_table->insert_point(1.0, 2.0, "x");
    data_table->insert_point(3.0, 4.0, "o");
    data_table->insert_point(5.0, 6.0, "x");
    data_table->insert_point(7.0, 8.0, "o");

    auto targets = data_table->get_distinct_targets();
    EXPECT_EQ(targets.size(), 2);

    // Check that both targets are present
    EXPECT_TRUE(std::find(targets.begin(), targets.end(), "x") != targets.end());
    EXPECT_TRUE(std::find(targets.begin(), targets.end(), "o") != targets.end());
}

// Test counting by target in empty table
TEST_F(DataTableTest, CountByTargetEmpty) {
    EXPECT_EQ(data_table->count_by_target("x"), 0);
    EXPECT_EQ(data_table->count_by_target("o"), 0);
}

// Test counting by target with points
TEST_F(DataTableTest, CountByTargetWithPoints) {
    data_table->insert_point(1.0, 2.0, "x");
    data_table->insert_point(3.0, 4.0, "o");
    data_table->insert_point(5.0, 6.0, "x");
    data_table->insert_point(7.0, 8.0, "x");

    EXPECT_EQ(data_table->count_by_target("x"), 3);
    EXPECT_EQ(data_table->count_by_target("o"), 1);
}

// Test counting non-existent target
TEST_F(DataTableTest, CountByTargetNonExistent) {
    data_table->insert_point(1.0, 2.0, "x");
    EXPECT_EQ(data_table->count_by_target("nonexistent"), 0);
}
