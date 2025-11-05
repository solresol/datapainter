#include <gtest/gtest.h>
#include "database.h"
#include "metadata.h"
#include "study_mode.h"
#include <sqlite3.h>

using namespace datapainter;

class StudyModeTest : public ::testing::Test {
protected:
    StudyModeTest() : db_(":memory:") {}

    void SetUp() override {
        db_.ensure_metadata_table();
    }

    // Helper to create a table with 3 columns (2 REAL, 1 TEXT with 2 distinct values)
    void create_valid_table(const std::string& table_name) {
        std::string create_sql = "CREATE TABLE " + table_name + " ("
                                "x_col REAL, "
                                "y_col REAL, "
                                "class TEXT)";
        char* err = nullptr;
        sqlite3_exec(db_.connection(), create_sql.c_str(), nullptr, nullptr, &err);
        if (err) {
            std::cerr << "Create table error: " << err << std::endl;
            sqlite3_free(err);
        }

        // Insert some test data
        std::string insert1 = "INSERT INTO " + table_name + " VALUES (1.0, 2.0, 'A')";
        std::string insert2 = "INSERT INTO " + table_name + " VALUES (3.0, 4.0, 'B')";
        std::string insert3 = "INSERT INTO " + table_name + " VALUES (5.0, 6.0, 'A')";
        sqlite3_exec(db_.connection(), insert1.c_str(), nullptr, nullptr, &err);
        sqlite3_exec(db_.connection(), insert2.c_str(), nullptr, nullptr, &err);
        sqlite3_exec(db_.connection(), insert3.c_str(), nullptr, nullptr, &err);
    }

    Database db_;
};

// Test: Check if metadata already exists (error if yes)
TEST_F(StudyModeTest, ErrorIfMetadataExists) {
    create_valid_table("test_table");

    // Create metadata for the table
    MetadataManager mgr(db_);
    Metadata meta;
    meta.table_name = "test_table";
    meta.target_col_name = "class";
    meta.x_axis_name = "x_col";
    meta.y_axis_name = "y_col";
    meta.x_meaning = "A";
    meta.o_meaning = "B";
    mgr.insert(meta);

    // Try to run study mode on table with existing metadata
    StudyMode study(db_, "test_table");
    auto result = study.validate();

    EXPECT_FALSE(result.is_valid);
    EXPECT_FALSE(result.error_message.empty());
    EXPECT_NE(result.error_message.find("Metadata"), std::string::npos);
}

// Test: Validate exactly 3 columns exist
TEST_F(StudyModeTest, ValidateThreeColumns) {
    // Create table with 2 columns (invalid)
    std::string create_sql = "CREATE TABLE test_table (x_col REAL, y_col REAL)";
    sqlite3_exec(db_.connection(), create_sql.c_str(), nullptr, nullptr, nullptr);

    StudyMode study(db_, "test_table");
    auto result = study.validate();

    EXPECT_FALSE(result.is_valid);
    EXPECT_NE(result.error_message.find("3 columns"), std::string::npos);
}

// Test: Validate 2 columns are REAL type
TEST_F(StudyModeTest, ValidateTwoRealColumns) {
    // Create table with only 1 REAL column (invalid)
    std::string create_sql = "CREATE TABLE test_table ("
                            "x_col REAL, "
                            "y_col TEXT, "
                            "class TEXT)";
    sqlite3_exec(db_.connection(), create_sql.c_str(), nullptr, nullptr, nullptr);

    StudyMode study(db_, "test_table");
    auto result = study.validate();

    EXPECT_FALSE(result.is_valid);
    EXPECT_NE(result.error_message.find("2 columns"), std::string::npos);
    EXPECT_NE(result.error_message.find("REAL"), std::string::npos);
}

// Test: Validate third column has exactly 2 distinct values
TEST_F(StudyModeTest, ValidateTwoDistinctValues) {
    create_valid_table("test_table");

    // Add a third distinct value (invalid)
    std::string insert = "INSERT INTO test_table VALUES (7.0, 8.0, 'C')";
    sqlite3_exec(db_.connection(), insert.c_str(), nullptr, nullptr, nullptr);

    StudyMode study(db_, "test_table");
    auto result = study.validate();

    EXPECT_FALSE(result.is_valid);
    EXPECT_NE(result.error_message.find("2 distinct"), std::string::npos);
}

// Test: Validate no nulls in any column
TEST_F(StudyModeTest, ValidateNoNulls) {
    create_valid_table("test_table");

    // Add a row with NULL (invalid)
    std::string insert = "INSERT INTO test_table VALUES (NULL, 10.0, 'A')";
    sqlite3_exec(db_.connection(), insert.c_str(), nullptr, nullptr, nullptr);

    StudyMode study(db_, "test_table");
    auto result = study.validate();

    EXPECT_FALSE(result.is_valid);
    EXPECT_NE(result.error_message.find("NULL"), std::string::npos);
}

// Test: Get column information for valid table
TEST_F(StudyModeTest, GetColumnInfo) {
    create_valid_table("test_table");

    StudyMode study(db_, "test_table");
    auto result = study.validate();

    EXPECT_TRUE(result.is_valid);
    EXPECT_EQ(result.columns.size(), 3);

    // Check that we identified 2 REAL columns and 1 TEXT column
    int real_count = 0;
    int text_count = 0;
    for (const auto& col : result.columns) {
        if (col.type == "REAL") real_count++;
        if (col.type == "TEXT") text_count++;
    }
    EXPECT_EQ(real_count, 2);
    EXPECT_EQ(text_count, 1);
}

// Test: Get distinct values for target column
TEST_F(StudyModeTest, GetDistinctValues) {
    create_valid_table("test_table");

    StudyMode study(db_, "test_table");
    auto result = study.validate();

    EXPECT_TRUE(result.is_valid);

    // Find the TEXT column
    std::string text_col_name;
    for (const auto& col : result.columns) {
        if (col.type == "TEXT") {
            text_col_name = col.name;
            break;
        }
    }

    auto distinct_values = study.get_distinct_values(text_col_name);
    EXPECT_EQ(distinct_values.size(), 2);
    // Values should be "A" and "B"
    EXPECT_TRUE(std::find(distinct_values.begin(), distinct_values.end(), "A") != distinct_values.end());
    EXPECT_TRUE(std::find(distinct_values.begin(), distinct_values.end(), "B") != distinct_values.end());
}

// Test: Calculate suggested min/max based on data
TEST_F(StudyModeTest, CalculateSuggestedBounds) {
    create_valid_table("test_table");

    StudyMode study(db_, "test_table");

    // Assuming x_col has values 1.0, 3.0, 5.0
    // Assuming y_col has values 2.0, 4.0, 6.0
    auto bounds = study.calculate_suggested_bounds();

    EXPECT_TRUE(bounds.has_value());
    EXPECT_LE(bounds->x_min, 1.0);
    EXPECT_GE(bounds->x_max, 5.0);
    EXPECT_LE(bounds->y_min, 2.0);
    EXPECT_GE(bounds->y_max, 6.0);
}

// Test: Create metadata entry with user configuration
TEST_F(StudyModeTest, CreateMetadataEntry) {
    create_valid_table("test_table");

    StudyMode study(db_, "test_table");
    auto result = study.validate();
    ASSERT_TRUE(result.is_valid);

    // Configure study mode
    StudyConfiguration config;
    config.x_axis_col = "x_col";
    config.y_axis_col = "y_col";
    config.target_col = "class";
    config.x_meaning = "A";
    config.o_meaning = "B";
    config.x_min = 0.0;
    config.x_max = 10.0;
    config.y_min = 0.0;
    config.y_max = 10.0;

    bool success = study.create_metadata(config);
    EXPECT_TRUE(success);

    // Verify metadata was created
    MetadataManager mgr(db_);
    auto meta = mgr.read("test_table");
    ASSERT_TRUE(meta.has_value());
    EXPECT_EQ(meta->table_name, "test_table");
    EXPECT_EQ(meta->x_axis_name, "x_col");
    EXPECT_EQ(meta->y_axis_name, "y_col");
    EXPECT_EQ(meta->target_col_name, "class");
    EXPECT_EQ(meta->x_meaning, "A");
    EXPECT_EQ(meta->o_meaning, "B");
    EXPECT_EQ(meta->valid_x_min, 0.0);
    EXPECT_EQ(meta->valid_x_max, 10.0);
    EXPECT_EQ(meta->valid_y_min, 0.0);
    EXPECT_EQ(meta->valid_y_max, 10.0);
}
