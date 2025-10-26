#include "database.h"
#include <iostream>

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

} // namespace datapainter
