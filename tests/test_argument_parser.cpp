#include <gtest/gtest.h>
#include "argument_parser.h"
#include <vector>
#include <cstring>

using namespace datapainter;

// Helper to create argv from vector of strings
class ArgvHelper {
public:
    ArgvHelper(const std::vector<std::string>& args) {
        argv_ = new char*[args.size()];
        for (size_t i = 0; i < args.size(); i++) {
            argv_[i] = new char[args[i].size() + 1];
            std::strcpy(argv_[i], args[i].c_str());
        }
        argc_ = static_cast<int>(args.size());
    }

    ~ArgvHelper() {
        for (int i = 0; i < argc_; i++) {
            delete[] argv_[i];
        }
        delete[] argv_;
    }

    int argc() const { return argc_; }
    char** argv() { return argv_; }

private:
    int argc_;
    char** argv_;
};

// Test parsing --database argument
TEST(ArgumentParserTest, ParseDatabaseArgument) {
    ArgvHelper args({"datapainter", "--database", "test.db"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    ASSERT_TRUE(parsed.database.has_value());
    EXPECT_EQ(parsed.database.value(), "test.db");
}

// Test parsing --table argument
TEST(ArgumentParserTest, ParseTableArgument) {
    ArgvHelper args({"datapainter", "--table", "my_table"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    ASSERT_TRUE(parsed.table.has_value());
    EXPECT_EQ(parsed.table.value(), "my_table");
}

// Test parsing --database and --table together
TEST(ArgumentParserTest, ParseDatabaseAndTable) {
    ArgvHelper args({"datapainter", "--database", "test.db", "--table", "my_table"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    ASSERT_TRUE(parsed.database.has_value());
    EXPECT_EQ(parsed.database.value(), "test.db");
    ASSERT_TRUE(parsed.table.has_value());
    EXPECT_EQ(parsed.table.value(), "my_table");
}

// Test parsing axis names
TEST(ArgumentParserTest, ParseAxisNames) {
    ArgvHelper args({"datapainter", "--x-axis-name", "time", "--y-axis-name", "value"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    ASSERT_TRUE(parsed.x_axis_name.has_value());
    EXPECT_EQ(parsed.x_axis_name.value(), "time");
    ASSERT_TRUE(parsed.y_axis_name.has_value());
    EXPECT_EQ(parsed.y_axis_name.value(), "value");
}

// Test parsing target column name
TEST(ArgumentParserTest, ParseTargetColumnName) {
    ArgvHelper args({"datapainter", "--target-column-name", "class"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    ASSERT_TRUE(parsed.target_column_name.has_value());
    EXPECT_EQ(parsed.target_column_name.value(), "class");
}

// Test parsing meanings
TEST(ArgumentParserTest, ParseMeanings) {
    ArgvHelper args({"datapainter", "--x-meaning", "cat", "--o-meaning", "dog"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    ASSERT_TRUE(parsed.x_meaning.has_value());
    EXPECT_EQ(parsed.x_meaning.value(), "cat");
    ASSERT_TRUE(parsed.o_meaning.has_value());
    EXPECT_EQ(parsed.o_meaning.value(), "dog");
}

// Test parsing valid ranges
TEST(ArgumentParserTest, ParseValidRanges) {
    ArgvHelper args({"datapainter", "--min-x", "-10.5", "--max-x", "10.5",
                     "--min-y", "-5", "--max-y", "5"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    ASSERT_TRUE(parsed.min_x.has_value());
    EXPECT_DOUBLE_EQ(parsed.min_x.value(), -10.5);
    ASSERT_TRUE(parsed.max_x.has_value());
    EXPECT_DOUBLE_EQ(parsed.max_x.value(), 10.5);
    ASSERT_TRUE(parsed.min_y.has_value());
    EXPECT_DOUBLE_EQ(parsed.min_y.value(), -5.0);
    ASSERT_TRUE(parsed.max_y.has_value());
    EXPECT_DOUBLE_EQ(parsed.max_y.value(), 5.0);
}

// Test parsing --show-zero-bars flag
TEST(ArgumentParserTest, ParseShowZeroBars) {
    ArgvHelper args({"datapainter", "--show-zero-bars"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.show_zero_bars);
}

// Test parsing screen overrides
TEST(ArgumentParserTest, ParseScreenOverrides) {
    ArgvHelper args({"datapainter", "--override-screen-height", "50",
                     "--override-screen-width", "120"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    ASSERT_TRUE(parsed.override_screen_height.has_value());
    EXPECT_EQ(parsed.override_screen_height.value(), 50);
    ASSERT_TRUE(parsed.override_screen_width.has_value());
    EXPECT_EQ(parsed.override_screen_width.value(), 120);
}

// Test parsing --start-tabular flag
TEST(ArgumentParserTest, ParseStartTabular) {
    ArgvHelper args({"datapainter", "--start-tabular"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.start_tabular);
}

// Test parsing non-interactive commands
TEST(ArgumentParserTest, ParseCreateTable) {
    ArgvHelper args({"datapainter", "--create-table"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.create_table);
}

TEST(ArgumentParserTest, ParseListTables) {
    ArgvHelper args({"datapainter", "--list-tables"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.list_tables);
}

TEST(ArgumentParserTest, ParseToCsv) {
    ArgvHelper args({"datapainter", "--to-csv"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.to_csv);
}

// Test parsing study mode
TEST(ArgumentParserTest, ParseStudyMode) {
    ArgvHelper args({"datapainter", "--study"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.study);
}

// Test parsing random initialization
TEST(ArgumentParserTest, ParseRandomCount) {
    ArgvHelper args({"datapainter", "--random-count", "1000", "--random-target", "cat"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    ASSERT_TRUE(parsed.random_count.has_value());
    EXPECT_EQ(parsed.random_count.value(), 1000);
    ASSERT_TRUE(parsed.random_target.has_value());
    EXPECT_EQ(parsed.random_target.value(), "cat");
}

// Test validation: min must be <= max
TEST(ArgumentParserTest, ValidateMinLessThanMaxX) {
    Arguments args;
    args.min_x = 10.0;
    args.max_x = 5.0;  // Invalid: min > max

    auto errors = ArgumentParser::validate(args);
    EXPECT_FALSE(errors.empty());
    EXPECT_TRUE(std::any_of(errors.begin(), errors.end(),
                           [](const std::string& e) { return e.find("min_x") != std::string::npos; }));
}

TEST(ArgumentParserTest, ValidateMinLessThanMaxY) {
    Arguments args;
    args.min_y = 10.0;
    args.max_y = 5.0;  // Invalid: min > max

    auto errors = ArgumentParser::validate(args);
    EXPECT_FALSE(errors.empty());
    EXPECT_TRUE(std::any_of(errors.begin(), errors.end(),
                           [](const std::string& e) { return e.find("min_y") != std::string::npos; }));
}

// Test validation: valid ranges are OK
TEST(ArgumentParserTest, ValidateValidRanges) {
    Arguments args;
    args.min_x = -10.0;
    args.max_x = 10.0;
    args.min_y = -5.0;
    args.max_y = 5.0;

    auto errors = ArgumentParser::validate(args);
    EXPECT_TRUE(errors.empty());
}

// Test parsing with no arguments
TEST(ArgumentParserTest, ParseNoArguments) {
    ArgvHelper args({"datapainter"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_FALSE(parsed.database.has_value());
    EXPECT_FALSE(parsed.table.has_value());
    EXPECT_FALSE(parsed.has_errors());
}

// Test parsing invalid number for int
TEST(ArgumentParserTest, ParseInvalidInt) {
    ArgvHelper args({"datapainter", "--random-count", "not_a_number"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.has_errors());
}

// Test parsing invalid number for double
TEST(ArgumentParserTest, ParseInvalidDouble) {
    ArgvHelper args({"datapainter", "--min-x", "not_a_number"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.has_errors());
}
