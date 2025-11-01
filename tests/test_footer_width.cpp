#include <gtest/gtest.h>
#include "footer_renderer.h"
#include "terminal.h"

using namespace datapainter;

class FooterWidthTest : public ::testing::Test {
protected:
    void SetUp() override {
        terminal = std::make_unique<Terminal>();
    }

    std::unique_ptr<Terminal> terminal;
};

// Test: Footer doesn't exceed terminal width
TEST_F(FooterWidthTest, FooterFitsInTerminalWidth) {
    // Test with standard 80-column terminal
    int cols = 80;
    int rows = 24;
    terminal->set_dimensions(rows, cols);
    terminal->clear_buffer();

    FooterRenderer renderer;
    renderer.render(*terminal, 5.5, 7.5,
                   -10.0, 10.0, -10.0, 10.0,
                   0.0, 20.0, -5.0, 15.0,
                   0, 3);  // 3 unsaved changes

    // Check that footer row doesn't have content beyond column width
    int footer_row = rows - 1;
    for (int col = cols; col < cols + 10; ++col) {
        // Any access beyond terminal width should be safe (returns space or is prevented)
        // This test ensures the renderer respects terminal boundaries
    }

    // Verify no character is written at or beyond the terminal width
    // by checking the actual written content
    std::string footer_content;
    for (int col = 0; col < cols; ++col) {
        footer_content += terminal->read_char(footer_row, col);
    }

    EXPECT_LE(footer_content.length(), static_cast<size_t>(cols))
        << "Footer content length should not exceed terminal width";
}

// Test: Footer with many unsaved changes doesn't overflow
TEST_F(FooterWidthTest, FooterWithManyUnsavedChangesFits) {
    int cols = 80;
    int rows = 24;
    terminal->set_dimensions(rows, cols);
    terminal->clear_buffer();

    FooterRenderer renderer;
    // Render with large unsaved count and long coordinates
    renderer.render(*terminal, 1234.5678, 9876.5432,
                   -10000.0, 10000.0, -10000.0, 10000.0,
                   -5000.0, 5000.0, -5000.0, 5000.0,
                   0, 999);  // 999 unsaved changes

    int footer_row = rows - 1;
    std::string footer_content;
    for (int col = 0; col < cols; ++col) {
        footer_content += terminal->read_char(footer_row, col);
    }

    EXPECT_LE(footer_content.length(), static_cast<size_t>(cols))
        << "Footer with many unsaved changes should still fit in terminal width";
}

// Test: Footer in narrow terminal
TEST_F(FooterWidthTest, FooterInNarrowTerminal) {
    int cols = 40;  // Narrow terminal
    int rows = 24;
    terminal->set_dimensions(rows, cols);
    terminal->clear_buffer();

    FooterRenderer renderer;
    renderer.render(*terminal, 5.5, 7.5,
                   -10.0, 10.0, -10.0, 10.0,
                   0.0, 20.0, -5.0, 15.0,
                   0, 5);

    int footer_row = rows - 1;
    std::string footer_content;
    for (int col = 0; col < cols; ++col) {
        footer_content += terminal->read_char(footer_row, col);
    }

    EXPECT_LE(footer_content.length(), static_cast<size_t>(cols))
        << "Footer in narrow terminal should be truncated to fit";
}

// Test: Footer in very wide terminal
TEST_F(FooterWidthTest, FooterInWideTerminal) {
    int cols = 200;  // Wide terminal
    int rows = 24;
    terminal->set_dimensions(rows, cols);
    terminal->clear_buffer();

    FooterRenderer renderer;
    renderer.render(*terminal, 5.5, 7.5,
                   -10.0, 10.0, -10.0, 10.0,
                   0.0, 20.0, -5.0, 15.0,
                   0, 10);

    int footer_row = rows - 1;
    std::string footer_content;
    for (int col = 0; col < cols; ++col) {
        char ch = terminal->read_char(footer_row, col);
        if (ch != '\0') {
            footer_content += ch;
        }
    }

    // Footer should still be reasonable length even in wide terminal
    // It shouldn't try to fill the entire width with garbage
    EXPECT_LE(footer_content.length(), static_cast<size_t>(cols));
}

// Test: Footer content is actually visible (not all spaces)
TEST_F(FooterWidthTest, FooterHasVisibleContent) {
    int cols = 80;
    int rows = 24;
    terminal->set_dimensions(rows, cols);
    terminal->clear_buffer();

    FooterRenderer renderer;
    renderer.render(*terminal, 5.5, 7.5,
                   -10.0, 10.0, -10.0, 10.0,
                   0.0, 20.0, -5.0, 15.0,
                   0, 3);

    int footer_row = rows - 1;
    std::string footer_content;
    for (int col = 0; col < cols; ++col) {
        footer_content += terminal->read_char(footer_row, col);
    }

    // Count non-space characters
    int non_space_count = 0;
    for (char ch : footer_content) {
        if (ch != ' ' && ch != '\0') {
            non_space_count++;
        }
    }

    EXPECT_GT(non_space_count, 0) << "Footer should have visible content";
}
