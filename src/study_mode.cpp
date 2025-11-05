#include "study_mode.h"
#include "metadata.h"
#include <sqlite3.h>
#include <algorithm>
#include <cmath>

namespace datapainter {

StudyMode::StudyMode(Database& db, const std::string& table_name)
    : db_(db), table_name_(table_name) {
}

bool StudyMode::metadata_exists() {
    MetadataManager mgr(db_);
    auto meta = mgr.read(table_name_);
    return meta.has_value();
}

std::vector<ColumnInfo> StudyMode::get_columns() {
    std::vector<ColumnInfo> columns;

    // Query table info using PRAGMA table_info
    std::string query = "PRAGMA table_info(" + table_name_ + ")";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.connection(), query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return columns;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ColumnInfo col;
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        const char* type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        col.name = name ? name : "";
        col.type = type ? type : "";
        columns.push_back(col);
    }

    sqlite3_finalize(stmt);
    return columns;
}

int StudyMode::count_distinct_values(const std::string& column_name) {
    std::string query = "SELECT COUNT(DISTINCT " + column_name + ") FROM " + table_name_;
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.connection(), query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return 0;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

bool StudyMode::has_null_values() {
    auto columns = get_columns();

    for (const auto& col : columns) {
        std::string query = "SELECT COUNT(*) FROM " + table_name_ +
                           " WHERE " + col.name + " IS NULL";
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db_.connection(), query.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            continue;
        }

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int null_count = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
            if (null_count > 0) {
                return true;
            }
        } else {
            sqlite3_finalize(stmt);
        }
    }

    return false;
}

ValidationResult StudyMode::validate() {
    ValidationResult result;
    result.is_valid = true;

    // Check if metadata already exists
    if (metadata_exists()) {
        result.is_valid = false;
        result.error_message = "Metadata already exists for table '" + table_name_ + "'";
        return result;
    }

    // Get column information
    result.columns = get_columns();

    // Validate exactly 3 columns
    if (result.columns.size() != 3) {
        result.is_valid = false;
        result.error_message = "Table must have exactly 3 columns, found " +
                               std::to_string(result.columns.size());
        return result;
    }

    // Count REAL columns
    int real_count = 0;
    std::string text_col_name;
    for (const auto& col : result.columns) {
        if (col.type == "REAL") {
            real_count++;
        } else if (col.type == "TEXT") {
            text_col_name = col.name;
        }
    }

    // Validate 2 REAL columns
    if (real_count != 2) {
        result.is_valid = false;
        result.error_message = "Table must have exactly 2 columns of type REAL, found " +
                               std::to_string(real_count);
        return result;
    }

    // Validate third column has exactly 2 distinct values
    if (!text_col_name.empty()) {
        int distinct_count = count_distinct_values(text_col_name);
        if (distinct_count != 2) {
            result.is_valid = false;
            result.error_message = "Target column must have exactly 2 distinct values, found " +
                                   std::to_string(distinct_count);
            return result;
        }
    }

    // Check for NULL values
    if (has_null_values()) {
        result.is_valid = false;
        result.error_message = "Table contains NULL values, which are not allowed";
        return result;
    }

    return result;
}

std::vector<std::string> StudyMode::get_distinct_values(const std::string& column_name) {
    std::vector<std::string> values;

    std::string query = "SELECT DISTINCT " + column_name + " FROM " + table_name_;
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.connection(), query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return values;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (value) {
            values.push_back(value);
        }
    }

    sqlite3_finalize(stmt);
    return values;
}

std::optional<SuggestedBounds> StudyMode::calculate_suggested_bounds() {
    auto columns = get_columns();

    // Find the two REAL columns
    std::vector<std::string> real_cols;
    for (const auto& col : columns) {
        if (col.type == "REAL") {
            real_cols.push_back(col.name);
        }
    }

    if (real_cols.size() != 2) {
        return std::nullopt;
    }

    // Query min/max for both columns
    std::string query = "SELECT MIN(" + real_cols[0] + "), MAX(" + real_cols[0] + "), " +
                       "MIN(" + real_cols[1] + "), MAX(" + real_cols[1] + ") " +
                       "FROM " + table_name_;

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.connection(), query.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::nullopt;
    }

    SuggestedBounds bounds;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        double min_col0 = sqlite3_column_double(stmt, 0);
        double max_col0 = sqlite3_column_double(stmt, 1);
        double min_col1 = sqlite3_column_double(stmt, 2);
        double max_col1 = sqlite3_column_double(stmt, 3);

        // Add 10% padding
        double range_col0 = max_col0 - min_col0;
        double range_col1 = max_col1 - min_col1;

        bounds.x_min = min_col0 - range_col0 * 0.1;
        bounds.x_max = max_col0 + range_col0 * 0.1;
        bounds.y_min = min_col1 - range_col1 * 0.1;
        bounds.y_max = max_col1 + range_col1 * 0.1;

        sqlite3_finalize(stmt);
        return bounds;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

bool StudyMode::create_metadata(const StudyConfiguration& config) {
    MetadataManager mgr(db_);

    Metadata meta;
    meta.table_name = table_name_;
    meta.x_axis_name = config.x_axis_col;
    meta.y_axis_name = config.y_axis_col;
    meta.target_col_name = config.target_col;
    meta.x_meaning = config.x_meaning;
    meta.o_meaning = config.o_meaning;
    meta.valid_x_min = config.x_min;
    meta.valid_x_max = config.x_max;
    meta.valid_y_min = config.y_min;
    meta.valid_y_max = config.y_max;
    meta.show_zero_bars = false;

    return mgr.insert(meta);
}

}  // namespace datapainter
