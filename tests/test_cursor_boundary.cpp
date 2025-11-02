#include <gtest/gtest.h>
#include "cursor_utils.h"
#include "viewport.h"

using namespace datapainter;

class CursorBoundaryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Screen is 24 rows x 80 cols
        screen_height_ = 24;
        screen_width_ = 80;
        HEADER_ROWS_ = 3;
        FOOTER_ROWS_ = 1;
        edit_area_height_ = screen_height_ - HEADER_ROWS_ - FOOTER_ROWS_;  // 20
        edit_area_start_row_ = HEADER_ROWS_;  // 3

        int content_height = edit_area_height_ - 2;  // 18 (exclude border)
        int content_width = screen_width_ - 2;       // 78 (exclude border)

        // Create viewport with valid ranges [-10, 10] x [-10, 10]
        // Start with viewport showing full valid range
        viewport_ = std::make_unique<Viewport>(
            -10.0, 10.0,  // data bounds
            -10.0, 10.0,  // data bounds
            -10.0, 10.0,  // valid x range
            -10.0, 10.0,  // valid y range
            content_height, content_width
        );
    }

    int screen_height_;
    int screen_width_;
    int HEADER_ROWS_;
    int FOOTER_ROWS_;
    int edit_area_height_;
    int edit_area_start_row_;
    std::unique_ptr<Viewport> viewport_;
};

// Test: Cursor can move freely within valid ranges
TEST_F(CursorBoundaryTest, CursorMovesWithinValidRanges) {
    // Cursor near center of screen should be within valid ranges
    int cursor_row = edit_area_start_row_ + 10;  // Middle of edit area
    int cursor_col = 40;  // Middle of screen width

    EXPECT_TRUE(is_cursor_position_valid(*viewport_, cursor_row, cursor_col,
                                        edit_area_start_row_));
}

// Test: Cursor cannot move beyond valid x_max (right boundary)
TEST_F(CursorBoundaryTest, PreventMoveBeyondValidXMax) {
    // Zoom in on the right edge so that moving right would exceed valid x_max
    // Set viewport to show [8, 12] x [-10, 10]
    // This extends beyond valid x_max of 10
    viewport_ = std::make_unique<Viewport>(
        8.0, 12.0,   // data bounds (extends beyond valid range)
        -10.0, 10.0,
        -10.0, 10.0,  // valid x range
        -10.0, 10.0,  // valid y range
        edit_area_height_ - 2, screen_width_ - 2
    );

    // Cursor at far right of screen maps to x > 10 (beyond valid range)
    int cursor_row = edit_area_start_row_ + 10;
    int cursor_col = screen_width_ - 2;  // Rightmost position

    EXPECT_FALSE(is_cursor_position_valid(*viewport_, cursor_row, cursor_col,
                                         edit_area_start_row_));
}

// Test: Cursor cannot move beyond valid x_min (left boundary)
TEST_F(CursorBoundaryTest, PreventMoveBeyondValidXMin) {
    // Zoom in on the left edge so that moving left would exceed valid x_min
    // Set viewport to show [-12, -8] x [-10, 10]
    viewport_ = std::make_unique<Viewport>(
        -12.0, -8.0,  // data bounds (extends beyond valid range)
        -10.0, 10.0,
        -10.0, 10.0,  // valid x range
        -10.0, 10.0,  // valid y range
        edit_area_height_ - 2, screen_width_ - 2
    );

    // Cursor at far left of screen maps to x < -10 (beyond valid range)
    int cursor_row = edit_area_start_row_ + 10;
    int cursor_col = 1;  // Leftmost position

    EXPECT_FALSE(is_cursor_position_valid(*viewport_, cursor_row, cursor_col,
                                         edit_area_start_row_));
}

// Test: Cursor cannot move beyond valid y_max (top boundary)
TEST_F(CursorBoundaryTest, PreventMoveBeyondValidYMax) {
    // Zoom in on the top edge so that moving up would exceed valid y_max
    // Set viewport to show [-10, 10] x [8, 12]
    // Remember: in screen coords, y increases downward, but in data coords y increases upward
    viewport_ = std::make_unique<Viewport>(
        -10.0, 10.0,
        8.0, 12.0,   // data y bounds (extends beyond valid range)
        -10.0, 10.0,  // valid x range
        -10.0, 10.0,  // valid y range
        edit_area_height_ - 2, screen_width_ - 2
    );

    // Cursor at top of edit area maps to y > 10 (beyond valid range)
    // Top of content area is edit_area_start_row + 1
    int cursor_row = edit_area_start_row_ + 1;
    int cursor_col = 40;

    EXPECT_FALSE(is_cursor_position_valid(*viewport_, cursor_row, cursor_col,
                                         edit_area_start_row_));
}

// Test: Cursor cannot move beyond valid y_min (bottom boundary)
TEST_F(CursorBoundaryTest, PreventMoveBeyondValidYMin) {
    // Zoom in on the bottom edge so that moving down would exceed valid y_min
    // Set viewport to show [-10, 10] x [-12, -8]
    viewport_ = std::make_unique<Viewport>(
        -10.0, 10.0,
        -12.0, -8.0,  // data y bounds (extends beyond valid range)
        -10.0, 10.0,  // valid x range
        -10.0, 10.0,  // valid y range
        edit_area_height_ - 2, screen_width_ - 2
    );

    // Cursor at bottom of edit area maps to y < -10 (beyond valid range)
    // Bottom of content area is edit_area_start_row + edit_area_height - 2
    int cursor_row = edit_area_start_row_ + edit_area_height_ - 2;
    int cursor_col = 40;

    EXPECT_FALSE(is_cursor_position_valid(*viewport_, cursor_row, cursor_col,
                                         edit_area_start_row_));
}

// Test: Edge case - cursor exactly at valid boundary is allowed
TEST_F(CursorBoundaryTest, CursorAtValidBoundaryIsAllowed) {
    // Set viewport to show exactly the valid range
    viewport_ = std::make_unique<Viewport>(
        -10.0, 10.0,  // data bounds match valid range
        -10.0, 10.0,
        -10.0, 10.0,  // valid x range
        -10.0, 10.0,  // valid y range
        edit_area_height_ - 2, screen_width_ - 2
    );

    // Test all corners - they should all be valid since viewport exactly matches valid range

    // Top-left corner
    EXPECT_TRUE(is_cursor_position_valid(*viewport_, edit_area_start_row_ + 1, 1,
                                        edit_area_start_row_));

    // Top-right corner
    EXPECT_TRUE(is_cursor_position_valid(*viewport_, edit_area_start_row_ + 1, screen_width_ - 2,
                                        edit_area_start_row_));

    // Bottom-left corner
    EXPECT_TRUE(is_cursor_position_valid(*viewport_, edit_area_start_row_ + edit_area_height_ - 2, 1,
                                        edit_area_start_row_));

    // Bottom-right corner
    EXPECT_TRUE(is_cursor_position_valid(*viewport_, edit_area_start_row_ + edit_area_height_ - 2,
                                        screen_width_ - 2, edit_area_start_row_));
}
