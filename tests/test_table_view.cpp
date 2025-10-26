#include <gtest/gtest.h>
#include "database.h"
#include "metadata.h"
#include "data_table.h"
#include "table_view.h"

using namespace datapainter;

class TableViewTest : public ::testing::Test {
protected:
    TableViewTest() : db_(":memory:") {}

    void SetUp() override {
        // Create tables
        db_.ensure_metadata_table();
        db_.ensure_unsaved_changes_table();

        // Create test table
        MetadataManager mgr(db_);
        mgr.create_data_table("test_table");

        Metadata meta;
        meta.table_name = "test_table";
        meta.target_col_name = "target";
        meta.x_axis_name = "x";
        meta.y_axis_name = "y";
        meta.x_meaning = "x_val";
        meta.o_meaning = "o_val";
        meta.valid_x_min = -10.0;
        meta.valid_x_max = 10.0;
        meta.valid_y_min = -10.0;
        meta.valid_y_max = 10.0;
        meta.show_zero_bars = false;
        mgr.insert(meta);

        data_table_ = std::make_unique<DataTable>(db_, "test_table");

        // Add some test data
        data_table_->insert_point(1.0, 2.0, "x_val");
        data_table_->insert_point(3.0, 4.0, "o_val");
        data_table_->insert_point(5.0, 6.0, "x_val");
    }

    Database db_;
    std::unique_ptr<DataTable> data_table_;
};

// Test: Display three columns (x, y, target)
TEST_F(TableViewTest, DisplayThreeColumns) {
    TableView view(db_, "test_table");

    auto rows = view.get_visible_rows();
    EXPECT_EQ(rows.size(), 3);

    // Check that each row has x, y, target
    EXPECT_GT(rows[0].x, 0.0);
    EXPECT_GT(rows[0].y, 0.0);
    EXPECT_FALSE(rows[0].target.empty());
}

// Test: Show all rows by default
TEST_F(TableViewTest, ShowAllRowsByDefault) {
    TableView view(db_, "test_table");

    auto rows = view.get_visible_rows();
    EXPECT_EQ(rows.size(), 3);
}

// Test: Navigate rows with arrow keys
TEST_F(TableViewTest, NavigateRowsWithArrowKeys) {
    TableView view(db_, "test_table");

    // Start at first row
    EXPECT_EQ(view.current_row(), 0);

    // Move down
    view.move_down();
    EXPECT_EQ(view.current_row(), 1);

    // Move down again
    view.move_down();
    EXPECT_EQ(view.current_row(), 2);

    // Can't move down past last row
    view.move_down();
    EXPECT_EQ(view.current_row(), 2);

    // Move up
    view.move_up();
    EXPECT_EQ(view.current_row(), 1);

    // Move up again
    view.move_up();
    EXPECT_EQ(view.current_row(), 0);

    // Can't move up past first row
    view.move_up();
    EXPECT_EQ(view.current_row(), 0);
}

// Test: Default filter = current viewport bounds when entering from viewport
TEST_F(TableViewTest, DefaultFilterFromViewport) {
    // Create view with viewport bounds
    TableView view(db_, "test_table", -1.0, 5.0, -1.0, 5.0);

    auto rows = view.get_visible_rows();
    // Should only show rows where x in [-1, 5] and y in [-1, 5]
    // Points: (1,2), (3,4), (5,6)
    // (5,6) has y=6 which is outside [-1, 5]
    EXPECT_EQ(rows.size(), 2);
}

// Test: Edit filter (SQL WHERE clause)
TEST_F(TableViewTest, EditFilter) {
    TableView view(db_, "test_table");

    // Initial filter shows all rows
    EXPECT_EQ(view.get_visible_rows().size(), 3);

    // Set filter to only show x_val points
    view.set_filter("target = 'x_val'");
    EXPECT_EQ(view.get_visible_rows().size(), 2);

    // Set filter to only show points with x > 2
    view.set_filter("x > 2.0");
    EXPECT_EQ(view.get_visible_rows().size(), 2);

    // Clear filter
    view.set_filter("");
    EXPECT_EQ(view.get_visible_rows().size(), 3);
}

// Test: Get filter bounds for returning to viewport
TEST_F(TableViewTest, GetFilterBounds) {
    TableView view(db_, "test_table");

    // Set filter to specific range
    view.set_filter("x >= 2.0 AND x <= 4.0 AND y >= 3.0 AND y <= 5.0");

    auto bounds = view.get_filter_bounds();
    EXPECT_TRUE(bounds.has_value());

    // Should return bounds that fit the filtered data
    EXPECT_LE(bounds->x_min, 3.0);  // Only (3,4) matches the filter
    EXPECT_GE(bounds->x_max, 3.0);
    EXPECT_LE(bounds->y_min, 4.0);
    EXPECT_GE(bounds->y_max, 4.0);
}

// Test: Handle empty result from filter
TEST_F(TableViewTest, HandleEmptyFilter) {
    TableView view(db_, "test_table");

    // Set filter that matches nothing
    view.set_filter("x > 100.0");
    EXPECT_EQ(view.get_visible_rows().size(), 0);

    // Current row should be 0 even though no rows
    EXPECT_EQ(view.current_row(), 0);
}

// Test: Get column headers
TEST_F(TableViewTest, GetColumnHeaders) {
    TableView view(db_, "test_table");

    auto headers = view.get_column_headers();
    EXPECT_EQ(headers.size(), 3);
    EXPECT_EQ(headers[0], "x");
    EXPECT_EQ(headers[1], "y");
    EXPECT_EQ(headers[2], "target");
}

// Test: Get row count
TEST_F(TableViewTest, GetRowCount) {
    TableView view(db_, "test_table");

    EXPECT_EQ(view.row_count(), 3);

    // After filter
    view.set_filter("target = 'x_val'");
    EXPECT_EQ(view.row_count(), 2);
}

// Test: Get row by index
TEST_F(TableViewTest, GetRowByIndex) {
    TableView view(db_, "test_table");

    auto row = view.get_row(1);
    EXPECT_TRUE(row.has_value());
    EXPECT_EQ(row->x, 3.0);
    EXPECT_EQ(row->y, 4.0);
    EXPECT_EQ(row->target, "o_val");

    // Invalid index
    auto invalid = view.get_row(100);
    EXPECT_FALSE(invalid.has_value());
}

// Test: Navigate to specific row
TEST_F(TableViewTest, NavigateToSpecificRow) {
    TableView view(db_, "test_table");

    view.set_current_row(2);
    EXPECT_EQ(view.current_row(), 2);

    // Invalid row number clamps to valid range
    view.set_current_row(100);
    EXPECT_EQ(view.current_row(), 2);  // Last valid row

    view.set_current_row(-1);
    EXPECT_EQ(view.current_row(), 0);  // First valid row
}
