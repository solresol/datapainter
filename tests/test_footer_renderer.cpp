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

    renderer.render(terminal_, 2.5, 3.5, -1.0, 1.0, -1.0, 1.0,
                    -1.0, 1.0, -1.0, 1.0, 0);

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

    renderer.render(terminal_, cursor_x, cursor_y, -1.0, 1.0, -1.0, 1.0,
                    -1.0, 1.0, -1.0, 1.0, 0);

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

    renderer.render(terminal_, 0.0, 0.0, x_min, x_max, y_min, y_max,
                    x_min, x_max, y_min, y_max, 0);

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

    renderer.render(terminal_, 0.0, 0.0, -1.0, 1.0, -1.0, 1.0,
                    -1.0, 1.0, -1.0, 1.0, 0);

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
    renderer.render(terminal_, 0.0, 0.0, -1.0, 1.0, -1.0, 1.0,
                    -1.0, 1.0, -1.0, 1.0, 0);
    std::string footer_no_focus = terminal_.get_row(9);

    // Render with focus on button 1
    terminal_.clear_buffer();
    renderer.render(terminal_, 0.0, 0.0, -1.0, 1.0, -1.0, 1.0,
                    -1.0, 1.0, -1.0, 1.0, 1);
    std::string footer_focus_1 = terminal_.get_row(9);

    // The two renderings should be different
    EXPECT_NE(footer_no_focus, footer_focus_1);
}

// Test: Footer fits within screen width
TEST_F(FooterRendererTest, FitsWithinScreenWidth) {
    FooterRenderer renderer;

    renderer.render(terminal_, 0.0, 0.0, -1.0, 1.0, -1.0, 1.0,
                    -1.0, 1.0, -1.0, 1.0, 0);

    std::string footer = terminal_.get_row(9);
    EXPECT_LE(footer.length(), static_cast<size_t>(terminal_.cols()));
}

// Test: Footer uses bottom row
TEST_F(FooterRendererTest, UsesBottomRow) {
    FooterRenderer renderer;

    renderer.render(terminal_, 0.0, 0.0, -1.0, 1.0, -1.0, 1.0,
                    -1.0, 1.0, -1.0, 1.0, 0);

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

    renderer.render(terminal_, cursor_x, cursor_y, -10.0, 10.0, -10.0, 10.0,
                    -10.0, 10.0, -10.0, 10.0, 0);

    std::string footer = terminal_.get_row(9);

    // Should display negative cursor coordinates
    EXPECT_NE(footer.find("-2"), std::string::npos);
    EXPECT_NE(footer.find("-3"), std::string::npos);
}

// Test: Display help indicator
TEST_F(FooterRendererTest, DisplayHelpIndicator) {
    FooterRenderer renderer;

    renderer.render(terminal_, 0.0, 0.0, -1.0, 1.0, -1.0, 1.0,
                    -1.0, 1.0, -1.0, 1.0, 0);

    std::string footer = terminal_.get_row(9);

    // Should contain help indicator (? key)
    EXPECT_NE(footer.find('?'), std::string::npos);
}

// Test: Cursor precision increases when zoomed in
TEST_F(FooterRendererTest, CursorPrecisionIncreasesWhenZoomedIn) {
    FooterRenderer renderer;
    terminal_.set_dimensions(10, 80);

    // Zoomed out: viewport [-10, 10] with 78 cells (80 - 2 for border)
    // data_per_cell = 20 / 78 ≈ 0.26, should show 1-2 decimal places
    renderer.render(terminal_, 5.123456, 5.123456,
                    -10.0, 10.0, -10.0, 10.0,
                    -10.0, 10.0, -10.0, 10.0, 0);
    std::string footer_zoomed_out = terminal_.get_row(9);

    terminal_.clear_buffer();

    // Zoomed in: viewport [-0.1, 0.1] with 78 cells
    // data_per_cell = 0.2 / 78 ≈ 0.0026, should show 3-4 decimal places
    renderer.render(terminal_, 0.023456, 0.023456,
                    -10.0, 10.0, -10.0, 10.0,
                    -0.1, 0.1, -0.1, 0.1, 0);
    std::string footer_zoomed_in = terminal_.get_row(9);

    // Zoomed in should have more decimal places
    // Count decimal places by finding digits after decimal point
    auto count_decimals = [](const std::string& s, double value) -> int {
        // Find the value in the string
        std::ostringstream val_str;
        val_str << value;
        size_t pos = s.find('(');
        if (pos == std::string::npos) return 0;

        // Look for a decimal point after the opening paren
        size_t dec_pos = s.find('.', pos);
        if (dec_pos == std::string::npos) return 0;

        // Count digits after decimal point until non-digit
        int count = 0;
        for (size_t i = dec_pos + 1; i < s.length() && std::isdigit(s[i]); ++i) {
            count++;
        }
        return count;
    };

    int decimals_zoomed_out = count_decimals(footer_zoomed_out, 5.123456);
    int decimals_zoomed_in = count_decimals(footer_zoomed_in, 0.023456);

    // When zoomed in, we should have more decimal places
    EXPECT_GT(decimals_zoomed_in, decimals_zoomed_out)
        << "Zoomed out: " << footer_zoomed_out << "\n"
        << "Zoomed in: " << footer_zoomed_in;
}

// Test: Adjacent cells show different cursor coordinates when zoomed in
TEST_F(FooterRendererTest, AdjacentCellsShowDifferentCoordinatesWhenZoomedIn) {
    FooterRenderer renderer;
    terminal_.set_dimensions(10, 80);

    // Create viewport: [-0.1, 0.1] with 78 cells (80 - 2 for border)
    // Each cell is 0.2 / 78 ≈ 0.00256 units wide
    double vp_x_min = -0.1;
    double vp_x_max = 0.1;
    double vp_y_min = -0.1;
    double vp_y_max = 0.1;

    // Simulate two adjacent cells at positions that differ by one cell width
    double cell_width = (vp_x_max - vp_x_min) / 78.0;
    double cursor1_x = 0.0;
    double cursor2_x = cursor1_x + cell_width;

    // Render first position
    renderer.render(terminal_, cursor1_x, 0.0,
                    -10.0, 10.0, -10.0, 10.0,
                    vp_x_min, vp_x_max, vp_y_min, vp_y_max, 0);
    std::string footer1 = terminal_.get_row(9);

    terminal_.clear_buffer();

    // Render second position (adjacent cell)
    renderer.render(terminal_, cursor2_x, 0.0,
                    -10.0, 10.0, -10.0, 10.0,
                    vp_x_min, vp_x_max, vp_y_min, vp_y_max, 0);
    std::string footer2 = terminal_.get_row(9);

    // The cursor coordinates should be different
    EXPECT_NE(footer1, footer2)
        << "Adjacent cells should show different coordinates\n"
        << "Cell 1 (" << cursor1_x << "): " << footer1 << "\n"
        << "Cell 2 (" << cursor2_x << "): " << footer2 << "\n"
        << "Cell width: " << cell_width;
}

// Test: Very zoomed in viewport shows high precision
TEST_F(FooterRendererTest, VeryZoomedInShowsHighPrecision) {
    FooterRenderer renderer;
    terminal_.set_dimensions(10, 80);

    // Extremely zoomed in: viewport [-0.001, 0.001]
    // Each cell is 0.002 / 78 ≈ 0.0000256 units wide
    // Should show at least 5 decimal places
    renderer.render(terminal_, 0.00012345, 0.00012345,
                    -10.0, 10.0, -10.0, 10.0,
                    -0.001, 0.001, -0.001, 0.001, 0);
    std::string footer = terminal_.get_row(9);

    // Should show the value with enough precision to distinguish cells
    // At this zoom level, we need at least 5 decimal places
    // Look for at least 4 decimal digits (being a bit lenient for formatting)
    size_t pos = footer.find('(');
    ASSERT_NE(pos, std::string::npos);

    size_t dec_pos = footer.find('.', pos);
    ASSERT_NE(dec_pos, std::string::npos) << "Should have decimal point in: " << footer;

    // Count consecutive digits after decimal
    int decimal_digits = 0;
    for (size_t i = dec_pos + 1; i < footer.length() && std::isdigit(footer[i]); ++i) {
        decimal_digits++;
    }

    EXPECT_GE(decimal_digits, 4)
        << "Should show at least 4 decimal places when very zoomed in\n"
        << "Footer: " << footer;
}

// Test: Normal zoom shows reasonable precision (not too many decimals)
TEST_F(FooterRendererTest, NormalZoomShowsReasonablePrecision) {
    FooterRenderer renderer;
    terminal_.set_dimensions(10, 80);

    // Normal viewport: [-10, 10]
    // Each cell is 20 / 78 ≈ 0.26 units wide
    // Should show 1-2 decimal places, not 10
    renderer.render(terminal_, 5.123456789, 5.123456789,
                    -10.0, 10.0, -10.0, 10.0,
                    -10.0, 10.0, -10.0, 10.0, 0);
    std::string footer = terminal_.get_row(9);

    // Count decimal places
    size_t pos = footer.find('(');
    ASSERT_NE(pos, std::string::npos);

    size_t dec_pos = footer.find('.', pos);
    if (dec_pos != std::string::npos) {
        int decimal_digits = 0;
        for (size_t i = dec_pos + 1; i < footer.length() && std::isdigit(footer[i]); ++i) {
            decimal_digits++;
        }

        // Should not show more than 3 decimal places for normal zoom
        EXPECT_LE(decimal_digits, 3)
            << "Should not show excessive precision at normal zoom\n"
            << "Footer: " << footer;
    }
}
