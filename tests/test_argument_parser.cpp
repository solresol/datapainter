#include <gtest/gtest.h>
#include "argument_parser.h"
#include "metadata.h"
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

// Test parsing --help flag
TEST(ArgumentParserTest, ParseHelpFlag) {
    ArgvHelper args({"datapainter", "--help"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.show_help);
}

// Test parsing -h shorthand
TEST(ArgumentParserTest, ParseHelpShorthand) {
    ArgvHelper args({"datapainter", "-h"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.show_help);
}

// Test parsing --version flag
TEST(ArgumentParserTest, ParseVersionFlag) {
    ArgvHelper args({"datapainter", "--version"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.show_version);
}

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

// Test parsing --rename-table flag
TEST(ArgumentParserTest, ParseRenameTable) {
    ArgvHelper args({"datapainter", "--rename-table"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.rename_table);
}

// Test parsing --copy-table flag
TEST(ArgumentParserTest, ParseCopyTable) {
    ArgvHelper args({"datapainter", "--copy-table"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.copy_table);
}

// Test parsing --delete-table flag
TEST(ArgumentParserTest, ParseDeleteTable) {
    ArgvHelper args({"datapainter", "--delete-table"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.delete_table);
}

// Test parsing --show-metadata flag
TEST(ArgumentParserTest, ParseShowMetadata) {
    ArgvHelper args({"datapainter", "--show-metadata"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.show_metadata);
}

// Test parsing --add-point flag
TEST(ArgumentParserTest, ParseAddPoint) {
    ArgvHelper args({"datapainter", "--add-point", "--x", "1.5", "--y", "2.3", "--target", "positive"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.add_point);
    ASSERT_TRUE(parsed.point_x.has_value());
    EXPECT_DOUBLE_EQ(parsed.point_x.value(), 1.5);
    ASSERT_TRUE(parsed.point_y.has_value());
    EXPECT_DOUBLE_EQ(parsed.point_y.value(), 2.3);
    ASSERT_TRUE(parsed.point_target.has_value());
    EXPECT_EQ(parsed.point_target.value(), "positive");
}

// Test parsing --delete-point flag
TEST(ArgumentParserTest, ParseDeletePoint) {
    ArgvHelper args({"datapainter", "--delete-point", "--point-id", "42"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.delete_point);
    ASSERT_TRUE(parsed.point_id.has_value());
    EXPECT_EQ(parsed.point_id.value(), 42);
}

// Test parsing --key-stroke-at-point
TEST(ArgumentParserTest, ParseKeyStrokeAtPoint) {
    ArgvHelper args({"datapainter", "--key-stroke-at-point", "1.5,2.3,x"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    ASSERT_TRUE(parsed.key_stroke_at_point.has_value());
    EXPECT_EQ(parsed.key_stroke_at_point.value(), "1.5,2.3,x");
}

// Test parsing --dump-screen flag
TEST(ArgumentParserTest, ParseDumpScreen) {
    ArgvHelper args({"datapainter", "--dump-screen"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.dump_screen);
}

// Test parsing --dump-edit-area-contents flag
TEST(ArgumentParserTest, ParseDumpEditAreaContents) {
    ArgvHelper args({"datapainter", "--dump-edit-area-contents"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.dump_edit_area_contents);
}

// Test parsing --zoom-in flag
TEST(ArgumentParserTest, ParseZoomIn) {
    ArgvHelper args({"datapainter", "--zoom-in"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.zoom_in);
}

// Test parsing --zoom-out flag
TEST(ArgumentParserTest, ParseZoomOut) {
    ArgvHelper args({"datapainter", "--zoom-out"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.zoom_out);
}

// Test parsing --list-x-axis-marks flag
TEST(ArgumentParserTest, ParseListXAxisMarks) {
    ArgvHelper args({"datapainter", "--list-x-axis-marks"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.list_x_axis_marks);
}

// Test parsing --list-y-axis-marks flag
TEST(ArgumentParserTest, ParseListYAxisMarks) {
    ArgvHelper args({"datapainter", "--list-y-axis-marks"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.list_y_axis_marks);
}

// Test parsing --random-target
TEST(ArgumentParserTest, ParseRandomTarget) {
    ArgvHelper args({"datapainter", "--random-target", "cat"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    ASSERT_TRUE(parsed.random_target.has_value());
    EXPECT_EQ(parsed.random_target.value(), "cat");
}

// Test parsing --mean-x and --mean-y
TEST(ArgumentParserTest, ParseMeanXY) {
    ArgvHelper args({"datapainter", "--mean-x", "3.5", "--mean-y", "4.2"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    ASSERT_TRUE(parsed.mean_x.has_value());
    EXPECT_DOUBLE_EQ(parsed.mean_x.value(), 3.5);
    ASSERT_TRUE(parsed.mean_y.has_value());
    EXPECT_DOUBLE_EQ(parsed.mean_y.value(), 4.2);
}

// Test parsing --normal-x and --normal-y
TEST(ArgumentParserTest, ParseNormalXY) {
    ArgvHelper args({"datapainter", "--normal-x", "--normal-y", "--std-x", "1.5", "--std-y", "2.0"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.normal_x);
    EXPECT_TRUE(parsed.normal_y);
    ASSERT_TRUE(parsed.std_x.has_value());
    EXPECT_DOUBLE_EQ(parsed.std_x.value(), 1.5);
    ASSERT_TRUE(parsed.std_y.has_value());
    EXPECT_DOUBLE_EQ(parsed.std_y.value(), 2.0);
}

// Test parsing --uniform-x and --uniform-y
TEST(ArgumentParserTest, ParseUniformXY) {
    ArgvHelper args({"datapainter", "--uniform-x", "--uniform-y", "--range-x", "10.0", "--range-y", "20.0"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.uniform_x);
    EXPECT_TRUE(parsed.uniform_y);
    ASSERT_TRUE(parsed.range_x.has_value());
    EXPECT_DOUBLE_EQ(parsed.range_x.value(), 10.0);
    ASSERT_TRUE(parsed.range_y.has_value());
    EXPECT_DOUBLE_EQ(parsed.range_y.value(), 20.0);
}

// Test parsing --clear-undo-log flag
TEST(ArgumentParserTest, ParseClearUndoLog) {
    ArgvHelper args({"datapainter", "--clear-undo-log"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.clear_undo_log);
}

// Test parsing --clear-all-undo-log flag
TEST(ArgumentParserTest, ParseClearAllUndoLog) {
    ArgvHelper args({"datapainter", "--clear-all-undo-log"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.clear_all_undo_log);
}

// Test parsing --commit-unsaved-changes flag
TEST(ArgumentParserTest, ParseCommitUnsavedChanges) {
    ArgvHelper args({"datapainter", "--commit-unsaved-changes"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.commit_unsaved_changes);
}

// Test parsing --list-unsaved-changes flag
TEST(ArgumentParserTest, ParseListUnsavedChanges) {
    ArgvHelper args({"datapainter", "--list-unsaved-changes"});
    auto parsed = ArgumentParser::parse(args.argc(), args.argv());

    EXPECT_TRUE(parsed.list_unsaved_changes);
}

// ========== Conflict Detection Tests ==========

// Test conflict detection: x-axis name mismatch
TEST(ArgumentParserTest, DetectXAxisNameConflict) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "time";
    meta.y_axis_name = "value";
    meta.target_col_name = "label";
    meta.x_meaning = "positive";
    meta.o_meaning = "negative";
    meta.valid_x_min = -10.0;
    meta.valid_x_max = 10.0;
    meta.valid_y_min = -10.0;
    meta.valid_y_max = 10.0;
    meta.show_zero_bars = false;

    Arguments args;
    args.x_axis_name = "frequency";  // Conflict!

    auto conflicts = ArgumentParser::detect_conflicts(args, meta);
    EXPECT_FALSE(conflicts.empty());
    EXPECT_TRUE(std::any_of(conflicts.begin(), conflicts.end(),
                           [](const std::string& msg) { return msg.find("--x-axis-name") != std::string::npos; }));
}

// Test conflict detection: y-axis name mismatch
TEST(ArgumentParserTest, DetectYAxisNameConflict) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "time";
    meta.y_axis_name = "value";
    meta.target_col_name = "label";
    meta.x_meaning = "positive";
    meta.o_meaning = "negative";

    Arguments args;
    args.y_axis_name = "amplitude";  // Conflict!

    auto conflicts = ArgumentParser::detect_conflicts(args, meta);
    EXPECT_FALSE(conflicts.empty());
    EXPECT_TRUE(std::any_of(conflicts.begin(), conflicts.end(),
                           [](const std::string& msg) { return msg.find("--y-axis-name") != std::string::npos; }));
}

// Test conflict detection: target column name mismatch
TEST(ArgumentParserTest, DetectTargetColumnNameConflict) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "time";
    meta.y_axis_name = "value";
    meta.target_col_name = "label";
    meta.x_meaning = "positive";
    meta.o_meaning = "negative";

    Arguments args;
    args.target_column_name = "class";  // Conflict!

    auto conflicts = ArgumentParser::detect_conflicts(args, meta);
    EXPECT_FALSE(conflicts.empty());
    EXPECT_TRUE(std::any_of(conflicts.begin(), conflicts.end(),
                           [](const std::string& msg) { return msg.find("--target-column-name") != std::string::npos; }));
}

// Test conflict detection: x-meaning mismatch
TEST(ArgumentParserTest, DetectXMeaningConflict) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "time";
    meta.y_axis_name = "value";
    meta.target_col_name = "label";
    meta.x_meaning = "positive";
    meta.o_meaning = "negative";

    Arguments args;
    args.x_meaning = "cat";  // Conflict!

    auto conflicts = ArgumentParser::detect_conflicts(args, meta);
    EXPECT_FALSE(conflicts.empty());
    EXPECT_TRUE(std::any_of(conflicts.begin(), conflicts.end(),
                           [](const std::string& msg) { return msg.find("--x-meaning") != std::string::npos; }));
}

// Test conflict detection: o-meaning mismatch
TEST(ArgumentParserTest, DetectOMeaningConflict) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "time";
    meta.y_axis_name = "value";
    meta.target_col_name = "label";
    meta.x_meaning = "positive";
    meta.o_meaning = "negative";

    Arguments args;
    args.o_meaning = "dog";  // Conflict!

    auto conflicts = ArgumentParser::detect_conflicts(args, meta);
    EXPECT_FALSE(conflicts.empty());
    EXPECT_TRUE(std::any_of(conflicts.begin(), conflicts.end(),
                           [](const std::string& msg) { return msg.find("--o-meaning") != std::string::npos; }));
}

// Test conflict detection: min-x mismatch
TEST(ArgumentParserTest, DetectMinXConflict) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "time";
    meta.y_axis_name = "value";
    meta.target_col_name = "label";
    meta.x_meaning = "positive";
    meta.o_meaning = "negative";
    meta.valid_x_min = -10.0;
    meta.valid_x_max = 10.0;
    meta.valid_y_min = -10.0;
    meta.valid_y_max = 10.0;

    Arguments args;
    args.min_x = -5.0;  // Conflict!

    auto conflicts = ArgumentParser::detect_conflicts(args, meta);
    EXPECT_FALSE(conflicts.empty());
    EXPECT_TRUE(std::any_of(conflicts.begin(), conflicts.end(),
                           [](const std::string& msg) { return msg.find("--min-x") != std::string::npos; }));
}

// Test conflict detection: max-y mismatch
TEST(ArgumentParserTest, DetectMaxYConflict) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "time";
    meta.y_axis_name = "value";
    meta.target_col_name = "label";
    meta.x_meaning = "positive";
    meta.o_meaning = "negative";
    meta.valid_x_min = -10.0;
    meta.valid_x_max = 10.0;
    meta.valid_y_min = -10.0;
    meta.valid_y_max = 10.0;

    Arguments args;
    args.max_y = 20.0;  // Conflict!

    auto conflicts = ArgumentParser::detect_conflicts(args, meta);
    EXPECT_FALSE(conflicts.empty());
    EXPECT_TRUE(std::any_of(conflicts.begin(), conflicts.end(),
                           [](const std::string& msg) { return msg.find("--max-y") != std::string::npos; }));
}

// Test conflict detection: no conflicts when args match metadata
TEST(ArgumentParserTest, DetectNoConflictsWhenMatching) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "time";
    meta.y_axis_name = "value";
    meta.target_col_name = "label";
    meta.x_meaning = "positive";
    meta.o_meaning = "negative";
    meta.valid_x_min = -10.0;
    meta.valid_x_max = 10.0;
    meta.valid_y_min = -10.0;
    meta.valid_y_max = 10.0;
    meta.show_zero_bars = false;

    Arguments args;
    args.x_axis_name = "time";  // Matches!
    args.y_axis_name = "value";  // Matches!
    args.min_x = -10.0;  // Matches!

    auto conflicts = ArgumentParser::detect_conflicts(args, meta);
    EXPECT_TRUE(conflicts.empty());
}

// Test conflict detection: no conflicts when no args provided
TEST(ArgumentParserTest, DetectNoConflictsWhenNoArgs) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "time";
    meta.y_axis_name = "value";
    meta.target_col_name = "label";
    meta.x_meaning = "positive";
    meta.o_meaning = "negative";

    Arguments args;
    // No conflicting arguments provided

    auto conflicts = ArgumentParser::detect_conflicts(args, meta);
    EXPECT_TRUE(conflicts.empty());
}

// Test conflict detection: generate clear error message
TEST(ArgumentParserTest, ConflictMessageFormat) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "time";
    meta.y_axis_name = "value";
    meta.target_col_name = "label";
    meta.x_meaning = "positive";
    meta.o_meaning = "negative";

    Arguments args;
    args.x_meaning = "cat";  // Conflict!

    auto conflicts = ArgumentParser::detect_conflicts(args, meta);
    ASSERT_FALSE(conflicts.empty());

    // Verify message contains key elements
    const std::string& msg = conflicts[0];
    EXPECT_TRUE(msg.find("Conflict detected") != std::string::npos);
    EXPECT_TRUE(msg.find("CLI argument") != std::string::npos);
    EXPECT_TRUE(msg.find("Existing metadata") != std::string::npos);
    EXPECT_TRUE(msg.find("Resolution") != std::string::npos);
    EXPECT_TRUE(msg.find("cat") != std::string::npos);
    EXPECT_TRUE(msg.find("positive") != std::string::npos);
}

// Test conflict detection: suggest resolution
TEST(ArgumentParserTest, ConflictMessageSuggestsResolution) {
    Metadata meta;
    meta.table_name = "test_table";
    meta.x_axis_name = "time";
    meta.y_axis_name = "value";
    meta.target_col_name = "label";
    meta.x_meaning = "positive";
    meta.o_meaning = "negative";

    Arguments args;
    args.y_axis_name = "amplitude";  // Conflict!

    auto conflicts = ArgumentParser::detect_conflicts(args, meta);
    ASSERT_FALSE(conflicts.empty());

    const std::string& msg = conflicts[0];
    EXPECT_TRUE(msg.find("Remove") != std::string::npos ||
                msg.find("remove") != std::string::npos);
    EXPECT_TRUE(msg.find("different table") != std::string::npos);
}
