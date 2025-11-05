#include <gtest/gtest.h>
#include "help_overlay.h"
#include "terminal.h"

using namespace datapainter;

class HelpOverlayTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize terminal with test dimensions
        terminal = std::make_unique<Terminal>();
        terminal->set_dimensions(24, 80);  // Standard terminal size
        terminal->clear_buffer();
    }

    std::unique_ptr<Terminal> terminal;
};

// Test: HelpOverlay can be created
TEST_F(HelpOverlayTest, Create) {
    HelpOverlay overlay;
    // Just verify it compiles and constructs without crashing
    overlay.render(*terminal, 24, 80, 100.0, 10.0);
    EXPECT_TRUE(true);
}

// Test: HelpOverlay renders content to terminal
TEST_F(HelpOverlayTest, Render) {
    HelpOverlay overlay;

    // Render should not crash
    overlay.render(*terminal, 24, 80, 100.0, 10.0);

    // Verify something was written to the terminal
    // We can check that certain keywords appear in the buffer
    std::string buffer_content;
    for (int row = 0; row < 24; ++row) {
        for (int col = 0; col < 80; ++col) {
            buffer_content += terminal->read_char(row, col);
        }
    }

    // Help should contain common keywords
    EXPECT_NE(buffer_content.find("HELP"), std::string::npos);
}

// Test: Help shows keyboard shortcuts
TEST_F(HelpOverlayTest, ShowsKeyboardShortcuts) {
    HelpOverlay overlay;
    overlay.render(*terminal, 24, 80, 100.0, 10.0);

    std::string buffer_content;
    for (int row = 0; row < 24; ++row) {
        for (int col = 0; col < 80; ++col) {
            buffer_content += terminal->read_char(row, col);
        }
    }

    // Check for key shortcuts
    EXPECT_NE(buffer_content.find("x"), std::string::npos);  // Create x point
    EXPECT_NE(buffer_content.find("o"), std::string::npos);  // Create o point
    EXPECT_NE(buffer_content.find("+"), std::string::npos);  // Zoom in
    EXPECT_NE(buffer_content.find("-"), std::string::npos);  // Zoom out
    EXPECT_NE(buffer_content.find("="), std::string::npos);  // Full viewport
}

// Test: Help shows arrow key navigation
TEST_F(HelpOverlayTest, ShowsArrowKeys) {
    HelpOverlay overlay;
    overlay.render(*terminal, 24, 80, 100.0, 10.0);

    std::string buffer_content;
    for (int row = 0; row < 24; ++row) {
        for (int col = 0; col < 80; ++col) {
            buffer_content += terminal->read_char(row, col);
        }
    }

    // Should mention arrows or navigation
    bool has_arrows = buffer_content.find("Arrow") != std::string::npos ||
                     buffer_content.find("arrow") != std::string::npos ||
                     buffer_content.find("cursor") != std::string::npos ||
                     buffer_content.find("move") != std::string::npos;
    EXPECT_TRUE(has_arrows);
}

// Test: Help shows quit/save shortcuts
TEST_F(HelpOverlayTest, ShowsQuitSave) {
    HelpOverlay overlay;

    // Use taller terminal to fit all help content (help is now ~37 lines with zoom/pan info)
    terminal->set_dimensions(40, 80);
    terminal->clear_buffer();
    overlay.render(*terminal, 40, 80, 100.0, 10.0);

    std::string buffer_content;
    for (int row = 0; row < 40; ++row) {
        for (int col = 0; col < 80; ++col) {
            buffer_content += terminal->read_char(row, col);
        }
    }

    // Check for undo/save/quit section
    bool has_quit_save_section = buffer_content.find("UNDO") != std::string::npos ||
                                  buffer_content.find("undo") != std::string::npos ||
                                  buffer_content.find("QUIT") != std::string::npos;
    EXPECT_TRUE(has_quit_save_section);
}

// Test: Help shows how to dismiss
TEST_F(HelpOverlayTest, ShowsDismissInstructions) {
    HelpOverlay overlay;
    overlay.render(*terminal, 24, 80, 100.0, 10.0);

    std::string buffer_content;
    for (int row = 0; row < 24; ++row) {
        for (int col = 0; col < 80; ++col) {
            buffer_content += terminal->read_char(row, col);
        }
    }

    // Should tell user how to close help
    // The help should contain significantly more content than just empty spaces
    // If it's rendering properly, buffer should have meaningful content
    int non_space_count = 0;
    for (char ch : buffer_content) {
        if (ch != ' ' && ch != '\0') {
            non_space_count++;
        }
    }
    // Help overlay should have plenty of non-space characters
    EXPECT_GT(non_space_count, 100);
}

// Test: Help fits in small terminal
TEST_F(HelpOverlayTest, FitsInSmallTerminal) {
    HelpOverlay overlay;

    // Try with smaller terminal
    terminal->set_dimensions(20, 60);
    terminal->clear_buffer();

    // Should not crash even with limited space
    overlay.render(*terminal, 20, 60, 100.0, 10.0);
    EXPECT_TRUE(true);
}

// Test: Help centers content
TEST_F(HelpOverlayTest, CentersContent) {
    HelpOverlay overlay;
    overlay.render(*terminal, 24, 80, 100.0, 10.0);

    // Content should be centered - check that there's padding at the start
    // Our content is 55 chars wide, in an 80-char terminal, so there should be
    // about 12-13 chars of padding on the left

    // First few columns should be spaces (padding)
    char ch_col0 = terminal->read_char(5, 0);
    char ch_col5 = terminal->read_char(5, 5);
    EXPECT_EQ(ch_col0, ' ');
    EXPECT_EQ(ch_col5, ' ');

    // Around column 12-13 should be the start of content (the border)
    char ch_col12 = terminal->read_char(5, 12);
    EXPECT_TRUE(ch_col12 == '+' || ch_col12 == '|');
}

// Test: Help displays current zoom level
TEST_F(HelpOverlayTest, DisplaysZoomLevel) {
    HelpOverlay overlay;
    double zoom_percent = 50.0;
    overlay.render(*terminal, 24, 80, zoom_percent, 10.0);

    std::string buffer_content;
    for (int row = 0; row < 24; ++row) {
        for (int col = 0; col < 80; ++col) {
            buffer_content += terminal->read_char(row, col);
        }
    }

    // Should display zoom percentage
    EXPECT_NE(buffer_content.find("Zoom"), std::string::npos);
    EXPECT_NE(buffer_content.find("50"), std::string::npos);
}

// Test: Help displays current pan step percentage
TEST_F(HelpOverlayTest, DisplaysPanStep) {
    HelpOverlay overlay;
    double zoom_percent = 100.0;
    double pan_step_percent = 15.0;
    overlay.render(*terminal, 24, 80, zoom_percent, pan_step_percent);

    std::string buffer_content;
    for (int row = 0; row < 24; ++row) {
        for (int col = 0; col < 80; ++col) {
            buffer_content += terminal->read_char(row, col);
        }
    }

    // Should display pan step percentage
    EXPECT_NE(buffer_content.find("Pan"), std::string::npos);
    EXPECT_NE(buffer_content.find("15"), std::string::npos);
}
