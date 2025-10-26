#include "undo_manager.h"
#include "unsaved_changes.h"
#include <sqlite3.h>

namespace datapainter {

UndoManager::UndoManager(Database& db, const std::string& table_name)
    : db_(db), table_name_(table_name), current_position_(0), total_changes_(0) {
    refresh();
}

void UndoManager::refresh(bool clear_inactive) {
    // If clear_inactive is true, remove all inactive changes (clears redo stack)
    if (clear_inactive) {
        const char* delete_sql = "DELETE FROM unsaved_changes WHERE table_name = ? AND is_active = 0";
        sqlite3_stmt* delete_stmt = nullptr;

        if (sqlite3_prepare_v2(db_.connection(), delete_sql, -1, &delete_stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(delete_stmt, 1, table_name_.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(delete_stmt);
            sqlite3_finalize(delete_stmt);
        }
    }

    // Count total changes for this table
    const char* sql = "SELECT COUNT(*) FROM unsaved_changes WHERE table_name = ?";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }

    sqlite3_bind_text(stmt, 1, table_name_.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        total_changes_ = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);

    // Count active changes to determine current position
    sql = "SELECT COUNT(*) FROM unsaved_changes WHERE table_name = ? AND is_active = 1";
    stmt = nullptr;

    if (sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return;
    }

    sqlite3_bind_text(stmt, 1, table_name_.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        current_position_ = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
}

bool UndoManager::undo() {
    if (!can_undo()) {
        return false;
    }

    // Find the last active change and mark it as inactive
    const char* sql = R"(
        UPDATE unsaved_changes
        SET is_active = 0
        WHERE id = (
            SELECT id FROM unsaved_changes
            WHERE table_name = ? AND is_active = 1
            ORDER BY id DESC
            LIMIT 1
        )
    )";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, table_name_.c_str(), -1, SQLITE_TRANSIENT);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);

    if (success) {
        current_position_--;
    }

    return success;
}

bool UndoManager::redo() {
    if (!can_redo()) {
        return false;
    }

    // Find the first inactive change in id order and mark it as active
    const char* sql = R"(
        UPDATE unsaved_changes
        SET is_active = 1
        WHERE table_name = ? AND id = (
            SELECT MIN(id) FROM unsaved_changes
            WHERE table_name = ? AND is_active = 0
        )
    )";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db_.connection(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, table_name_.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, table_name_.c_str(), -1, SQLITE_TRANSIENT);

    bool success = sqlite3_step(stmt) == SQLITE_DONE && sqlite3_changes(db_.connection()) > 0;
    sqlite3_finalize(stmt);

    if (success) {
        current_position_++;
    }

    return success;
}

bool UndoManager::can_undo() const {
    return current_position_ > 0;
}

bool UndoManager::can_redo() const {
    return current_position_ < total_changes_;
}

int UndoManager::current_position() const {
    return current_position_;
}

int UndoManager::undo_count() const {
    return current_position_;
}

int UndoManager::redo_count() const {
    return total_changes_ - current_position_;
}

}  // namespace datapainter
