#include "unsaved_changes.h"
#include "database.h"
#include <sqlite3.h>

namespace datapainter {

UnsavedChanges::UnsavedChanges(Database& db) : db_(db) {}

std::optional<int> UnsavedChanges::record_insert(const std::string& table_name,
                                                   double x, double y, const std::string& target) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = R"(
        INSERT INTO unsaved_changes (table_name, action, x, y, new_target)
        VALUES (?, 'insert', ?, ?, ?)
    )";

    int rc = sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, table_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, x);
    sqlite3_bind_double(stmt, 3, y);
    sqlite3_bind_text(stmt, 4, target.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        return std::nullopt;
    }

    return static_cast<int>(sqlite3_last_insert_rowid(db_.connection()));
}

std::optional<int> UnsavedChanges::record_delete(const std::string& table_name, int data_id,
                                                   double x, double y, const std::string& target) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = R"(
        INSERT INTO unsaved_changes (table_name, action, data_id, x, y, old_target)
        VALUES (?, 'delete', ?, ?, ?, ?)
    )";

    int rc = sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, table_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, data_id);
    sqlite3_bind_double(stmt, 3, x);
    sqlite3_bind_double(stmt, 4, y);
    sqlite3_bind_text(stmt, 5, target.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        return std::nullopt;
    }

    return static_cast<int>(sqlite3_last_insert_rowid(db_.connection()));
}

std::optional<int> UnsavedChanges::record_update(const std::string& table_name, int data_id,
                                                   const std::string& old_target,
                                                   const std::string& new_target) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = R"(
        INSERT INTO unsaved_changes (table_name, action, data_id, old_target, new_target)
        VALUES (?, 'update', ?, ?, ?)
    )";

    int rc = sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, table_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, data_id);
    sqlite3_bind_text(stmt, 3, old_target.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, new_target.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        return std::nullopt;
    }

    return static_cast<int>(sqlite3_last_insert_rowid(db_.connection()));
}

std::optional<int> UnsavedChanges::record_metadata_change(const std::string& table_name,
                                                           const std::string& meta_field,
                                                           const std::string& old_value,
                                                           const std::string& new_value) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = R"(
        INSERT INTO unsaved_changes (table_name, action, meta_field, old_value, new_value)
        VALUES (?, 'meta', ?, ?, ?)
    )";

    int rc = sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, table_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, meta_field.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, old_value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, new_value.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        return std::nullopt;
    }

    return static_cast<int>(sqlite3_last_insert_rowid(db_.connection()));
}

std::vector<ChangeRecord> UnsavedChanges::get_changes(const std::string& table_name) {
    std::vector<ChangeRecord> records;

    sqlite3_stmt* stmt = nullptr;
    const char* sql = R"(
        SELECT id, table_name, action, data_id, x, y, old_target, new_target,
               meta_field, old_value, new_value, is_active
        FROM unsaved_changes
        WHERE table_name = ?
        ORDER BY id
    )";

    int rc = sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return records;
    }

    sqlite3_bind_text(stmt, 1, table_name.c_str(), -1, SQLITE_STATIC);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ChangeRecord rec;
        rec.id = sqlite3_column_int(stmt, 0);
        rec.table_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        rec.action = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
            rec.data_id = sqlite3_column_int(stmt, 3);
        }

        if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
            rec.x = sqlite3_column_double(stmt, 4);
        }

        if (sqlite3_column_type(stmt, 5) != SQLITE_NULL) {
            rec.y = sqlite3_column_double(stmt, 5);
        }

        if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
            rec.old_target = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        }

        if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) {
            rec.new_target = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        }

        if (sqlite3_column_type(stmt, 8) != SQLITE_NULL) {
            rec.meta_field = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        }

        if (sqlite3_column_type(stmt, 9) != SQLITE_NULL) {
            rec.old_value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        }

        if (sqlite3_column_type(stmt, 10) != SQLITE_NULL) {
            rec.new_value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        }

        rec.is_active = sqlite3_column_int(stmt, 11) != 0;

        records.push_back(rec);
    }

    sqlite3_finalize(stmt);
    return records;
}

std::vector<ChangeRecord> UnsavedChanges::get_all_changes() {
    std::vector<ChangeRecord> records;

    sqlite3_stmt* stmt = nullptr;
    const char* sql = R"(
        SELECT id, table_name, action, data_id, x, y, old_target, new_target,
               meta_field, old_value, new_value, is_active
        FROM unsaved_changes
        ORDER BY id
    )";

    int rc = sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return records;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ChangeRecord rec;
        rec.id = sqlite3_column_int(stmt, 0);
        rec.table_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        rec.action = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
            rec.data_id = sqlite3_column_int(stmt, 3);
        }

        if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
            rec.x = sqlite3_column_double(stmt, 4);
        }

        if (sqlite3_column_type(stmt, 5) != SQLITE_NULL) {
            rec.y = sqlite3_column_double(stmt, 5);
        }

        if (sqlite3_column_type(stmt, 6) != SQLITE_NULL) {
            rec.old_target = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        }

        if (sqlite3_column_type(stmt, 7) != SQLITE_NULL) {
            rec.new_target = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        }

        if (sqlite3_column_type(stmt, 8) != SQLITE_NULL) {
            rec.meta_field = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        }

        if (sqlite3_column_type(stmt, 9) != SQLITE_NULL) {
            rec.old_value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
        }

        if (sqlite3_column_type(stmt, 10) != SQLITE_NULL) {
            rec.new_value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
        }

        rec.is_active = sqlite3_column_int(stmt, 11) != 0;

        records.push_back(rec);
    }

    sqlite3_finalize(stmt);
    return records;
}

bool UnsavedChanges::clear_changes(const std::string& table_name) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "DELETE FROM unsaved_changes WHERE table_name = ?";

    int rc = sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, table_name.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool UnsavedChanges::clear_all_changes() {
    return db_.execute("DELETE FROM unsaved_changes");
}

bool UnsavedChanges::mark_change_inactive(int change_id) {
    const char* sql = "UPDATE unsaved_changes SET is_active = 0 WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_int(stmt, 1, change_id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool UnsavedChanges::update_insert_target(int change_id, const std::string& new_target) {
    const char* sql = "UPDATE unsaved_changes SET new_target = ? WHERE id = ? AND action = 'insert'";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, new_target.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, change_id);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

} // namespace datapainter
