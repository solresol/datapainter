#include <gtest/gtest.h>
#include "database.h"

using namespace datapainter;

// Test valid table names
TEST(TableValidationTest, ValidTableNamesAreAccepted) {
    EXPECT_TRUE(Database::is_valid_table_name("simple"));
    EXPECT_TRUE(Database::is_valid_table_name("with_underscores"));
    EXPECT_TRUE(Database::is_valid_table_name("MixedCase123"));
    EXPECT_TRUE(Database::is_valid_table_name("numbers123"));
    EXPECT_TRUE(Database::is_valid_table_name("ALLCAPS"));
    EXPECT_TRUE(Database::is_valid_table_name("a"));  // Single character
    EXPECT_TRUE(Database::is_valid_table_name("_starts_with_underscore"));
}

// Test invalid table names with spaces
TEST(TableValidationTest, TableNamesWithSpacesAreRejected) {
    EXPECT_FALSE(Database::is_valid_table_name("has space"));
    EXPECT_FALSE(Database::is_valid_table_name("multiple  spaces"));
    EXPECT_FALSE(Database::is_valid_table_name(" leading_space"));
    EXPECT_FALSE(Database::is_valid_table_name("trailing_space "));
}

// Test invalid table names with special characters
TEST(TableValidationTest, TableNamesWithSpecialCharsAreRejected) {
    EXPECT_FALSE(Database::is_valid_table_name("has-dash"));
    EXPECT_FALSE(Database::is_valid_table_name("has.dot"));
    EXPECT_FALSE(Database::is_valid_table_name("has@symbol"));
    EXPECT_FALSE(Database::is_valid_table_name("has$dollar"));
    EXPECT_FALSE(Database::is_valid_table_name("has!exclamation"));
    EXPECT_FALSE(Database::is_valid_table_name("has#hash"));
    EXPECT_FALSE(Database::is_valid_table_name("has%percent"));
    EXPECT_FALSE(Database::is_valid_table_name("has&ampersand"));
    EXPECT_FALSE(Database::is_valid_table_name("has*asterisk"));
}

// Test empty string
TEST(TableValidationTest, EmptyStringIsRejected) {
    EXPECT_FALSE(Database::is_valid_table_name(""));
}

// Test SQL injection attempts
TEST(TableValidationTest, SQLInjectionAttemptsAreRejected) {
    EXPECT_FALSE(Database::is_valid_table_name("table'; DROP TABLE users--"));
    EXPECT_FALSE(Database::is_valid_table_name("table;"));
    EXPECT_FALSE(Database::is_valid_table_name("table'"));
    EXPECT_FALSE(Database::is_valid_table_name("table\""));
}

// Test unicode/non-ASCII characters
TEST(TableValidationTest, NonASCIICharactersAreRejected) {
    EXPECT_FALSE(Database::is_valid_table_name("café"));
    EXPECT_FALSE(Database::is_valid_table_name("日本語"));
    EXPECT_FALSE(Database::is_valid_table_name("émoji"));
}
