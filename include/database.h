#pragma once

#include <sqlite3.h>
#include <memory>
#include <string>

namespace datapainter {

// Database connection manager for DataPainter
// Handles SQLite connection lifecycle and basic table operations
class Database {
public:
    // Open or create a database at the given path
    // Use ":memory:" for in-memory database (useful for tests)
    explicit Database(const std::string& db_path);

    // Destructor closes the connection
    ~Database();

    // No copying (SQLite connection is unique)
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Moving is allowed
    Database(Database&& other) noexcept;
    Database& operator=(Database&& other) noexcept;

    // Check if database is successfully opened
    bool is_open() const;

    // Get the database path
    const std::string& path() const;

    // Get last error message
    std::string last_error() const;

    // Execute a SQL statement (for DDL like CREATE TABLE)
    bool execute(const std::string& sql);

    // Create the metadata table if it doesn't exist
    bool ensure_metadata_table();

    // Create the unsaved_changes table if it doesn't exist
    bool ensure_unsaved_changes_table();

    // Check if a table exists
    bool table_exists(const std::string& table_name);

    // Validate table name (must match [A-Za-z0-9_]+)
    static bool is_valid_table_name(const std::string& name);

    // Access to raw connection (for advanced operations)
    sqlite3* connection();

private:
    std::string db_path_;
    sqlite3* db_;
};

} // namespace datapainter
