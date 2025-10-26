#include <gtest/gtest.h>
#include "terminal.h"

using namespace datapainter;

// Test fixture for terminal tests
class TerminalTest : public ::testing::Test {
protected:
    std::unique_ptr<Terminal> term;

    void SetUp() override {
        term = std::make_unique<Terminal>();
        // Set a known size for testing
        term->set_dimensions(20, 40);
    }
};

// Test setting dimensions
TEST_F(TerminalTest, SetDimensions) {
    term->set_dimensions(25, 80);
    EXPECT_EQ(term->rows(), 25);
    EXPECT_EQ(term->cols(), 80);
}

// Test initial buffer is clear
TEST_F(TerminalTest, InitialBufferIsClear) {
    term->clear_buffer();
    for (int row = 0; row < term->rows(); ++row) {
        for (int col = 0; col < term->cols(); ++col) {
            EXPECT_EQ(term->read_char(row, col), ' ');
        }
    }
}

// Test writing and reading characters
TEST_F(TerminalTest, WriteAndReadChar) {
    term->clear_buffer();
    term->write_char(5, 10, 'X');
    EXPECT_EQ(term->read_char(5, 10), 'X');

    // Other positions should still be blank
    EXPECT_EQ(term->read_char(5, 11), ' ');
    EXPECT_EQ(term->read_char(6, 10), ' ');
}

// Test writing multiple characters
TEST_F(TerminalTest, WriteMultipleChars) {
    term->clear_buffer();
    term->write_char(0, 0, 'A');
    term->write_char(0, 1, 'B');
    term->write_char(0, 2, 'C');

    EXPECT_EQ(term->read_char(0, 0), 'A');
    EXPECT_EQ(term->read_char(0, 1), 'B');
    EXPECT_EQ(term->read_char(0, 2), 'C');
}

// Test overwriting characters
TEST_F(TerminalTest, OverwriteChar) {
    term->clear_buffer();
    term->write_char(5, 10, 'X');
    EXPECT_EQ(term->read_char(5, 10), 'X');

    term->write_char(5, 10, 'Y');
    EXPECT_EQ(term->read_char(5, 10), 'Y');
}

// Test getting entire row
TEST_F(TerminalTest, GetRow) {
    term->clear_buffer();
    term->write_char(0, 0, 'H');
    term->write_char(0, 1, 'e');
    term->write_char(0, 2, 'l');
    term->write_char(0, 3, 'l');
    term->write_char(0, 4, 'o');

    std::string row = term->get_row(0);
    EXPECT_EQ(row.substr(0, 5), "Hello");
    EXPECT_EQ(row.length(), 40);  // Full width
}

// Test clearing buffer
TEST_F(TerminalTest, ClearBuffer) {
    term->write_char(5, 10, 'X');
    term->write_char(10, 20, 'Y');

    EXPECT_EQ(term->read_char(5, 10), 'X');
    EXPECT_EQ(term->read_char(10, 20), 'Y');

    term->clear_buffer();

    EXPECT_EQ(term->read_char(5, 10), ' ');
    EXPECT_EQ(term->read_char(10, 20), ' ');
}

// Test resizing
TEST_F(TerminalTest, Resize) {
    term->clear_buffer();
    term->write_char(5, 10, 'X');

    term->set_dimensions(30, 60);

    EXPECT_EQ(term->rows(), 30);
    EXPECT_EQ(term->cols(), 60);

    // Character should still be there if within new bounds
    EXPECT_EQ(term->read_char(5, 10), 'X');
}

// Test size adequacy check (minimum size)
TEST_F(TerminalTest, SizeAdequacy) {
    // Normal size should be adequate
    term->set_dimensions(20, 40);
    EXPECT_TRUE(term->is_size_adequate());

    // Too small should not be adequate (< header + 3 rows)
    term->set_dimensions(3, 40);
    EXPECT_FALSE(term->is_size_adequate());

    // Too narrow
    term->set_dimensions(20, 10);
    EXPECT_FALSE(term->is_size_adequate());
}

// Test boundary conditions (writing at edges)
TEST_F(TerminalTest, BoundaryWrites) {
    term->clear_buffer();

    // Top-left corner
    term->write_char(0, 0, 'A');
    EXPECT_EQ(term->read_char(0, 0), 'A');

    // Top-right corner
    term->write_char(0, 39, 'B');
    EXPECT_EQ(term->read_char(0, 39), 'B');

    // Bottom-left corner
    term->write_char(19, 0, 'C');
    EXPECT_EQ(term->read_char(19, 0), 'C');

    // Bottom-right corner
    term->write_char(19, 39, 'D');
    EXPECT_EQ(term->read_char(19, 39), 'D');
}

// Test out-of-bounds handling (should not crash)
TEST_F(TerminalTest, OutOfBoundsHandling) {
    term->clear_buffer();

    // These should not crash (implementation should handle gracefully)
    // Testing that out-of-bounds reads return space
    EXPECT_EQ(term->read_char(-1, 0), ' ');
    EXPECT_EQ(term->read_char(0, -1), ' ');
    EXPECT_EQ(term->read_char(100, 0), ' ');
    EXPECT_EQ(term->read_char(0, 100), ' ');
}

// Test detect size doesn't crash
TEST_F(TerminalTest, DetectSizeDoesNotCrash) {
    // detect_size() reads actual terminal
    // Just verify it doesn't crash and returns a value
    bool result = term->detect_size();
    (void)result;  // May succeed or fail depending on environment

    // Dimensions should be set to something
    EXPECT_GT(term->rows(), 0);
    EXPECT_GT(term->cols(), 0);
}
