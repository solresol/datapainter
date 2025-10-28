#include <gtest/gtest.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <array>
#include <memory>
#include <vector>

namespace {
struct PipeCloser {
    void operator()(FILE* pipe) const {
        if (pipe != nullptr) {
            pclose(pipe);
        }
    }
};
} // namespace

namespace fs = std::filesystem;

// Helper to run a command and capture output (both stdout and stderr)
std::string exec_command(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    // Redirect stderr to stdout so we capture all output
    std::string full_cmd = cmd + " 2>&1";
    std::unique_ptr<FILE, PipeCloser> pipe(popen(full_cmd.c_str(), "r"), PipeCloser{});
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary test database
        test_db_ = "test_integration.db";
        // Clean up if it exists
        if (fs::exists(test_db_)) {
            fs::remove(test_db_);
        }

        exe_ = resolve_executable_path();
    }

    void TearDown() override {
        // Clean up test database
        if (fs::exists(test_db_)) {
            fs::remove(test_db_);
        }
    }

    static std::string resolve_executable_path() {
        const fs::path cwd = fs::current_path();
        const std::vector<fs::path> candidates = {
            cwd / "datapainter",
            cwd / "build" / "datapainter",
            cwd.parent_path() / "build" / "datapainter",
            cwd.parent_path() / "datapainter"
        };

        for (const auto& candidate : candidates) {
            if (!candidate.empty() && fs::exists(candidate)) {
                return candidate.string();
            }
        }

        throw std::runtime_error("Could not locate datapainter executable");
    }

    std::string test_db_;
    std::string exe_;
};

// Test: --list-tables on empty database
TEST_F(IntegrationTest, ListTablesEmpty) {
    std::string cmd = exe_ + " --database " + test_db_ + " --list-tables";
    std::string output = exec_command(cmd);

    // Should indicate no tables
    EXPECT_NE(output.find("No tables"), std::string::npos);
}

// Test: --create-table
TEST_F(IntegrationTest, CreateTable) {
    std::string cmd = exe_ + " --database " + test_db_ +
                      " --create-table --table test_table" +
                      " --target-column-name target" +
                      " --x-axis-name x --y-axis-name y" +
                      " --x-meaning x_val --o-meaning o_val" +
                      " --min-x -10.0 --max-x 10.0" +
                      " --min-y -10.0 --max-y 10.0";
    std::string output = exec_command(cmd);

    // Should succeed (exit code 0 and possibly success message)
    EXPECT_EQ(output.find("Error"), std::string::npos);
}

// Test: --list-tables after creating table
TEST_F(IntegrationTest, ListTablesAfterCreate) {
    // First create a table
    std::string create_cmd = exe_ + " --database " + test_db_ +
                             " --create-table --table test_table" +
                             " --target-column-name target" +
                             " --x-axis-name x --y-axis-name y" +
                             " --x-meaning x_val --o-meaning o_val" +
                             " --min-x -10.0 --max-x 10.0" +
                             " --min-y -10.0 --max-y 10.0";
    exec_command(create_cmd);

    // Then list tables
    std::string list_cmd = exe_ + " --database " + test_db_ + " --list-tables";
    std::string output = exec_command(list_cmd);

    EXPECT_NE(output.find("test_table"), std::string::npos);
}

// Test: --show-metadata
TEST_F(IntegrationTest, ShowMetadata) {
    // First create a table
    std::string create_cmd = exe_ + " --database " + test_db_ +
                             " --create-table --table test_table" +
                             " --target-column-name target" +
                             " --x-axis-name x --y-axis-name y" +
                             " --x-meaning x_val --o-meaning o_val" +
                             " --min-x -10.0 --max-x 10.0" +
                             " --min-y -10.0 --max-y 10.0";
    exec_command(create_cmd);

    // Show metadata
    std::string meta_cmd = exe_ + " --database " + test_db_ +
                           " --show-metadata --table test_table";
    std::string output = exec_command(meta_cmd);

    // Should show table information
    EXPECT_NE(output.find("test_table"), std::string::npos);
    EXPECT_NE(output.find("target"), std::string::npos);
    EXPECT_NE(output.find("x_val"), std::string::npos);
    EXPECT_NE(output.find("o_val"), std::string::npos);
}

// Test: --list-unsaved-changes on table with no changes
TEST_F(IntegrationTest, ListUnsavedChangesEmpty) {
    // First create a table
    std::string create_cmd = exe_ + " --database " + test_db_ +
                             " --create-table --table test_table" +
                             " --target-column-name target" +
                             " --x-axis-name x --y-axis-name y" +
                             " --x-meaning x_val --o-meaning o_val" +
                             " --min-x -10.0 --max-x 10.0" +
                             " --min-y -10.0 --max-y 10.0";
    exec_command(create_cmd);

    // List unsaved changes
    std::string list_cmd = exe_ + " --database " + test_db_ +
                           " --list-unsaved-changes --table test_table";
    std::string output = exec_command(list_cmd);

    // Should indicate no changes
    EXPECT_NE(output.find("No unsaved changes"), std::string::npos);
}

// Test: --add-point
TEST_F(IntegrationTest, AddPoint) {
    // First create a table
    std::string create_cmd = exe_ + " --database " + test_db_ +
                             " --create-table --table test_table" +
                             " --target-column-name target" +
                             " --x-axis-name x --y-axis-name y" +
                             " --x-meaning x_val --o-meaning o_val" +
                             " --min-x -10.0 --max-x 10.0" +
                             " --min-y -10.0 --max-y 10.0";
    exec_command(create_cmd);

    // Add a point
    std::string add_cmd = exe_ + " --database " + test_db_ +
                          " --add-point --table test_table" +
                          " --x 1.5 --y 2.5 --target x_val";
    std::string output = exec_command(add_cmd);

    // Should indicate success
    EXPECT_NE(output.find("Point added"), std::string::npos);
    EXPECT_NE(output.find("ID 1"), std::string::npos);
}

// Test: --delete-point
TEST_F(IntegrationTest, DeletePoint) {
    // First create a table
    std::string create_cmd = exe_ + " --database " + test_db_ +
                             " --create-table --table test_table" +
                             " --target-column-name target" +
                             " --x-axis-name x --y-axis-name y" +
                             " --x-meaning x_val --o-meaning o_val" +
                             " --min-x -10.0 --max-x 10.0" +
                             " --min-y -10.0 --max-y 10.0";
    exec_command(create_cmd);

    // Add a point
    std::string add_cmd = exe_ + " --database " + test_db_ +
                          " --add-point --table test_table" +
                          " --x 1.5 --y 2.5 --target x_val";
    exec_command(add_cmd);

    // Delete the point
    std::string delete_cmd = exe_ + " --database " + test_db_ +
                             " --delete-point --table test_table --point-id 1";
    std::string output = exec_command(delete_cmd);

    // Should indicate success
    EXPECT_NE(output.find("deleted successfully"), std::string::npos);
}

// Test: Add multiple points and verify they exist
TEST_F(IntegrationTest, AddMultiplePoints) {
    // First create a table
    std::string create_cmd = exe_ + " --database " + test_db_ +
                             " --create-table --table test_table" +
                             " --target-column-name target" +
                             " --x-axis-name x --y-axis-name y" +
                             " --x-meaning x_val --o-meaning o_val" +
                             " --min-x -10.0 --max-x 10.0" +
                             " --min-y -10.0 --max-y 10.0";
    exec_command(create_cmd);

    // Add three points
    exec_command(exe_ + " --database " + test_db_ +
                 " --add-point --table test_table" +
                 " --x 1.0 --y 2.0 --target x_val");
    exec_command(exe_ + " --database " + test_db_ +
                 " --add-point --table test_table" +
                 " --x 3.0 --y 4.0 --target o_val");
    std::string output = exec_command(exe_ + " --database " + test_db_ +
                                      " --add-point --table test_table" +
                                      " --x 5.0 --y 6.0 --target x_val");

    // Last point should be ID 3
    EXPECT_NE(output.find("ID 3"), std::string::npos);
}

// Test: Missing required arguments
TEST_F(IntegrationTest, MissingDatabaseArgument) {
    std::string cmd = exe_ + " --list-tables";
    std::string output = exec_command(cmd);

    // Should indicate error about missing database
    EXPECT_NE(output.find("database"), std::string::npos);
}

// Test: --add-point missing required arguments
TEST_F(IntegrationTest, AddPointMissingArguments) {
    // Create table first
    std::string create_cmd = exe_ + " --database " + test_db_ +
                             " --create-table --table test_table" +
                             " --target-column-name target" +
                             " --x-axis-name x --y-axis-name y" +
                             " --x-meaning x_val --o-meaning o_val" +
                             " --min-x -10.0 --max-x 10.0" +
                             " --min-y -10.0 --max-y 10.0";
    exec_command(create_cmd);

    // Try to add point without --x
    std::string cmd = exe_ + " --database " + test_db_ +
                      " --add-point --table test_table" +
                      " --y 2.0 --target x_val";
    std::string output = exec_command(cmd);

    EXPECT_NE(output.find("--x is required"), std::string::npos);
}

// Test: --delete-point with non-existent ID
TEST_F(IntegrationTest, DeleteNonExistentPoint) {
    // Create table first
    std::string create_cmd = exe_ + " --database " + test_db_ +
                             " --create-table --table test_table" +
                             " --target-column-name target" +
                             " --x-axis-name x --y-axis-name y" +
                             " --x-meaning x_val --o-meaning o_val" +
                             " --min-x -10.0 --max-x 10.0" +
                             " --min-y -10.0 --max-y 10.0";
    exec_command(create_cmd);

    // Try to delete non-existent point
    std::string cmd = exe_ + " --database " + test_db_ +
                      " --delete-point --table test_table --point-id 999";
    std::string output = exec_command(cmd);

    EXPECT_NE(output.find("Point not found"), std::string::npos);
}

// Test: --to-csv exports empty table
TEST_F(IntegrationTest, ToCsvEmpty) {
    // Create table
    std::string create_cmd = exe_ + " --database " + test_db_ +
                             " --create-table --table test_table" +
                             " --target-column-name target" +
                             " --x-axis-name x --y-axis-name y" +
                             " --x-meaning x_val --o-meaning o_val" +
                             " --min-x -10.0 --max-x 10.0" +
                             " --min-y -10.0 --max-y 10.0";
    exec_command(create_cmd);

    // Export to CSV
    std::string cmd = exe_ + " --database " + test_db_ + " --to-csv --table test_table";
    std::string output = exec_command(cmd);

    // Should have header but no data rows
    EXPECT_NE(output.find("x,y,target"), std::string::npos);
    // Count lines (header only = 1 line)
    int line_count = std::count(output.begin(), output.end(), '\n');
    EXPECT_EQ(line_count, 1);
}

// Test: --to-csv exports data
TEST_F(IntegrationTest, ToCsvWithData) {
    // Create table
    std::string create_cmd = exe_ + " --database " + test_db_ +
                             " --create-table --table test_table" +
                             " --target-column-name target" +
                             " --x-axis-name x --y-axis-name y" +
                             " --x-meaning x_val --o-meaning o_val" +
                             " --min-x -10.0 --max-x 10.0" +
                             " --min-y -10.0 --max-y 10.0";
    exec_command(create_cmd);

    // Add points
    exec_command(exe_ + " --database " + test_db_ +
                 " --add-point --table test_table --x 1.5 --y 2.5 --target x_val");
    exec_command(exe_ + " --database " + test_db_ +
                 " --add-point --table test_table --x 3.5 --y 4.5 --target o_val");

    // Export to CSV
    std::string cmd = exe_ + " --database " + test_db_ + " --to-csv --table test_table";
    std::string output = exec_command(cmd);

    // Should have header and two data rows
    EXPECT_NE(output.find("x,y,target"), std::string::npos);
    EXPECT_NE(output.find("1.5,2.5,x_val"), std::string::npos);
    EXPECT_NE(output.find("3.5,4.5,o_val"), std::string::npos);

    // Count lines (header + 2 data = 3 lines)
    int line_count = std::count(output.begin(), output.end(), '\n');
    EXPECT_EQ(line_count, 3);
}

// Test: --to-csv with special characters in target
TEST_F(IntegrationTest, ToCsvWithQuotes) {
    // Create table
    std::string create_cmd = exe_ + " --database " + test_db_ +
                             " --create-table --table test_table" +
                             " --target-column-name target" +
                             " --x-axis-name x --y-axis-name y" +
                             " --x-meaning x_val --o-meaning o_val" +
                             " --min-x -10.0 --max-x 10.0" +
                             " --min-y -10.0 --max-y 10.0";
    exec_command(create_cmd);

    // Add point with comma in target value
    exec_command(exe_ + " --database " + test_db_ +
                 " --add-point --table test_table --x 1.0 --y 2.0 --target \"has,comma\"");

    // Export to CSV
    std::string cmd = exe_ + " --database " + test_db_ + " --to-csv --table test_table";
    std::string output = exec_command(cmd);

    // Target value should be quoted
    EXPECT_NE(output.find("\"has,comma\""), std::string::npos);
}

// Test: --help flag
TEST_F(IntegrationTest, HelpFlag) {
    std::string cmd = exe_ + " --help";
    std::string output = exec_command(cmd);

    // Should show help message
    EXPECT_NE(output.find("USAGE:"), std::string::npos);
    EXPECT_NE(output.find("--database"), std::string::npos);
    EXPECT_NE(output.find("EXAMPLES:"), std::string::npos);
}

// Test: -h shorthand for help
TEST_F(IntegrationTest, HelpShorthand) {
    std::string cmd = exe_ + " -h";
    std::string output = exec_command(cmd);

    // Should show help message
    EXPECT_NE(output.find("USAGE:"), std::string::npos);
}

// Test: --version flag
TEST_F(IntegrationTest, VersionFlag) {
    std::string cmd = exe_ + " --version";
    std::string output = exec_command(cmd);

    // Should show version
    EXPECT_NE(output.find("DataPainter v"), std::string::npos);
}

// Test: --delete-table removes table
TEST_F(IntegrationTest, DeleteTable) {
    // Create table
    std::string create_cmd = exe_ + " --database " + test_db_ +
                             " --create-table --table test_table" +
                             " --target-column-name target" +
                             " --x-axis-name x --y-axis-name y" +
                             " --x-meaning x_val --o-meaning o_val";
    exec_command(create_cmd);

    // Verify table exists
    std::string list_cmd = exe_ + " --database " + test_db_ + " --list-tables";
    std::string list_output = exec_command(list_cmd);
    EXPECT_NE(list_output.find("test_table"), std::string::npos);

    // Delete table
    std::string delete_cmd = exe_ + " --database " + test_db_ +
                             " --delete-table --table test_table";
    std::string delete_output = exec_command(delete_cmd);
    EXPECT_NE(delete_output.find("deleted successfully"), std::string::npos);

    // Verify table no longer exists
    list_output = exec_command(list_cmd);
    EXPECT_NE(list_output.find("No tables"), std::string::npos);
}

// Test: --clear-undo-log
TEST_F(IntegrationTest, ClearUndoLog) {
    // Create table
    std::string create_cmd = exe_ + " --database " + test_db_ +
                             " --create-table --table test_table" +
                             " --target-column-name target" +
                             " --x-axis-name x --y-axis-name y" +
                             " --x-meaning x_val --o-meaning o_val";
    exec_command(create_cmd);

    // Add a point (creates undo log entry)
    exec_command(exe_ + " --database " + test_db_ +
                 " --add-point --table test_table --x 1.0 --y 2.0 --target x_val");

    // Clear undo log
    std::string clear_cmd = exe_ + " --database " + test_db_ +
                            " --clear-undo-log --table test_table";
    std::string output = exec_command(clear_cmd);

    EXPECT_NE(output.find("Undo log cleared"), std::string::npos);
}

// Test: --clear-all-undo-log
TEST_F(IntegrationTest, ClearAllUndoLogs) {
    // Create two tables
    exec_command(exe_ + " --database " + test_db_ +
                 " --create-table --table table1" +
                 " --target-column-name target" +
                 " --x-axis-name x --y-axis-name y" +
                 " --x-meaning x_val --o-meaning o_val");
    exec_command(exe_ + " --database " + test_db_ +
                 " --create-table --table table2" +
                 " --target-column-name target" +
                 " --x-axis-name x --y-axis-name y" +
                 " --x-meaning x_val --o-meaning o_val");

    // Add points to both tables
    exec_command(exe_ + " --database " + test_db_ +
                 " --add-point --table table1 --x 1.0 --y 2.0 --target x_val");
    exec_command(exe_ + " --database " + test_db_ +
                 " --add-point --table table2 --x 3.0 --y 4.0 --target o_val");

    // Clear all undo logs
    std::string clear_cmd = exe_ + " --database " + test_db_ + " --clear-all-undo-log";
    std::string output = exec_command(clear_cmd);

    EXPECT_NE(output.find("All undo logs cleared"), std::string::npos);
}

// Test: --commit-unsaved-changes
TEST_F(IntegrationTest, CommitUnsavedChanges) {
    // Create table
    std::string create_cmd = exe_ + " --database " + test_db_ +
                             " --create-table --table test_table" +
                             " --target-column-name target" +
                             " --x-axis-name x --y-axis-name y" +
                             " --x-meaning x_val --o-meaning o_val";
    exec_command(create_cmd);

    // Add a point (creates unsaved changes)
    exec_command(exe_ + " --database " + test_db_ +
                 " --add-point --table test_table --x 1.0 --y 2.0 --target x_val");

    // Commit unsaved changes
    std::string commit_cmd = exe_ + " --database " + test_db_ +
                             " --commit-unsaved-changes --table test_table";
    std::string output = exec_command(commit_cmd);

    EXPECT_NE(output.find("Unsaved changes committed"), std::string::npos);
}

// Test: End-to-end workflow: create, add points, export, verify
TEST_F(IntegrationTest, EndToEndWorkflow) {
    // Create a new table
    exec_command(exe_ + " --database " + test_db_ +
                 " --create-table --table iris" +
                 " --target-column-name species" +
                 " --x-axis-name sepal_length --y-axis-name sepal_width" +
                 " --x-meaning setosa --o-meaning other" +
                 " --min-x 4.0 --max-x 8.0 --min-y 2.0 --max-y 5.0");

    // Add several data points
    exec_command(exe_ + " --database " + test_db_ +
                 " --add-point --table iris --x 5.1 --y 3.5 --target setosa");
    exec_command(exe_ + " --database " + test_db_ +
                 " --add-point --table iris --x 7.0 --y 3.2 --target versicolor");
    exec_command(exe_ + " --database " + test_db_ +
                 " --add-point --table iris --x 6.3 --y 3.3 --target virginica");

    // Export to CSV
    std::string csv_output = exec_command(exe_ + " --database " + test_db_ +
                                          " --to-csv --table iris");

    // Verify CSV contains all points
    // Note: C++ iostream prints 7.0 as "7" (no trailing zero)
    EXPECT_NE(csv_output.find("5.1,3.5,setosa"), std::string::npos);
    EXPECT_NE(csv_output.find("7,3.2,versicolor"), std::string::npos);
    EXPECT_NE(csv_output.find("6.3,3.3,virginica"), std::string::npos);

    // Verify we have 4 lines (header + 3 data rows)
    int line_count = std::count(csv_output.begin(), csv_output.end(), '\n');
    EXPECT_EQ(line_count, 4);

    // Show metadata
    std::string meta_output = exec_command(exe_ + " --database " + test_db_ +
                                           " --show-metadata --table iris");
    EXPECT_NE(meta_output.find("iris"), std::string::npos);
    EXPECT_NE(meta_output.find("species"), std::string::npos);
    EXPECT_NE(meta_output.find("sepal_length"), std::string::npos);
    EXPECT_NE(meta_output.find("sepal_width"), std::string::npos);
}

// Test: Multiple tables in same database
TEST_F(IntegrationTest, MultipleTablesWorkflow) {
    // Create first table
    exec_command(exe_ + " --database " + test_db_ +
                 " --create-table --table train" +
                 " --target-column-name label" +
                 " --x-axis-name feature1 --y-axis-name feature2" +
                 " --x-meaning positive --o-meaning negative");

    // Create second table
    exec_command(exe_ + " --database " + test_db_ +
                 " --create-table --table test" +
                 " --target-column-name label" +
                 " --x-axis-name feature1 --y-axis-name feature2" +
                 " --x-meaning positive --o-meaning negative");

    // List tables
    std::string list_output = exec_command(exe_ + " --database " + test_db_ +
                                           " --list-tables");
    EXPECT_NE(list_output.find("train"), std::string::npos);
    EXPECT_NE(list_output.find("test"), std::string::npos);

    // Add points to both tables
    exec_command(exe_ + " --database " + test_db_ +
                 " --add-point --table train --x 1.0 --y 2.0 --target positive");
    exec_command(exe_ + " --database " + test_db_ +
                 " --add-point --table test --x 3.0 --y 4.0 --target negative");

    // Verify each table has its own data
    // Note: C++ iostream prints 1.0 as "1", 2.0 as "2", etc. (no trailing zeros)
    std::string train_csv = exec_command(exe_ + " --database " + test_db_ +
                                         " --to-csv --table train");
    EXPECT_NE(train_csv.find("1,2,positive"), std::string::npos);
    EXPECT_EQ(train_csv.find("3,4,negative"), std::string::npos);

    std::string test_csv = exec_command(exe_ + " --database " + test_db_ +
                                        " --to-csv --table test");
    EXPECT_NE(test_csv.find("3,4,negative"), std::string::npos);
    EXPECT_EQ(test_csv.find("1,2,positive"), std::string::npos);
}
