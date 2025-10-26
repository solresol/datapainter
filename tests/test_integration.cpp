#include <gtest/gtest.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <array>
#include <memory>

namespace fs = std::filesystem;

// Helper to run a command and capture output (both stdout and stderr)
std::string exec_command(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    // Redirect stderr to stdout so we capture all output
    std::string full_cmd = cmd + " 2>&1";
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(full_cmd.c_str(), "r"), pclose);
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
    }

    void TearDown() override {
        // Clean up test database
        if (fs::exists(test_db_)) {
            fs::remove(test_db_);
        }
    }

    std::string test_db_;
    std::string exe_ = "./datapainter";
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
