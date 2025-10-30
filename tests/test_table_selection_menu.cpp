#include <gtest/gtest.h>
#include "table_selection_menu.h"
#include "terminal.h"
#include <vector>
#include <string>

using namespace datapainter;

class TableSelectionMenuTest : public ::testing::Test {
protected:
    void SetUp() override {
        terminal.set_dimensions(24, 80);
        terminal.clear_buffer();
    }

    Terminal terminal;
};

TEST_F(TableSelectionMenuTest, RenderEmptyTableList) {
    std::vector<std::string> tables;
    TableSelectionMenu menu(terminal);

    menu.render(tables, 0);

    // Check for title
    std::string row0 = terminal.get_row(0);
    EXPECT_NE(row0.find("DataPainter"), std::string::npos);

    // Check for "No tables" message when list is empty
    bool found_no_tables = false;
    for (int row = 0; row < terminal.rows(); ++row) {
        std::string line = terminal.get_row(row);
        if (line.find("No tables") != std::string::npos) {
            found_no_tables = true;
            break;
        }
    }
    EXPECT_TRUE(found_no_tables);
}

TEST_F(TableSelectionMenuTest, RenderTableList) {
    std::vector<std::string> tables = {"users", "products", "orders"};
    TableSelectionMenu menu(terminal);

    menu.render(tables, 0);

    // Check that all table names appear
    std::string full_output;
    for (int row = 0; row < terminal.rows(); ++row) {
        full_output += terminal.get_row(row);
    }

    EXPECT_NE(full_output.find("users"), std::string::npos);
    EXPECT_NE(full_output.find("products"), std::string::npos);
    EXPECT_NE(full_output.find("orders"), std::string::npos);
}

TEST_F(TableSelectionMenuTest, RenderWithSelection) {
    std::vector<std::string> tables = {"users", "products", "orders"};
    TableSelectionMenu menu(terminal);

    // Render with first item selected
    menu.render(tables, 0);

    // The selected item should be highlighted somehow (we'll check for visual indicator)
    bool found_indicator = false;
    for (int row = 0; row < terminal.rows(); ++row) {
        std::string line = terminal.get_row(row);
        // Look for selection indicator (e.g., '>', '*', or reverse video marker)
        if (line.find(">") != std::string::npos || line.find("*") != std::string::npos) {
            // Also check if "users" is on the same line or nearby
            if (line.find("users") != std::string::npos) {
                found_indicator = true;
                break;
            }
        }
    }
    EXPECT_TRUE(found_indicator);
}

TEST_F(TableSelectionMenuTest, RenderActions) {
    std::vector<std::string> tables = {"users"};
    TableSelectionMenu menu(terminal);

    menu.render(tables, 0);

    // Check for action menu items
    std::string full_output;
    for (int row = 0; row < terminal.rows(); ++row) {
        full_output += terminal.get_row(row);
    }

    // Should show options like Open, Create, Delete, etc.
    EXPECT_NE(full_output.find("Open"), std::string::npos);
}

TEST_F(TableSelectionMenuTest, RenderBorder) {
    std::vector<std::string> tables = {"users"};
    TableSelectionMenu menu(terminal);

    menu.render(tables, 0);

    // Check for border characters (or their ASCII fallbacks)
    char top_left = terminal.read_char(2, 0);  // Assuming border starts at row 2
    EXPECT_TRUE(top_left == '+' || top_left == '|' || top_left == '-');
}

TEST_F(TableSelectionMenuTest, CalculateSelectionBounds) {
    std::vector<std::string> tables = {"users", "products", "orders"};
    TableSelectionMenu menu(terminal);

    // With 3 tables, we have 3 + 5 actions = 8 total items (indices 0-7)
    // Test selection wrapping
    EXPECT_EQ(menu.normalize_selection(-1, tables), 7);  // Wrap to last item
    EXPECT_EQ(menu.normalize_selection(8, tables), 0);   // Wrap to first item
    EXPECT_EQ(menu.normalize_selection(0, tables), 0);   // Stay in bounds
    EXPECT_EQ(menu.normalize_selection(1, tables), 1);   // Stay in bounds
    EXPECT_EQ(menu.normalize_selection(7, tables), 7);   // Last item
}

TEST_F(TableSelectionMenuTest, EmptyTableListSelection) {
    std::vector<std::string> tables;
    TableSelectionMenu menu(terminal);

    // With no tables, we have 2 actions: Create and Exit (indices 0-1)
    EXPECT_EQ(menu.normalize_selection(0, tables), 0);   // First action
    EXPECT_EQ(menu.normalize_selection(1, tables), 1);   // Second action
    EXPECT_EQ(menu.normalize_selection(-1, tables), 1);  // Wrap to last action
    EXPECT_EQ(menu.normalize_selection(2, tables), 0);   // Wrap to first action
}

TEST_F(TableSelectionMenuTest, ResizeUpdatesDisplay) {
    std::vector<std::string> tables = {"users", "products"};
    TableSelectionMenu menu(terminal);

    // Initial render at 24x80
    menu.render(tables, 0);
    EXPECT_EQ(terminal.rows(), 24);
    EXPECT_EQ(terminal.cols(), 80);

    // Simulate terminal resize to 30x100
    terminal.set_dimensions(30, 100);

    // Re-render after resize
    menu.render(tables, 0);

    // Verify the buffer has been resized
    EXPECT_EQ(terminal.rows(), 30);
    EXPECT_EQ(terminal.cols(), 100);

    // Verify content still renders correctly (title should still be present)
    std::string row0 = terminal.get_row(0);
    EXPECT_NE(row0.find("DataPainter"), std::string::npos);

    // Verify tables still appear
    std::string full_output;
    for (int row = 0; row < terminal.rows(); ++row) {
        full_output += terminal.get_row(row);
    }
    EXPECT_NE(full_output.find("users"), std::string::npos);
    EXPECT_NE(full_output.find("products"), std::string::npos);
}

TEST_F(TableSelectionMenuTest, ShowEnlargeTerminalMessage) {
    std::vector<std::string> tables = {"users", "products"};
    TableSelectionMenu menu(terminal);

    // Set terminal to too small (< 5 rows or < 40 cols)
    terminal.set_dimensions(4, 30);

    // Render with too-small terminal
    menu.render(tables, 0);

    // Should show "enlarge terminal" message
    std::string full_output;
    for (int row = 0; row < terminal.rows(); ++row) {
        full_output += terminal.get_row(row);
    }

    EXPECT_NE(full_output.find("enlarge"), std::string::npos);
}

TEST_F(TableSelectionMenuTest, ResumeRenderingWhenSizeAdequate) {
    std::vector<std::string> tables = {"users", "products"};
    TableSelectionMenu menu(terminal);

    // Start with too-small terminal
    terminal.set_dimensions(4, 30);
    menu.render(tables, 0);

    std::string small_output;
    for (int row = 0; row < terminal.rows(); ++row) {
        small_output += terminal.get_row(row);
    }
    EXPECT_NE(small_output.find("enlarge"), std::string::npos);

    // Resize to adequate size
    terminal.set_dimensions(24, 80);
    menu.render(tables, 0);

    // Should show normal menu content, not "enlarge" message
    std::string normal_output;
    for (int row = 0; row < terminal.rows(); ++row) {
        normal_output += terminal.get_row(row);
    }

    EXPECT_EQ(normal_output.find("enlarge"), std::string::npos);
    EXPECT_NE(normal_output.find("users"), std::string::npos);
}
