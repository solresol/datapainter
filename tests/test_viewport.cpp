#include <gtest/gtest.h>
#include "viewport.h"
#include <cmath>

using namespace datapainter;

// Test fixture for viewport tests
class ViewportTest : public ::testing::Test {
protected:
    // Standard viewport: data [-1,1] x [-1,1], screen 20x40
    std::unique_ptr<Viewport> viewport;

    void SetUp() override {
        viewport = std::make_unique<Viewport>(-1.0, 1.0, -1.0, 1.0, 20, 40);
    }
};

// Test viewport initialization
TEST_F(ViewportTest, InitializeViewport) {
    EXPECT_DOUBLE_EQ(viewport->data_x_min(), -1.0);
    EXPECT_DOUBLE_EQ(viewport->data_x_max(), 1.0);
    EXPECT_DOUBLE_EQ(viewport->data_y_min(), -1.0);
    EXPECT_DOUBLE_EQ(viewport->data_y_max(), 1.0);
    EXPECT_EQ(viewport->screen_height(), 20);
    EXPECT_EQ(viewport->screen_width(), 40);
}

// Test screen to data transformation (center)
TEST_F(ViewportTest, ScreenToDataCenter) {
    // Center of screen should map to center of data
    ScreenCoord screen{10, 20};  // Middle of 20x40 screen
    DataCoord data = viewport->screen_to_data(screen);

    EXPECT_NEAR(data.x, 0.0, 0.1);  // Should be near center
    EXPECT_NEAR(data.y, 0.0, 0.1);
}

// Test screen to data transformation (corners)
TEST_F(ViewportTest, ScreenToDataCorners) {
    // Top-left corner (note: y increases downward on screen, upward in data)
    ScreenCoord top_left{0, 0};
    DataCoord data_tl = viewport->screen_to_data(top_left);
    EXPECT_NEAR(data_tl.x, -1.0, 0.1);
    EXPECT_NEAR(data_tl.y, 1.0, 0.1);  // Top of screen = max y in data

    // Bottom-right corner
    ScreenCoord bottom_right{19, 39};  // Last valid indices
    DataCoord data_br = viewport->screen_to_data(bottom_right);
    EXPECT_NEAR(data_br.x, 1.0, 0.1);
    EXPECT_NEAR(data_br.y, -1.0, 0.1);  // Bottom of screen = min y in data
}

// Test data to screen transformation (center)
TEST_F(ViewportTest, DataToScreenCenter) {
    DataCoord data{0.0, 0.0};
    auto screen = viewport->data_to_screen(data);

    ASSERT_TRUE(screen.has_value());
    EXPECT_NEAR(screen->row, 10, 1);  // Should be near middle
    EXPECT_NEAR(screen->col, 20, 1);
}

// Test data to screen transformation (corners)
TEST_F(ViewportTest, DataToScreenCorners) {
    // Top-left in data space
    DataCoord data_tl{-1.0, 1.0};
    auto screen_tl = viewport->data_to_screen(data_tl);
    ASSERT_TRUE(screen_tl.has_value());
    EXPECT_NEAR(screen_tl->row, 0, 1);
    EXPECT_NEAR(screen_tl->col, 0, 1);

    // Bottom-right in data space
    DataCoord data_br{1.0, -1.0};
    auto screen_br = viewport->data_to_screen(data_br);
    ASSERT_TRUE(screen_br.has_value());
    EXPECT_NEAR(screen_br->row, 19, 1);
    EXPECT_NEAR(screen_br->col, 39, 1);
}

// Test data outside viewport returns nullopt
TEST_F(ViewportTest, DataToScreenOutsideViewport) {
    DataCoord outside{5.0, 5.0};
    auto screen = viewport->data_to_screen(outside);
    EXPECT_FALSE(screen.has_value());
}

// Test is_visible for points inside viewport
TEST_F(ViewportTest, IsVisibleInside) {
    DataCoord inside{0.5, 0.5};
    EXPECT_TRUE(viewport->is_visible(inside));

    DataCoord at_edge{1.0, 1.0};
    EXPECT_TRUE(viewport->is_visible(at_edge));
}

// Test is_visible for points outside viewport
TEST_F(ViewportTest, IsVisibleOutside) {
    DataCoord outside_x{2.0, 0.0};
    EXPECT_FALSE(viewport->is_visible(outside_x));

    DataCoord outside_y{0.0, 2.0};
    EXPECT_FALSE(viewport->is_visible(outside_y));

    DataCoord outside_both{2.0, 2.0};
    EXPECT_FALSE(viewport->is_visible(outside_both));
}

// Test round to cell
TEST_F(ViewportTest, RoundToCell) {
    // Points close together should round to same cell
    DataCoord point1{0.01, 0.01};
    DataCoord point2{0.02, 0.02};

    DataCoord rounded1 = viewport->round_to_cell(point1);
    DataCoord rounded2 = viewport->round_to_cell(point2);

    // Should be close (within cell resolution)
    EXPECT_NEAR(rounded1.x, rounded2.x, 0.001);
    EXPECT_NEAR(rounded1.y, rounded2.y, 0.001);
}

// Test zoom in (halve viewport size)
TEST_F(ViewportTest, ZoomIn) {
    DataCoord center{0.0, 0.0};
    viewport->zoom_in(center);

    // Viewport should be half the size
    double x_range = viewport->data_x_max() - viewport->data_x_min();
    double y_range = viewport->data_y_max() - viewport->data_y_min();

    EXPECT_NEAR(x_range, 1.0, 0.01);  // Was 2.0, now 1.0
    EXPECT_NEAR(y_range, 1.0, 0.01);

    // Center should still be at 0,0
    EXPECT_NEAR((viewport->data_x_min() + viewport->data_x_max()) / 2.0, 0.0, 0.01);
    EXPECT_NEAR((viewport->data_y_min() + viewport->data_y_max()) / 2.0, 0.0, 0.01);
}

// Test zoom out (double viewport size)
TEST_F(ViewportTest, ZoomOut) {
    DataCoord center{0.0, 0.0};
    viewport->zoom_out(center);

    // Viewport should be double the size
    double x_range = viewport->data_x_max() - viewport->data_x_min();
    double y_range = viewport->data_y_max() - viewport->data_y_min();

    EXPECT_NEAR(x_range, 4.0, 0.01);  // Was 2.0, now 4.0
    EXPECT_NEAR(y_range, 4.0, 0.01);

    // Center should still be at 0,0
    EXPECT_NEAR((viewport->data_x_min() + viewport->data_x_max()) / 2.0, 0.0, 0.01);
    EXPECT_NEAR((viewport->data_y_min() + viewport->data_y_max()) / 2.0, 0.0, 0.01);
}

// Test zoom in off-center
TEST_F(ViewportTest, ZoomInOffCenter) {
    DataCoord center{0.5, 0.5};
    viewport->zoom_in(center);

    // Viewport should be centered on 0.5, 0.5
    double center_x = (viewport->data_x_min() + viewport->data_x_max()) / 2.0;
    double center_y = (viewport->data_y_min() + viewport->data_y_max()) / 2.0;

    EXPECT_NEAR(center_x, 0.5, 0.01);
    EXPECT_NEAR(center_y, 0.5, 0.01);
}

// Test zoom to fit all (placeholder - implementation will need data bounds)
TEST_F(ViewportTest, ZoomToFitAll) {
    // For now, just test that it doesn't crash
    // Real implementation will need actual data bounds
    viewport->zoom_to_fit_all();

    // Viewport should still be valid
    EXPECT_LT(viewport->data_x_min(), viewport->data_x_max());
    EXPECT_LT(viewport->data_y_min(), viewport->data_y_max());
}

// Test coordinate roundtrip (screen -> data -> screen)
TEST_F(ViewportTest, CoordinateRoundtripScreenToData) {
    ScreenCoord original{10, 20};
    DataCoord data = viewport->screen_to_data(original);
    auto back_to_screen = viewport->data_to_screen(data);

    ASSERT_TRUE(back_to_screen.has_value());
    EXPECT_NEAR(back_to_screen->row, original.row, 1);
    EXPECT_NEAR(back_to_screen->col, original.col, 1);
}

// Test coordinate roundtrip (data -> screen -> data)
TEST_F(ViewportTest, CoordinateRoundtripDataToScreen) {
    DataCoord original{0.5, 0.5};
    auto screen = viewport->data_to_screen(original);
    ASSERT_TRUE(screen.has_value());

    DataCoord back_to_data = viewport->screen_to_data(*screen);

    // Should be close (some rounding error expected)
    EXPECT_NEAR(back_to_data.x, original.x, 0.1);
    EXPECT_NEAR(back_to_data.y, original.y, 0.1);
}

// Test viewport with non-square aspect ratio
TEST(ViewportAspectRatioTest, NonSquareViewport) {
    // Data is square, but screen is wide
    Viewport vp(-1.0, 1.0, -1.0, 1.0, 20, 80);

    DataCoord center{0.0, 0.0};
    auto screen = vp.data_to_screen(center);

    ASSERT_TRUE(screen.has_value());
    EXPECT_NEAR(screen->row, 10, 1);
    EXPECT_NEAR(screen->col, 40, 1);  // Center of 80-wide screen
}

// Test viewport with non-origin data bounds
TEST(ViewportOffsetTest, NonOriginBounds) {
    // Data from [10, 20] x [30, 40]
    Viewport vp(10.0, 20.0, 30.0, 40.0, 20, 40);

    DataCoord center{15.0, 35.0};
    auto screen = vp.data_to_screen(center);

    ASSERT_TRUE(screen.has_value());
    EXPECT_NEAR(screen->row, 10, 1);
    EXPECT_NEAR(screen->col, 20, 1);
}

// Test: Respect valid range constraints during zoom
TEST(ViewportConstraintsTest, ZoomRespectValidRanges) {
    // Create viewport with valid ranges [-10, 10] x [-10, 10]
    // Start with viewport showing [-5, 5] x [-5, 5]
    Viewport vp(-5.0, 5.0, -5.0, 5.0,
                -10.0, 10.0, -10.0, 10.0,
                20, 40);

    // Zoom out repeatedly - should stop at valid ranges
    DataCoord center{0.0, 0.0};
    vp.zoom_out(center);  // Should expand
    vp.zoom_out(center);  // Should expand more
    vp.zoom_out(center);  // Should hit valid range limits

    // Should be clamped to valid ranges
    EXPECT_DOUBLE_EQ(vp.data_x_min(), -10.0);
    EXPECT_DOUBLE_EQ(vp.data_x_max(), 10.0);
    EXPECT_DOUBLE_EQ(vp.data_y_min(), -10.0);
    EXPECT_DOUBLE_EQ(vp.data_y_max(), 10.0);
}

// Test: Zoom in stays within valid ranges
TEST(ViewportConstraintsTest, ZoomInStaysWithinValidRanges) {
    // Create viewport starting at valid range boundaries
    Viewport vp(-10.0, 10.0, -10.0, 10.0,
                -10.0, 10.0, -10.0, 10.0,
                20, 40);

    // Zoom in from center
    DataCoord center{0.0, 0.0};
    vp.zoom_in(center);

    // Should be smaller than valid ranges but still within them
    EXPECT_GT(vp.data_x_min(), -10.0);
    EXPECT_LT(vp.data_x_max(), 10.0);
    EXPECT_GT(vp.data_y_min(), -10.0);
    EXPECT_LT(vp.data_y_max(), 10.0);
}

// Test: Pan right
TEST(ViewportPanTest, PanRight) {
    // Create viewport with valid ranges
    Viewport vp(-5.0, 5.0, -5.0, 5.0,
                -10.0, 10.0, -10.0, 10.0,
                20, 40);

    double original_x_min = vp.data_x_min();
    double original_x_max = vp.data_x_max();

    vp.pan_right();

    // X bounds should have shifted right
    EXPECT_GT(vp.data_x_min(), original_x_min);
    EXPECT_GT(vp.data_x_max(), original_x_max);
    // Y bounds should not have changed
    EXPECT_DOUBLE_EQ(vp.data_y_min(), -5.0);
    EXPECT_DOUBLE_EQ(vp.data_y_max(), 5.0);
}

// Test: Pan left
TEST(ViewportPanTest, PanLeft) {
    // Create viewport with valid ranges
    Viewport vp(-5.0, 5.0, -5.0, 5.0,
                -10.0, 10.0, -10.0, 10.0,
                20, 40);

    double original_x_min = vp.data_x_min();
    double original_x_max = vp.data_x_max();

    vp.pan_left();

    // X bounds should have shifted left
    EXPECT_LT(vp.data_x_min(), original_x_min);
    EXPECT_LT(vp.data_x_max(), original_x_max);
    // Y bounds should not have changed
    EXPECT_DOUBLE_EQ(vp.data_y_min(), -5.0);
    EXPECT_DOUBLE_EQ(vp.data_y_max(), 5.0);
}

// Test: Pan up
TEST(ViewportPanTest, PanUp) {
    // Create viewport with valid ranges
    Viewport vp(-5.0, 5.0, -5.0, 5.0,
                -10.0, 10.0, -10.0, 10.0,
                20, 40);

    double original_y_min = vp.data_y_min();
    double original_y_max = vp.data_y_max();

    vp.pan_up();

    // Y bounds should have shifted up
    EXPECT_GT(vp.data_y_min(), original_y_min);
    EXPECT_GT(vp.data_y_max(), original_y_max);
    // X bounds should not have changed
    EXPECT_DOUBLE_EQ(vp.data_x_min(), -5.0);
    EXPECT_DOUBLE_EQ(vp.data_x_max(), 5.0);
}

// Test: Pan down
TEST(ViewportPanTest, PanDown) {
    // Create viewport with valid ranges
    Viewport vp(-5.0, 5.0, -5.0, 5.0,
                -10.0, 10.0, -10.0, 10.0,
                20, 40);

    double original_y_min = vp.data_y_min();
    double original_y_max = vp.data_y_max();

    vp.pan_down();

    // Y bounds should have shifted down
    EXPECT_LT(vp.data_y_min(), original_y_min);
    EXPECT_LT(vp.data_y_max(), original_y_max);
    // X bounds should not have changed
    EXPECT_DOUBLE_EQ(vp.data_x_min(), -5.0);
    EXPECT_DOUBLE_EQ(vp.data_x_max(), 5.0);
}

// Test: Prevent pan beyond valid ranges (right)
TEST(ViewportPanTest, PreventPanBeyondValidRangesRight) {
    // Create viewport near right edge of valid range
    Viewport vp(5.0, 9.0, -5.0, 5.0,
                -10.0, 10.0, -10.0, 10.0,
                20, 40);

    // Try to pan right multiple times
    vp.pan_right();
    vp.pan_right();
    vp.pan_right();

    // Should be clamped to valid range
    EXPECT_LE(vp.data_x_max(), 10.0);
    EXPECT_GE(vp.data_x_min(), -10.0);
}

// Test: Prevent pan beyond valid ranges (left)
TEST(ViewportPanTest, PreventPanBeyondValidRangesLeft) {
    // Create viewport near left edge of valid range
    Viewport vp(-9.0, -5.0, -5.0, 5.0,
                -10.0, 10.0, -10.0, 10.0,
                20, 40);

    // Try to pan left multiple times
    vp.pan_left();
    vp.pan_left();
    vp.pan_left();

    // Should be clamped to valid range
    EXPECT_GE(vp.data_x_min(), -10.0);
    EXPECT_LE(vp.data_x_max(), 10.0);
}

// Test: Cursor movement within viewport (no pan)
TEST(ViewportPanTest, CursorMovementWithinViewport) {
    // This test verifies that cursor movement doesn't affect viewport bounds
    // The actual cursor movement is handled by InputHandler, not Viewport
    Viewport vp(-5.0, 5.0, -5.0, 5.0,
                -10.0, 10.0, -10.0, 10.0,
                20, 40);

    double original_x_min = vp.data_x_min();
    double original_x_max = vp.data_x_max();
    double original_y_min = vp.data_y_min();
    double original_y_max = vp.data_y_max();

    // Simulate checking cursor positions (viewport should not change)
    DataCoord cursor1{0.0, 0.0};
    DataCoord cursor2{2.0, 3.0};
    DataCoord cursor3{-2.0, -3.0};

    // Verify positions are visible
    EXPECT_TRUE(vp.is_visible(cursor1));
    EXPECT_TRUE(vp.is_visible(cursor2));
    EXPECT_TRUE(vp.is_visible(cursor3));

    // Viewport bounds should not have changed
    EXPECT_DOUBLE_EQ(vp.data_x_min(), original_x_min);
    EXPECT_DOUBLE_EQ(vp.data_x_max(), original_x_max);
    EXPECT_DOUBLE_EQ(vp.data_y_min(), original_y_min);
    EXPECT_DOUBLE_EQ(vp.data_y_max(), original_y_max);
}
