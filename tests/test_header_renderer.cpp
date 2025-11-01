#include <gtest/gtest.h>
#include "header_renderer.h"
#include "terminal.h"
#include <string>

using namespace datapainter;

class HeaderRendererTest : public ::testing::Test {
protected:
    void SetUp() override {
        terminal_.set_dimensions(10, 80);
    }

    Terminal terminal_;
};

// Test: Display database filename
TEST_F(HeaderRendererTest, DisplayDatabaseFilename) {
    HeaderRenderer renderer;

    std::string db_path = "/path/to/mydata.db";
    std::string table_name = "test_table";
    std::string target_col = "target";
    std::string x_meaning = "positive";
    std::string o_meaning = "negative";
    int total_count = 100;
    int x_count = 60;
    int o_count = 40;

    renderer.render(terminal_, db_path, table_name, target_col,
                    x_meaning, o_meaning, total_count, x_count, o_count,
                    -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 0);

    std::string row0 = terminal_.get_row(0);
    EXPECT_NE(row0.find("mydata.db"), std::string::npos);
}

// Test: Display table name
TEST_F(HeaderRendererTest, DisplayTableName) {
    HeaderRenderer renderer;

    std::string db_path = "data.db";
    std::string table_name = "my_test_table";
    std::string target_col = "target";
    std::string x_meaning = "positive";
    std::string o_meaning = "negative";
    int total_count = 100;
    int x_count = 60;
    int o_count = 40;

    renderer.render(terminal_, db_path, table_name, target_col,
                    x_meaning, o_meaning, total_count, x_count, o_count,
                    -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 0);

    std::string row0 = terminal_.get_row(0);
    EXPECT_NE(row0.find("my_test_table"), std::string::npos);
}

// Test: Display target column name
TEST_F(HeaderRendererTest, DisplayTargetColumnName) {
    HeaderRenderer renderer;

    std::string db_path = "data.db";
    std::string table_name = "test_table";
    std::string target_col = "classification";
    std::string x_meaning = "positive";
    std::string o_meaning = "negative";
    int total_count = 100;
    int x_count = 60;
    int o_count = 40;

    renderer.render(terminal_, db_path, table_name, target_col,
                    x_meaning, o_meaning, total_count, x_count, o_count,
                    -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 0);

    // Check header rows for target column name
    std::string header;
    for (int row = 0; row < 3; ++row) {
        header += terminal_.get_row(row);
    }
    EXPECT_NE(header.find("classification"), std::string::npos);
}

// Test: Display x and o meanings
TEST_F(HeaderRendererTest, DisplayMeanings) {
    HeaderRenderer renderer;

    std::string db_path = "data.db";
    std::string table_name = "test_table";
    std::string target_col = "target";
    std::string x_meaning = "approved";
    std::string o_meaning = "rejected";
    int total_count = 100;
    int x_count = 60;
    int o_count = 40;

    renderer.render(terminal_, db_path, table_name, target_col,
                    x_meaning, o_meaning, total_count, x_count, o_count,
                    -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 0);

    // Check header rows for meanings
    std::string header;
    for (int row = 0; row < 3; ++row) {
        header += terminal_.get_row(row);
    }
    EXPECT_NE(header.find("approved"), std::string::npos);
    EXPECT_NE(header.find("rejected"), std::string::npos);
}

// Test: Display counts (total, x count, o count)
TEST_F(HeaderRendererTest, DisplayCounts) {
    HeaderRenderer renderer;

    std::string db_path = "data.db";
    std::string table_name = "test_table";
    std::string target_col = "target";
    std::string x_meaning = "positive";
    std::string o_meaning = "negative";
    int total_count = 123;
    int x_count = 78;
    int o_count = 45;

    renderer.render(terminal_, db_path, table_name, target_col,
                    x_meaning, o_meaning, total_count, x_count, o_count,
                    -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 0);

    // Check header rows for counts
    std::string header;
    for (int row = 0; row < 3; ++row) {
        header += terminal_.get_row(row);
    }
    EXPECT_NE(header.find("123"), std::string::npos);
    EXPECT_NE(header.find("78"), std::string::npos);
    EXPECT_NE(header.find("45"), std::string::npos);
}

// Test: Display valid x/y ranges
TEST_F(HeaderRendererTest, DisplayValidRanges) {
    HeaderRenderer renderer;

    std::string db_path = "data.db";
    std::string table_name = "test_table";
    std::string target_col = "target";
    std::string x_meaning = "positive";
    std::string o_meaning = "negative";
    int total_count = 100;
    int x_count = 60;
    int o_count = 40;
    double x_min = -10.5;
    double x_max = 10.5;
    double y_min = -5.0;
    double y_max = 5.0;

    renderer.render(terminal_, db_path, table_name, target_col,
                    x_meaning, o_meaning, total_count, x_count, o_count,
                    x_min, x_max, y_min, y_max,
                    x_min, x_max, y_min, y_max, 0);

    // Check header rows for range values
    std::string header;
    for (int row = 0; row < 3; ++row) {
        header += terminal_.get_row(row);
    }
    // Should contain the range values (allowing for some formatting variation)
    EXPECT_TRUE(header.find("-10.5") != std::string::npos ||
                header.find("-10") != std::string::npos);
    EXPECT_TRUE(header.find("10.5") != std::string::npos ||
                header.find("10") != std::string::npos);
    EXPECT_TRUE(header.find("-5") != std::string::npos);
}

// Test: Highlight focused field (for Tab navigation)
TEST_F(HeaderRendererTest, HighlightFocusedField) {
    HeaderRenderer renderer;

    std::string db_path = "data.db";
    std::string table_name = "test_table";
    std::string target_col = "target";
    std::string x_meaning = "positive";
    std::string o_meaning = "negative";
    int total_count = 100;
    int x_count = 60;
    int o_count = 40;

    // Render with focus on field 0 (database)
    renderer.render(terminal_, db_path, table_name, target_col,
                    x_meaning, o_meaning, total_count, x_count, o_count,
                    -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 0);

    std::string header_focused;
    for (int row = 0; row < 3; ++row) {
        header_focused += terminal_.get_row(row);
    }

    // Render with focus on field 1 (table)
    renderer.render(terminal_, db_path, table_name, target_col,
                    x_meaning, o_meaning, total_count, x_count, o_count,
                    -1.0, 1.0, -1.0, 1.0,
                    -1.0, 1.0, -1.0, 1.0, 1);

    std::string header_focused_1;
    for (int row = 0; row < 3; ++row) {
        header_focused_1 += terminal_.get_row(row);
    }

    // The two renderings should be different (one highlights database, other highlights table)
    // This is a basic check - actual implementation will use markers like [] or highlighting
    EXPECT_NE(header_focused, header_focused_1);
}

// Test: Header fits within screen width
TEST_F(HeaderRendererTest, FitsWithinScreenWidth) {
    HeaderRenderer renderer;

    std::string db_path = "data.db";
    std::string table_name = "test_table";
    std::string target_col = "target";
    std::string x_meaning = "positive";
    std::string o_meaning = "negative";
    int total_count = 100;
    int x_count = 60;
    int o_count = 40;

    renderer.render(terminal_, db_path, table_name, target_col,
                    x_meaning, o_meaning, total_count, x_count, o_count,
                    -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 0);

    // Check that no row exceeds terminal width
    for (int row = 0; row < 3; ++row) {
        std::string row_str = terminal_.get_row(row);
        EXPECT_LE(row_str.length(), static_cast<size_t>(terminal_.cols()));
    }
}

// Test: Header uses multiple rows (typically 2-3 rows)
TEST_F(HeaderRendererTest, UsesMultipleRows) {
    HeaderRenderer renderer;

    std::string db_path = "data.db";
    std::string table_name = "test_table";
    std::string target_col = "target";
    std::string x_meaning = "positive";
    std::string o_meaning = "negative";
    int total_count = 100;
    int x_count = 60;
    int o_count = 40;

    renderer.render(terminal_, db_path, table_name, target_col,
                    x_meaning, o_meaning, total_count, x_count, o_count,
                    -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 0);

    // At least one of the first 3 rows should have non-space content
    bool has_content = false;
    for (int row = 0; row < 3; ++row) {
        std::string row_str = terminal_.get_row(row);
        if (row_str.find_first_not_of(' ') != std::string::npos) {
            has_content = true;
            break;
        }
    }
    EXPECT_TRUE(has_content);
}

// Test: Display unsaved changes count when there are unsaved changes
TEST_F(HeaderRendererTest, DisplayUnsavedChangesCount) {
    HeaderRenderer renderer;

    std::string db_path = "data.db";
    std::string table_name = "test_table";
    std::string target_col = "target";
    std::string x_meaning = "positive";
    std::string o_meaning = "negative";
    int total_count = 100;
    int x_count = 60;
    int o_count = 40;
    int unsaved_count = 5;

    renderer.render(terminal_, db_path, table_name, target_col,
                    x_meaning, o_meaning, total_count, x_count, o_count,
                    -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 0, unsaved_count);

    std::string row0 = terminal_.get_row(0);
    // Should display [Unsaved: 5] in the header
    EXPECT_NE(row0.find("Unsaved"), std::string::npos) << "Header should show 'Unsaved' indicator";
    EXPECT_NE(row0.find("5"), std::string::npos) << "Header should show unsaved count";
}

// Test: Do NOT display unsaved changes when count is zero
TEST_F(HeaderRendererTest, NoUnsavedIndicatorWhenZero) {
    HeaderRenderer renderer;

    std::string db_path = "data.db";
    std::string table_name = "test_table";
    std::string target_col = "target";
    std::string x_meaning = "positive";
    std::string o_meaning = "negative";
    int total_count = 100;
    int x_count = 60;
    int o_count = 40;
    int unsaved_count = 0;

    renderer.render(terminal_, db_path, table_name, target_col,
                    x_meaning, o_meaning, total_count, x_count, o_count,
                    -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 0, unsaved_count);

    std::string row0 = terminal_.get_row(0);
    // Should NOT display [Unsaved] when count is zero
    EXPECT_EQ(row0.find("Unsaved"), std::string::npos) << "Header should not show 'Unsaved' when count is 0";
}

// Test: Unsaved changes count appears on right side of header
TEST_F(HeaderRendererTest, UnsavedCountRightAligned) {
    HeaderRenderer renderer;

    std::string db_path = "data.db";
    std::string table_name = "test_table";
    std::string target_col = "target";
    std::string x_meaning = "positive";
    std::string o_meaning = "negative";
    int total_count = 100;
    int x_count = 60;
    int o_count = 40;
    int unsaved_count = 12;

    renderer.render(terminal_, db_path, table_name, target_col,
                    x_meaning, o_meaning, total_count, x_count, o_count,
                    -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 0, unsaved_count);

    std::string row0 = terminal_.get_row(0);
    size_t unsaved_pos = row0.find("Unsaved");
    size_t table_pos = row0.find("test_table");

    // Unsaved indicator should appear after (to the right of) the table name
    ASSERT_NE(unsaved_pos, std::string::npos) << "Should find 'Unsaved' in row 0";
    ASSERT_NE(table_pos, std::string::npos) << "Should find 'test_table' in row 0";
    EXPECT_GT(unsaved_pos, table_pos) << "Unsaved indicator should be right-aligned (after table name)";
}
