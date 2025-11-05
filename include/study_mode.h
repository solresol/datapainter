#pragma once

#include "database.h"
#include <string>
#include <vector>
#include <optional>

namespace datapainter {

// Information about a table column
struct ColumnInfo {
    std::string name;
    std::string type;  // "REAL", "TEXT", "INTEGER", etc.
};

// Result of validation
struct ValidationResult {
    bool is_valid;
    std::string error_message;
    std::vector<ColumnInfo> columns;
};

// Suggested bounds based on data
struct SuggestedBounds {
    double x_min;
    double x_max;
    double y_min;
    double y_max;
};

// User configuration for study mode
struct StudyConfiguration {
    std::string x_axis_col;
    std::string y_axis_col;
    std::string target_col;
    std::string x_meaning;
    std::string o_meaning;
    double x_min;
    double x_max;
    double y_min;
    double y_max;
};

// Manages study mode functionality
// Study mode allows importing and configuring existing tables
class StudyMode {
public:
    StudyMode(Database& db, const std::string& table_name);

    // Validate table structure
    // Returns validation result with error message if invalid
    ValidationResult validate();

    // Get distinct values for a column
    std::vector<std::string> get_distinct_values(const std::string& column_name);

    // Calculate suggested min/max bounds based on data
    std::optional<SuggestedBounds> calculate_suggested_bounds();

    // Create metadata entry with user configuration
    bool create_metadata(const StudyConfiguration& config);

private:
    Database& db_;
    std::string table_name_;

    // Helper: Check if metadata already exists
    bool metadata_exists();

    // Helper: Get column information from table
    std::vector<ColumnInfo> get_columns();

    // Helper: Count distinct values in a column
    int count_distinct_values(const std::string& column_name);

    // Helper: Check for NULL values in table
    bool has_null_values();
};

}  // namespace datapainter
