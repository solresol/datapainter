#include "database.h"
#include <iostream>
#include <regex>

namespace datapainter {

Database::Database(const std::string& db_path) : db_path_(db_path), db_(nullptr) {
    int rc = sqlite3_open(db_path.c_str(), &db_);

    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }
}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

Database::Database(Database&& other) noexcept : db_path_(std::move(other.db_path_)), db_(other.db_) {
    other.db_ = nullptr;
}

Database& Database::operator=(Database&& other) noexcept {
    if (this != &other) {
        // Close our current connection
        if (db_) {
            sqlite3_close(db_);
        }

        // Take ownership of other's resources
        db_path_ = std::move(other.db_path_);
        db_ = other.db_;

        // Leave other in valid but empty state
        other.db_ = nullptr;
    }
    return *this;
}

bool Database::is_open() const {
    return db_ != nullptr;
}

const std::string& Database::path() const {
    return db_path_;
}

std::string Database::last_error() const {
    if (db_) {
        return sqlite3_errmsg(db_);
    }
    return "Database not open";
}

bool Database::execute(const std::string& sql) {
    if (!db_) {
        return false;
    }

    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);

    if (rc != SQLITE_OK) {
        if (err_msg) {
            std::cerr << "SQL error: " << err_msg << std::endl;
            sqlite3_free(err_msg);
        }
        return false;
    }

    return true;
}

sqlite3* Database::connection() {
    return db_;
}

bool Database::ensure_metadata_table() {
    if (!db_) {
        return false;
    }

    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS metadata (
            table_name        TEXT PRIMARY KEY,
            x_axis_name       TEXT NOT NULL,
            y_axis_name       TEXT NOT NULL,
            target_col_name   TEXT NOT NULL,
            x_meaning         TEXT NOT NULL,
            o_meaning         TEXT NOT NULL,
            valid_x_min       REAL,
            valid_x_max       REAL,
            valid_y_min       REAL,
            valid_y_max       REAL,
            show_zero_bars    INTEGER NOT NULL DEFAULT 0
        )
    )";

    return execute(sql);
}

bool Database::ensure_unsaved_changes_table() {
    if (!db_) {
        return false;
    }

    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS unsaved_changes (
            id            INTEGER PRIMARY KEY,
            table_name    TEXT NOT NULL,
            action        TEXT NOT NULL CHECK (action IN ('insert','delete','update','meta')),
            data_id       INTEGER,
            x             REAL,
            y             REAL,
            old_target    TEXT,
            new_target    TEXT,
            meta_field    TEXT,
            old_value     TEXT,
            new_value     TEXT,
            is_active     INTEGER NOT NULL DEFAULT 1
        )
    )";

    if (!execute(sql)) {
        return false;
    }

    // Create index
    const char* index_sql = R"(
        CREATE INDEX IF NOT EXISTS uc_table ON unsaved_changes(table_name, id)
    )";

    return execute(index_sql);
}

bool Database::table_exists(const std::string& table_name) {
    if (!db_) {
        return false;
    }

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name=?";

    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, table_name.c_str(), -1, SQLITE_STATIC);

    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_finalize(stmt);
    return exists;
}

bool Database::is_valid_table_name(const std::string& name) {
    if (name.empty()) {
        return false;
    }

    // Table name must match [A-Za-z0-9_]+
    // This means: one or more alphanumeric characters or underscores
    static const std::regex valid_name_regex("^[A-Za-z0-9_]+$");
    return std::regex_match(name, valid_name_regex);
}

} // namespace datapainter
