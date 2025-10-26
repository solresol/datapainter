#include <gtest/gtest.h>
#include "point_editor.h"
#include "database.h"
#include "metadata.h"
#include "data_table.h"
#include "unsaved_changes.h"

using namespace datapainter;

class PointEditorTest : public ::testing::Test {
protected:
    PointEditorTest() : db_(":memory:") {}

    void SetUp() override {
        // Create metadata table
        db_.ensure_metadata_table();
        db_.ensure_unsaved_changes_table();

        // Create test table
        MetadataManager mgr(db_);
        mgr.create_data_table("test_table");

        Metadata meta;
        meta.table_name = "test_table";
        meta.target_col_name = "target_col";
        meta.x_axis_name = "x_axis";
        meta.y_axis_name = "y_axis";
        meta.x_meaning = "x_meaning";
        meta.o_meaning = "o_meaning";
        meta.valid_x_min = -10.0;
        meta.valid_x_max = 10.0;
        meta.valid_y_min = -10.0;
        meta.valid_y_max = 10.0;
        meta.show_zero_bars = false;
        mgr.insert(meta);
    }

    Database db_;
};

// Test: Create x point at cursor position
TEST_F(PointEditorTest, CreateXPoint) {
    PointEditor editor(db_, "test_table");

    bool result = editor.create_point(2.5, 3.5, 'x');
    EXPECT_TRUE(result);

    // Verify point was created
    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-10.0, 10.0, -10.0, 10.0);
    EXPECT_EQ(points.size(), 1);
    EXPECT_DOUBLE_EQ(points[0].x, 2.5);
    EXPECT_DOUBLE_EQ(points[0].y, 3.5);
    EXPECT_EQ(points[0].target, "x_meaning");
}

// Test: Create o point at cursor position
TEST_F(PointEditorTest, CreateOPoint) {
    PointEditor editor(db_, "test_table");

    bool result = editor.create_point(2.5, 3.5, 'o');
    EXPECT_TRUE(result);

    // Verify point was created
    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-10.0, 10.0, -10.0, 10.0);
    EXPECT_EQ(points.size(), 1);
    EXPECT_DOUBLE_EQ(points[0].x, 2.5);
    EXPECT_DOUBLE_EQ(points[0].y, 3.5);
    EXPECT_EQ(points[0].target, "o_meaning");
}

// Test: Reject point creation outside valid x range
TEST_F(PointEditorTest, RejectPointOutsideValidXRange) {
    PointEditor editor(db_, "test_table");

    bool result = editor.create_point(15.0, 5.0, 'x');  // x > x_max (10.0)
    EXPECT_FALSE(result);

    result = editor.create_point(-15.0, 5.0, 'x');  // x < x_min (-10.0)
    EXPECT_FALSE(result);

    // Verify no points were created
    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-20.0, 20.0, -20.0, 20.0);
    EXPECT_EQ(points.size(), 0);
}

// Test: Reject point creation outside valid y range
TEST_F(PointEditorTest, RejectPointOutsideValidYRange) {
    PointEditor editor(db_, "test_table");

    bool result = editor.create_point(5.0, 15.0, 'o');  // y > y_max (10.0)
    EXPECT_FALSE(result);

    result = editor.create_point(5.0, -15.0, 'o');  // y < y_min (-10.0)
    EXPECT_FALSE(result);

    // Verify no points were created
    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-20.0, 20.0, -20.0, 20.0);
    EXPECT_EQ(points.size(), 0);
}

// Test: Record point creation in unsaved_changes
TEST_F(PointEditorTest, RecordPointCreationInUnsavedChanges) {
    PointEditor editor(db_, "test_table");

    editor.create_point(2.5, 3.5, 'x');

    // Verify change was recorded
    UnsavedChanges uc(db_);
    auto changes = uc.get_changes("test_table");
    EXPECT_EQ(changes.size(), 1);
    EXPECT_EQ(changes[0].action, "insert");
}

// Test: Create multiple points
TEST_F(PointEditorTest, CreateMultiplePoints) {
    PointEditor editor(db_, "test_table");

    editor.create_point(1.0, 1.0, 'x');
    editor.create_point(2.0, 2.0, 'o');
    editor.create_point(3.0, 3.0, 'x');

    // Verify all points were created
    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-10.0, 10.0, -10.0, 10.0);
    EXPECT_EQ(points.size(), 3);
}

// Test: Delete all points at cursor position
TEST_F(PointEditorTest, DeletePointsAtCursor) {
    PointEditor editor(db_, "test_table");

    // Create some points
    DataTable dt(db_, "test_table");
    dt.insert_point(2.5, 3.5, "x_meaning");
    dt.insert_point(2.6, 3.6, "o_meaning");  // Close to first point
    dt.insert_point(7.0, 8.0, "x_meaning");  // Far away

    // Delete points at (2.5, 3.5) with cell size 1.0
    int deleted = editor.delete_points_at_cursor(2.5, 3.5, 1.0);
    EXPECT_EQ(deleted, 2);  // Should delete both nearby points

    // Verify only distant point remains
    auto points = dt.query_viewport(-10.0, 10.0, -10.0, 10.0);
    EXPECT_EQ(points.size(), 1);
    EXPECT_DOUBLE_EQ(points[0].x, 7.0);
}

// Test: Delete points rounds to screen cell
TEST_F(PointEditorTest, DeletePointsRoundsToCell) {
    PointEditor editor(db_, "test_table");

    // Create points within same cell
    DataTable dt(db_, "test_table");
    dt.insert_point(2.1, 3.1, "x_meaning");
    dt.insert_point(2.9, 3.9, "o_meaning");

    // Delete with cell size 1.0 - both points should be in cell [2,3]
    int deleted = editor.delete_points_at_cursor(2.5, 3.5, 1.0);
    EXPECT_EQ(deleted, 2);

    auto points = dt.query_viewport(-10.0, 10.0, -10.0, 10.0);
    EXPECT_EQ(points.size(), 0);
}

// Test: Record deletion in unsaved_changes
TEST_F(PointEditorTest, RecordDeletionInUnsavedChanges) {
    PointEditor editor(db_, "test_table");

    // Create a point first
    DataTable dt(db_, "test_table");
    dt.insert_point(2.5, 3.5, "x_meaning");

    // Clear initial unsaved changes
    UnsavedChanges uc(db_);
    uc.clear_changes("test_table");

    // Delete the point
    editor.delete_points_at_cursor(2.5, 3.5, 1.0);

    // Verify deletion was recorded
    auto changes = uc.get_changes("test_table");
    EXPECT_GE(changes.size(), 1);

    // Check that at least one delete action exists
    bool found_delete = false;
    for (const auto& change : changes) {
        if (change.action == "delete") {
            found_delete = true;
            break;
        }
    }
    EXPECT_TRUE(found_delete);
}

// Test: Delete returns 0 when no points at cursor
TEST_F(PointEditorTest, DeleteReturnsZeroWhenEmpty) {
    PointEditor editor(db_, "test_table");

    int deleted = editor.delete_points_at_cursor(5.0, 5.0, 1.0);
    EXPECT_EQ(deleted, 0);
}

// Test: Convert o points to x at cursor
TEST_F(PointEditorTest, ConvertOPointsToX) {
    PointEditor editor(db_, "test_table");

    // Create o points
    DataTable dt(db_, "test_table");
    dt.insert_point(2.5, 3.5, "o_meaning");
    dt.insert_point(2.6, 3.6, "o_meaning");

    // Convert to x
    int converted = editor.convert_points_at_cursor(2.5, 3.5, 1.0, 'x');
    EXPECT_EQ(converted, 2);

    // Verify conversion
    auto points = dt.query_viewport(-10.0, 10.0, -10.0, 10.0);
    EXPECT_EQ(points.size(), 2);
    for (const auto& point : points) {
        EXPECT_EQ(point.target, "x_meaning");
    }
}

// Test: Convert x points to o at cursor
TEST_F(PointEditorTest, ConvertXPointsToO) {
    PointEditor editor(db_, "test_table");

    // Create x points
    DataTable dt(db_, "test_table");
    dt.insert_point(2.5, 3.5, "x_meaning");
    dt.insert_point(2.6, 3.6, "x_meaning");

    // Convert to o
    int converted = editor.convert_points_at_cursor(2.5, 3.5, 1.0, 'o');
    EXPECT_EQ(converted, 2);

    // Verify conversion
    auto points = dt.query_viewport(-10.0, 10.0, -10.0, 10.0);
    EXPECT_EQ(points.size(), 2);
    for (const auto& point : points) {
        EXPECT_EQ(point.target, "o_meaning");
    }
}

// Test: Flip points (x↔o) at cursor
TEST_F(PointEditorTest, FlipPoints) {
    PointEditor editor(db_, "test_table");

    // Create mixed points
    DataTable dt(db_, "test_table");
    dt.insert_point(2.5, 3.5, "x_meaning");
    dt.insert_point(2.6, 3.6, "o_meaning");

    // Flip points
    int flipped = editor.flip_points_at_cursor(2.5, 3.5, 1.0);
    EXPECT_EQ(flipped, 2);

    // Verify flip - x became o, o became x
    auto points = dt.query_viewport(-10.0, 10.0, -10.0, 10.0);
    EXPECT_EQ(points.size(), 2);

    // Count each type
    int x_count = 0, o_count = 0;
    for (const auto& point : points) {
        if (point.target == "x_meaning") x_count++;
        if (point.target == "o_meaning") o_count++;
    }
    EXPECT_EQ(x_count, 1);
    EXPECT_EQ(o_count, 1);
}

// Test: Record conversion in unsaved_changes
TEST_F(PointEditorTest, RecordConversionInUnsavedChanges) {
    PointEditor editor(db_, "test_table");

    // Create a point
    DataTable dt(db_, "test_table");
    dt.insert_point(2.5, 3.5, "x_meaning");

    // Clear initial unsaved changes
    UnsavedChanges uc(db_);
    uc.clear_changes("test_table");

    // Convert the point
    editor.convert_points_at_cursor(2.5, 3.5, 1.0, 'o');

    // Verify conversion was recorded as update
    auto changes = uc.get_changes("test_table");
    EXPECT_GE(changes.size(), 1);

    bool found_update = false;
    for (const auto& change : changes) {
        if (change.action == "update") {
            found_update = true;
            break;
        }
    }
    EXPECT_TRUE(found_update);
}

// Test: Conversion only affects specified type
TEST_F(PointEditorTest, ConversionOnlyAffectsSpecifiedType) {
    PointEditor editor(db_, "test_table");

    // Create mixed points
    DataTable dt(db_, "test_table");
    dt.insert_point(2.5, 3.5, "x_meaning");
    dt.insert_point(2.6, 3.6, "o_meaning");

    // Convert only o to x
    int converted = editor.convert_points_at_cursor(2.5, 3.5, 1.0, 'x');
    EXPECT_EQ(converted, 1);  // Only converted the o point

    // Verify only o point was converted
    auto points = dt.query_viewport(-10.0, 10.0, -10.0, 10.0);
    EXPECT_EQ(points.size(), 2);
    for (const auto& point : points) {
        EXPECT_EQ(point.target, "x_meaning");
    }
}

// Test: Get points at cursor returns correct points
TEST_F(PointEditorTest, GetPointsAtCursor) {
    PointEditor editor(db_, "test_table");

    // Create points
    DataTable dt(db_, "test_table");
    dt.insert_point(2.5, 3.5, "x_meaning");
    dt.insert_point(2.6, 3.6, "o_meaning");
    dt.insert_point(7.0, 8.0, "x_meaning");  // Far away

    // Get points at cursor
    auto points = editor.get_points_at_cursor(2.5, 3.5, 1.0);
    EXPECT_EQ(points.size(), 2);
}

// Test: Point at valid range boundaries
TEST_F(PointEditorTest, CreatePointAtValidBoundaries) {
    PointEditor editor(db_, "test_table");

    // Create points at boundaries (should succeed)
    EXPECT_TRUE(editor.create_point(-10.0, -10.0, 'x'));
    EXPECT_TRUE(editor.create_point(10.0, 10.0, 'o'));

    DataTable dt(db_, "test_table");
    auto points = dt.query_viewport(-10.0, 10.0, -10.0, 10.0);
    EXPECT_EQ(points.size(), 2);
}
