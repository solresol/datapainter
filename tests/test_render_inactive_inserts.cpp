// Test to verify EditAreaRenderer skips inactive insert changes
#include <gtest/gtest.h>
#include "../include/database.h"
#include "../include/unsaved_changes.h"
#include "../include/edit_area_renderer.h"
#include "../include/viewport.h"
#include "../include/data_table.h"
#include "../include/terminal.h"

using namespace datapainter;

TEST(RenderInactiveInsertsTest, SkipsInactiveInserts) {
    // Create in-memory database
    Database db(":memory:");
    ASSERT_TRUE(db.is_open());

    // Create test data table
    const char* create_table_sql = R"(
        CREATE TABLE test_data (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            x REAL NOT NULL,
            y REAL NOT NULL,
            label TEXT NOT NULL
        )
    )";
    ASSERT_TRUE(db.execute(create_table_sql));

    // Create unsaved_changes table
    ASSERT_TRUE(db.ensure_unsaved_changes_table());

    // Create an insert change that is active
    UnsavedChanges uc(db);
    auto change_id_1 = uc.record_insert("test_data", 0.0, 0.0, "positive");
    ASSERT_TRUE(change_id_1.has_value());

    // Create another insert change and mark it inactive
    auto change_id_2 = uc.record_insert("test_data", 1.0, 0.0, "positive");
    ASSERT_TRUE(change_id_2.has_value());
    ASSERT_TRUE(uc.mark_change_inactive(change_id_2.value()));

    // Get changes
    auto changes = uc.get_changes("test_data");
    ASSERT_EQ(changes.size(), 2);

    // Verify one is active, one is inactive
    bool has_active = false;
    bool has_inactive = false;
    for (const auto& change : changes) {
        if (change.is_active) has_active = true;
        if (!change.is_active) has_inactive = true;
    }
    EXPECT_TRUE(has_active) << "Should have at least one active change";
    EXPECT_TRUE(has_inactive) << "Should have at least one inactive change";

    // Set up viewport and rendering
    Terminal terminal(24, 80);
    Viewport viewport(-10.0, 10.0, -10.0, 10.0, -10.0, 10.0, -10.0, 10.0, 18, 78);
    DataTable dt(db, "test_data");

    // Render points
    EditAreaRenderer renderer;
    renderer.render(terminal, viewport, dt, changes, 3, 20, 80, 10, 40, "positive", "negative");

    // Check the rendered output
    // Active insert at (0,0) should appear, inactive insert at (1,0) should not
    std::string screen = terminal.get_debug_string();

    // Count 'x' characters in the middle content area (excluding borders)
    // There should be exactly 1 'x' from the active insert
    int x_count = 0;
    auto lines = terminal.get_display_lines();
    for (size_t row = 4; row < 22; ++row) {
        if (row >= lines.size()) break;
        for (size_t col = 1; col < 79; ++col) {
            if (col >= lines[row].size()) break;
            if (lines[row][col] == 'x' || lines[row][col] == 'X') {
                x_count++;
            }
        }
    }

    // Should have exactly 1 point visible (the active insert)
    EXPECT_EQ(x_count, 1) << "Should render only the active insert, not the inactive one";
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
