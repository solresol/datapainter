#include <gtest/gtest.h>
#include "viewport.h"

using namespace datapainter;

// Test that cursor coordinates are properly converted from screen space to data space
// when accounting for edit area offset and border
TEST(CursorCoordinateTest, ScreenToDataWithEditAreaOffset) {
    // Setup: Screen is 10 rows x 10 cols
    // Header takes 3 rows, footer takes 1 row
    // Edit area starts at row 3, has height 6
    // Border takes 1 row at top/bottom, 1 col at left/right
    // Content area is 4 rows x 8 cols (inside border)

    const int screen_width = 10;
    const int HEADER_ROWS = 3;
    const int edit_area_start_row = HEADER_ROWS;
    const int edit_area_height = 6;

    // Create viewport: data range [-4, 4] x [-4, 4], screen size 4x8 (content area)
    int content_height = edit_area_height - 2;  // Exclude border
    int content_width = screen_width - 2;       // Exclude border
    Viewport viewport(-4.0, 4.0, -4.0, 4.0, content_height, content_width);

    // Test 1: Cursor at center of edit area content
    // Absolute screen position: row=5, col=5 (middle of 10x10 screen)
    // Edit area starts at row 3, border at row 3, content starts at row 4
    // So row=5 in screen coords = row=1 in edit area content coords (5 - 3 - 1)
    // col=5 in screen coords = col=4 in edit area content coords (5 - 1)
    int cursor_screen_row = 5;
    int cursor_screen_col = 5;

    // Convert to edit area-relative coordinates
    int cursor_content_row = cursor_screen_row - edit_area_start_row - 1;  // -1 for border
    int cursor_content_col = cursor_screen_col - 1;  // -1 for border

    EXPECT_EQ(cursor_content_row, 1);
    EXPECT_EQ(cursor_content_col, 4);

    // Convert to data coordinates
    DataCoord data = viewport.screen_to_data({cursor_content_row, cursor_content_col});

    // Center of viewport should map to data (0, 0) approximately
    // Tolerance is larger because we have a small viewport (4x8)
    EXPECT_NEAR(data.x, 0.0, 2.0);  // Should be close to center
    EXPECT_NEAR(data.y, 0.0, 2.0);
}

// Test that point creation at cursor position appears at correct screen location
TEST(CursorCoordinateTest, PointCreationAtCursorAppearsCorrectly) {
    // Simpler test: viewport maps directly to content area
    // Content area is 4 rows x 8 cols
    Viewport viewport(-4.0, 4.0, -4.0, 4.0, 4, 8);

    // Cursor at content coordinate (2, 4) - middle of content area
    ScreenCoord cursor_content{2, 4};

    // Convert to data coordinates
    DataCoord data = viewport.screen_to_data(cursor_content);

    // Convert back to screen coordinates
    auto screen_opt = viewport.data_to_screen(data);
    ASSERT_TRUE(screen_opt.has_value());

    // Should map back to same screen coordinate
    EXPECT_EQ(screen_opt->row, cursor_content.row);
    EXPECT_EQ(screen_opt->col, cursor_content.col);
}

// Test the coordinate conversion formula used in main.cpp
TEST(CursorCoordinateTest, MainLoopCoordinateConversionFormula) {
    // Simulate the main loop setup
    const int screen_height = 24;
    const int screen_width = 80;
    const int HEADER_ROWS = 3;
    const int FOOTER_ROWS = 1;
    const int edit_area_height = screen_height - HEADER_ROWS - FOOTER_ROWS;  // 20
    const int edit_area_start_row = HEADER_ROWS;  // 3

    int content_height = edit_area_height - 2;  // 18 (exclude border)
    int content_width = screen_width - 2;       // 78 (exclude border)

    // Create viewport
    Viewport viewport(-10.0, 10.0, -10.0, 10.0, content_height, content_width);

    // Cursor initialized at center of screen content area
    int cursor_screen_row = edit_area_start_row + 1 + (edit_area_height - 2) / 2;  // 3 + 1 + 9 = 13
    int cursor_screen_col = 1 + (screen_width - 2) / 2;  // 1 + 39 = 40

    // WRONG: passing absolute screen coordinates
    // DataCoord data_wrong = viewport.screen_to_data({cursor_screen_row, cursor_screen_col});

    // CORRECT: convert to edit area content coordinates first
    int cursor_content_row = cursor_screen_row - edit_area_start_row - 1;  // 13 - 3 - 1 = 9
    int cursor_content_col = cursor_screen_col - 1;  // 40 - 1 = 39

    DataCoord data_correct = viewport.screen_to_data({cursor_content_row, cursor_content_col});

    // Cursor at center should map to data (0, 0) approximately
    EXPECT_NEAR(data_correct.x, 0.0, 1.5);
    EXPECT_NEAR(data_correct.y, 0.0, 1.5);
}
