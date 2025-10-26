#include "metadata.h"
#include "database.h"
#include <sqlite3.h>

namespace datapainter {

MetadataManager::MetadataManager(Database& db) : db_(db) {}

bool MetadataManager::insert(const Metadata& meta) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = R"(
        INSERT INTO metadata (
            table_name, x_axis_name, y_axis_name, target_col_name,
            x_meaning, o_meaning, valid_x_min, valid_x_max,
            valid_y_min, valid_y_max, show_zero_bars
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";

    int rc = sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, meta.table_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, meta.x_axis_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, meta.y_axis_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, meta.target_col_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, meta.x_meaning.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, meta.o_meaning.c_str(), -1, SQLITE_TRANSIENT);

    if (meta.valid_x_min.has_value()) {
        sqlite3_bind_double(stmt, 7, meta.valid_x_min.value());
    } else {
        sqlite3_bind_null(stmt, 7);
    }

    if (meta.valid_x_max.has_value()) {
        sqlite3_bind_double(stmt, 8, meta.valid_x_max.value());
    } else {
        sqlite3_bind_null(stmt, 8);
    }

    if (meta.valid_y_min.has_value()) {
        sqlite3_bind_double(stmt, 9, meta.valid_y_min.value());
    } else {
        sqlite3_bind_null(stmt, 9);
    }

    if (meta.valid_y_max.has_value()) {
        sqlite3_bind_double(stmt, 10, meta.valid_y_max.value());
    } else {
        sqlite3_bind_null(stmt, 10);
    }

    sqlite3_bind_int(stmt, 11, meta.show_zero_bars ? 1 : 0);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::optional<Metadata> MetadataManager::read(const std::string& table_name) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = R"(
        SELECT table_name, x_axis_name, y_axis_name, target_col_name,
               x_meaning, o_meaning, valid_x_min, valid_x_max,
               valid_y_min, valid_y_max, show_zero_bars
        FROM metadata
        WHERE table_name = ?
    )";

    int rc = sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, table_name.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    Metadata meta;
    meta.table_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
    meta.x_axis_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    meta.y_axis_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    meta.target_col_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    meta.x_meaning = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    meta.o_meaning = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

    if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
        meta.valid_x_min = sqlite3_column_double(stmt, 6);
    }

    if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) {
        meta.valid_x_max = sqlite3_column_double(stmt, 7);
    }

    if (sqlite3_column_type(stmt, 8) != SQLITE_NULL) {
        meta.valid_y_min = sqlite3_column_double(stmt, 8);
    }

    if (sqlite3_column_type(stmt, 9) != SQLITE_NULL) {
        meta.valid_y_max = sqlite3_column_double(stmt, 9);
    }

    meta.show_zero_bars = sqlite3_column_int(stmt, 10) != 0;

    sqlite3_finalize(stmt);
    return meta;
}

bool MetadataManager::update(const Metadata& meta) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = R"(
        UPDATE metadata SET
            x_axis_name = ?, y_axis_name = ?, target_col_name = ?,
            x_meaning = ?, o_meaning = ?, valid_x_min = ?, valid_x_max = ?,
            valid_y_min = ?, valid_y_max = ?, show_zero_bars = ?
        WHERE table_name = ?
    )";

    int rc = sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, meta.x_axis_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, meta.y_axis_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, meta.target_col_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, meta.x_meaning.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, meta.o_meaning.c_str(), -1, SQLITE_TRANSIENT);

    if (meta.valid_x_min.has_value()) {
        sqlite3_bind_double(stmt, 6, meta.valid_x_min.value());
    } else {
        sqlite3_bind_null(stmt, 6);
    }

    if (meta.valid_x_max.has_value()) {
        sqlite3_bind_double(stmt, 7, meta.valid_x_max.value());
    } else {
        sqlite3_bind_null(stmt, 7);
    }

    if (meta.valid_y_min.has_value()) {
        sqlite3_bind_double(stmt, 8, meta.valid_y_min.value());
    } else {
        sqlite3_bind_null(stmt, 8);
    }

    if (meta.valid_y_max.has_value()) {
        sqlite3_bind_double(stmt, 9, meta.valid_y_max.value());
    } else {
        sqlite3_bind_null(stmt, 9);
    }

    sqlite3_bind_int(stmt, 10, meta.show_zero_bars ? 1 : 0);
    sqlite3_bind_text(stmt, 11, meta.table_name.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    int changes = sqlite3_changes(db_.connection());
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE && changes > 0;
}

bool MetadataManager::remove(const std::string& table_name) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "DELETE FROM metadata WHERE table_name = ?";

    int rc = sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, table_name.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    int changes = sqlite3_changes(db_.connection());
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE && changes > 0;
}

std::vector<std::string> MetadataManager::list_tables() {
    std::vector<std::string> tables;

    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT table_name FROM metadata ORDER BY table_name";

    int rc = sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return tables;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        tables.push_back(name);
    }

    sqlite3_finalize(stmt);
    return tables;
}

bool MetadataManager::create_data_table(const std::string& table_name) {
    // Create table with parameterized name (validated elsewhere)
    std::string sql = "CREATE TABLE IF NOT EXISTS " + table_name + " ("
                     "id INTEGER PRIMARY KEY, "
                     "x REAL NOT NULL, "
                     "y REAL NOT NULL, "
                     "target TEXT NOT NULL)";

    if (!db_.execute(sql)) {
        return false;
    }

    // Create xy index
    std::string idx_xy = "CREATE INDEX IF NOT EXISTS " + table_name + "_xy ON " +
                        table_name + "(x, y)";

    if (!db_.execute(idx_xy)) {
        return false;
    }

    // Create target index
    std::string idx_target = "CREATE INDEX IF NOT EXISTS " + table_name + "_target ON " +
                            table_name + "(target)";

    return db_.execute(idx_target);
}

bool MetadataManager::rename_table(const std::string& old_name, const std::string& new_name) {
    // Rename data table
    std::string sql = "ALTER TABLE " + old_name + " RENAME TO " + new_name;
    if (!db_.execute(sql)) {
        return false;
    }

    // Update metadata
    sqlite3_stmt* stmt = nullptr;
    const char* update_sql = "UPDATE metadata SET table_name = ? WHERE table_name = ?";

    int rc = sqlite3_prepare_v2(db_.connection(), update_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, new_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, old_name.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool MetadataManager::copy_table(const std::string& source_name, const std::string& dest_name) {
    // Copy data table structure and data
    std::string sql = "CREATE TABLE " + dest_name + " AS SELECT * FROM " + source_name;
    if (!db_.execute(sql)) {
        return false;
    }

    // Recreate indexes for destination
    if (!create_data_table(dest_name)) {
        return false;
    }

    // Copy metadata
    auto source_meta = read(source_name);
    if (!source_meta.has_value()) {
        return false;
    }

    source_meta->table_name = dest_name;
    return insert(*source_meta);
}

bool MetadataManager::delete_table(const std::string& table_name) {
    // Delete data table
    std::string sql = "DROP TABLE IF EXISTS " + table_name;
    if (!db_.execute(sql)) {
        return false;
    }

    // Delete metadata
    return remove(table_name);
}

} // namespace datapainter
