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

    // Add a point - we need to parse this properly in main
    // For now, we'll need to think about how to pass x, y, target
    // This test will guide the implementation
}

// Test: Missing required arguments
TEST_F(IntegrationTest, MissingDatabaseArgument) {
    std::string cmd = exe_ + " --list-tables";
    std::string output = exec_command(cmd);

    // Should indicate error about missing database
    EXPECT_NE(output.find("database"), std::string::npos);
}
