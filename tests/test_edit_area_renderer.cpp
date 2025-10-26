#include <gtest/gtest.h>
#include "edit_area_renderer.h"
#include "terminal.h"
#include "viewport.h"
#include "data_table.h"
#include "database.h"
#include "metadata.h"
#include <cmath>

using namespace datapainter;

class EditAreaRendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        db_ = std::make_unique<Database>(":memory:");
        ASSERT_TRUE(db_->is_open());
        ASSERT_TRUE(db_->ensure_metadata_table());

        // Create the data table in the database
        mgr_ = std::make_unique<MetadataManager>(*db_);
        ASSERT_TRUE(mgr_->create_data_table("test_table"));

        table_ = std::make_unique<DataTable>(*db_, "test_table");
    }

    std::unique_ptr<Database> db_;
    std::unique_ptr<MetadataManager> mgr_;
    std::unique_ptr<DataTable> table_;
};

// Test: Draw border around edit area
TEST_F(EditAreaRendererTest, DrawBorder) {
    Terminal terminal;
    terminal.set_dimensions(10, 10);
    Viewport viewport(-5.0, 5.0, -5.0, 5.0, 8, 8);
    EditAreaRenderer renderer;

    renderer.render(terminal, viewport, *table_, {}, 0, 0, "0", "1");

    // Check corners
    EXPECT_EQ(terminal.read_char(0, 0), '+');
    EXPECT_EQ(terminal.read_char(0, 9), '+');
    EXPECT_EQ(terminal.read_char(9, 0), '+');
    EXPECT_EQ(terminal.read_char(9, 9), '+');

    // Check top and bottom edges
    for (int col = 1; col < 9; ++col) {
        EXPECT_EQ(terminal.read_char(0, col), '-');
        EXPECT_EQ(terminal.read_char(9, col), '-');
    }

    // Check left and right edges
    for (int row = 1; row < 9; ++row) {
        EXPECT_EQ(terminal.read_char(row, 0), '|');
        EXPECT_EQ(terminal.read_char(row, 9), '|');
    }
}

// Test: Render empty edit area
TEST_F(EditAreaRendererTest, RenderEmptyEditArea) {
    Terminal terminal;
    terminal.set_dimensions(10, 10);
    Viewport viewport(-5.0, 5.0, -5.0, 5.0, 8, 8);
    EditAreaRenderer renderer;

    renderer.render(terminal, viewport, *table_, {}, 0, 0, "0", "1");

    // Check interior is empty (spaces)
    for (int row = 1; row < 9; ++row) {
        for (int col = 1; col < 9; ++col) {
            EXPECT_EQ(terminal.read_char(row, col), ' ');
        }
    }
}

// Test: Render single 'x' point at screen position
TEST_F(EditAreaRendererTest, RenderSingleXPoint) {
    Terminal terminal;
    terminal.set_dimensions(10, 10);
    Viewport viewport(-4.0, 4.0, -4.0, 4.0, 8, 8);
    EditAreaRenderer renderer;

    // Insert a point at (0, 0) with target "0"
    auto id = table_->insert_point(0.0, 0.0, "0");
    ASSERT_TRUE(id.has_value()) << "Failed to insert point";

    renderer.render(terminal, viewport, *table_, {}, 0, 0, "0", "1");

    // Point (0, 0) should map to center of viewport
    DataCoord data{0.0, 0.0};
    auto screen_opt = viewport.data_to_screen(data);
    ASSERT_TRUE(screen_opt.has_value());
    auto screen = screen_opt.value();
    // Adjust for border
    EXPECT_EQ(terminal.read_char(screen.row + 1, screen.col + 1), 'x');
}

// Test: Render single 'o' point at screen position
TEST_F(EditAreaRendererTest, RenderSingleOPoint) {
    Terminal terminal;
    terminal.set_dimensions(10, 10);
    Viewport viewport(-4.0, 4.0, -4.0, 4.0, 8, 8);
    EditAreaRenderer renderer;

    // Insert a point at (0, 0) with target "1"
    table_->insert_point(0.0, 0.0, "1");

    renderer.render(terminal, viewport, *table_, {}, 0, 0, "0", "1");

    // Point (0, 0) should map to center of viewport
    DataCoord data{0.0, 0.0};
    auto screen_opt = viewport.data_to_screen(data);
    ASSERT_TRUE(screen_opt.has_value());
    auto screen = screen_opt.value();
    // Adjust for border
    EXPECT_EQ(terminal.read_char(screen.row + 1, screen.col + 1), 'o');
}

// Test: Render multiple x's at same cell as 'X'
TEST_F(EditAreaRendererTest, RenderMultipleXsAtSameCell) {
    Terminal terminal;
    terminal.set_dimensions(10, 10);
    Viewport viewport(-4.0, 4.0, -4.0, 4.0, 8, 8);
    EditAreaRenderer renderer;

    // Insert multiple points at the exact same location (target="0" = 'x')
    table_->insert_point(0.0, 0.0, "0");
    table_->insert_point(0.0, 0.0, "0");

    renderer.render(terminal, viewport, *table_, {}, 0, 0, "0", "1");

    DataCoord data{0.0, 0.0};
    auto screen_opt = viewport.data_to_screen(data);
    ASSERT_TRUE(screen_opt.has_value());
    auto screen = screen_opt.value();
    EXPECT_EQ(terminal.read_char(screen.row + 1, screen.col + 1), 'X');
}

// Test: Render multiple o's at same cell as 'O'
TEST_F(EditAreaRendererTest, RenderMultipleOsAtSameCell) {
    Terminal terminal;
    terminal.set_dimensions(10, 10);
    Viewport viewport(-4.0, 4.0, -4.0, 4.0, 8, 8);
    EditAreaRenderer renderer;

    // Insert multiple points at the exact same location (target="1" = 'o')
    table_->insert_point(0.0, 0.0, "1");
    table_->insert_point(0.0, 0.0, "1");

    renderer.render(terminal, viewport, *table_, {}, 0, 0, "0", "1");

    DataCoord data{0.0, 0.0};
    auto screen_opt = viewport.data_to_screen(data);
    ASSERT_TRUE(screen_opt.has_value());
    auto screen = screen_opt.value();
    EXPECT_EQ(terminal.read_char(screen.row + 1, screen.col + 1), 'O');
}

// Test: Render mixed x+o at same cell as '#'
TEST_F(EditAreaRendererTest, RenderMixedPointsAtSameCell) {
    Terminal terminal;
    terminal.set_dimensions(10, 10);
    Viewport viewport(-4.0, 4.0, -4.0, 4.0, 8, 8);
    EditAreaRenderer renderer;

    // Insert both x and o points at the exact same location
    table_->insert_point(0.0, 0.0, "0");  // x
    table_->insert_point(0.0, 0.0, "1");  // o

    renderer.render(terminal, viewport, *table_, {}, 0, 0, "0", "1");

    DataCoord data{0.0, 0.0};
    auto screen_opt = viewport.data_to_screen(data);
    ASSERT_TRUE(screen_opt.has_value());
    auto screen = screen_opt.value();
    EXPECT_EQ(terminal.read_char(screen.row + 1, screen.col + 1), '#');
}

// Test: Draw cursor at current position
TEST_F(EditAreaRendererTest, DrawCursor) {
    Terminal terminal;
    terminal.set_dimensions(10, 10);
    Viewport viewport(-4.0, 4.0, -4.0, 4.0, 8, 8);
    EditAreaRenderer renderer;

    // Set cursor at center of screen (row=4, col=4 in screen coords)
    renderer.render(terminal, viewport, *table_, {}, 4, 4, "0", "1");

    // Cursor should be visible at position (row=4+1, col=4+1 accounting for border)
    // The cursor is drawn after points, so if there's no point, we need a way to verify
    // For now, we'll test that rendering with a cursor doesn't crash
    // The actual cursor rendering might overlay the character
}

// Test: Query data points within current viewport
TEST_F(EditAreaRendererTest, QueryViewportPoints) {
    Terminal terminal;
    terminal.set_dimensions(10, 10);
    Viewport viewport(-2.0, 2.0, -2.0, 2.0, 8, 8);
    EditAreaRenderer renderer;

    // Insert points inside and outside viewport
    table_->insert_point(0.0, 0.0, "0");   // inside
    table_->insert_point(1.0, 1.0, "0");   // inside
    table_->insert_point(10.0, 10.0, "0"); // outside

    renderer.render(terminal, viewport, *table_, {}, 0, 0, "0", "1");

    // Verify points inside viewport are rendered
    DataCoord data1{0.0, 0.0};
    DataCoord data2{1.0, 1.0};
    auto screen_opt1 = viewport.data_to_screen(data1);
    auto screen_opt2 = viewport.data_to_screen(data2);
    ASSERT_TRUE(screen_opt1.has_value());
    ASSERT_TRUE(screen_opt2.has_value());
    auto screen1 = screen_opt1.value();
    auto screen2 = screen_opt2.value();

    EXPECT_EQ(terminal.read_char(screen1.row + 1, screen1.col + 1), 'x');
    EXPECT_EQ(terminal.read_char(screen2.row + 1, screen2.col + 1), 'x');

    // Point at (10, 10) should not be in the visible area
}

// Test: Render points at viewport boundaries
TEST_F(EditAreaRendererTest, RenderBoundaryPoints) {
    Terminal terminal;
    terminal.set_dimensions(10, 10);
    Viewport viewport(0.0, 8.0, 0.0, 8.0, 8, 8);
    EditAreaRenderer renderer;

    // Insert points at boundaries
    table_->insert_point(0.0, 0.0, "0");
    table_->insert_point(8.0, 8.0, "0");

    renderer.render(terminal, viewport, *table_, {}, 0, 0, "0", "1");

    DataCoord data1{0.0, 0.0};
    DataCoord data2{8.0, 8.0};
    auto screen_opt1 = viewport.data_to_screen(data1);
    auto screen_opt2 = viewport.data_to_screen(data2);
    ASSERT_TRUE(screen_opt1.has_value());
    ASSERT_TRUE(screen_opt2.has_value());
    auto screen1 = screen_opt1.value();
    auto screen2 = screen_opt2.value();

    EXPECT_EQ(terminal.read_char(screen1.row + 1, screen1.col + 1), 'x');
    EXPECT_EQ(terminal.read_char(screen2.row + 1, screen2.col + 1), 'x');
}
