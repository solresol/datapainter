#include <gtest/gtest.h>
#include "footer_renderer.h"
#include "terminal.h"
#include <string>

using namespace datapainter;

class FooterRendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        terminal_.set_dimensions(10, 80);
    }

    Terminal terminal_;
};

// Test: Display zoom controls (+ - =)
TEST_F(FooterRendererTest, DisplayZoomControls) {
    FooterRenderer renderer;

    renderer.render(terminal_, 2.5, 3.5, -1.0, 1.0, -1.0, 1.0, 0);

    // Footer should be at the bottom row (row 9)
    std::string footer = terminal_.get_row(9);

    // Should contain zoom control indicators
    EXPECT_NE(footer.find('+'), std::string::npos);
    EXPECT_NE(footer.find('-'), std::string::npos);
    EXPECT_NE(footer.find('='), std::string::npos);
}

// Test: Display cursor position
TEST_F(FooterRendererTest, DisplayCursorPosition) {
    FooterRenderer renderer;

    double cursor_x = 2.5;
    double cursor_y = 3.5;

    renderer.render(terminal_, cursor_x, cursor_y, -1.0, 1.0, -1.0, 1.0, 0);

    std::string footer = terminal_.get_row(9);

    // Should display cursor coordinates
    EXPECT_TRUE(footer.find("2.5") != std::string::npos ||
                footer.find("2") != std::string::npos);
    EXPECT_TRUE(footer.find("3.5") != std::string::npos ||
                footer.find("3") != std::string::npos);
}

// Test: Display valid ranges
TEST_F(FooterRendererTest, DisplayValidRanges) {
    FooterRenderer renderer;

    double x_min = -10.0;
    double x_max = 10.0;
    double y_min = -5.0;
    double y_max = 5.0;

    renderer.render(terminal_, 0.0, 0.0, x_min, x_max, y_min, y_max, 0);

    std::string footer = terminal_.get_row(9);

    // Should contain range information
    EXPECT_TRUE(footer.find("-10") != std::string::npos ||
                footer.find("10") != std::string::npos);
    EXPECT_TRUE(footer.find("-5") != std::string::npos ||
                footer.find("5") != std::string::npos);
}

// Test: Display action buttons (Tabular, Undo, Quit, Save)
TEST_F(FooterRendererTest, DisplayActionButtons) {
    FooterRenderer renderer;

    renderer.render(terminal_, 0.0, 0.0, -1.0, 1.0, -1.0, 1.0, 0);

    std::string footer = terminal_.get_row(9);

    // Should contain action button labels or shortcuts
    // Check for common button indicators (# for tabular, u for undo, q for quit, s for save)
    EXPECT_TRUE(footer.find('#') != std::string::npos ||
                footer.find("Tab") != std::string::npos ||
                footer.find("Tabular") != std::string::npos);
}

// Test: Highlight focused button
TEST_F(FooterRendererTest, HighlightFocusedButton) {
    FooterRenderer renderer;

    // Render with no focus
    renderer.render(terminal_, 0.0, 0.0, -1.0, 1.0, -1.0, 1.0, 0);
    std::string footer_no_focus = terminal_.get_row(9);

    // Render with focus on button 1
    terminal_.clear_buffer();
    renderer.render(terminal_, 0.0, 0.0, -1.0, 1.0, -1.0, 1.0, 1);
    std::string footer_focus_1 = terminal_.get_row(9);

    // The two renderings should be different
    EXPECT_NE(footer_no_focus, footer_focus_1);
}

// Test: Footer fits within screen width
TEST_F(FooterRendererTest, FitsWithinScreenWidth) {
    FooterRenderer renderer;

    renderer.render(terminal_, 0.0, 0.0, -1.0, 1.0, -1.0, 1.0, 0);

    std::string footer = terminal_.get_row(9);
    EXPECT_LE(footer.length(), static_cast<size_t>(terminal_.cols()));
}

// Test: Footer uses bottom row
TEST_F(FooterRendererTest, UsesBottomRow) {
    FooterRenderer renderer;

    renderer.render(terminal_, 0.0, 0.0, -1.0, 1.0, -1.0, 1.0, 0);

    // Footer should be at the last row (row 9 for 10-row terminal)
    std::string footer = terminal_.get_row(9);

    // Should have some content
    EXPECT_NE(footer.find_first_not_of(' '), std::string::npos);
}

// Test: Cursor position formatted correctly with negative values
TEST_F(FooterRendererTest, DisplayNegativeCursorPosition) {
    FooterRenderer renderer;

    double cursor_x = -2.5;
    double cursor_y = -3.5;

    renderer.render(terminal_, cursor_x, cursor_y, -10.0, 10.0, -10.0, 10.0, 0);

    std::string footer = terminal_.get_row(9);

    // Should display negative cursor coordinates
    EXPECT_NE(footer.find("-2"), std::string::npos);
    EXPECT_NE(footer.find("-3"), std::string::npos);
}

// Test: Display help indicator
TEST_F(FooterRendererTest, DisplayHelpIndicator) {
    FooterRenderer renderer;

    renderer.render(terminal_, 0.0, 0.0, -1.0, 1.0, -1.0, 1.0, 0);

    std::string footer = terminal_.get_row(9);

    // Should contain help indicator (? key)
    EXPECT_NE(footer.find('?'), std::string::npos);
}
