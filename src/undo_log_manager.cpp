#include "undo_log_manager.h"
#include "unsaved_changes.h"
#include "save_manager.h"
#include "metadata.h"
#include <sqlite3.h>

namespace datapainter {

UndoLogManager::UndoLogManager(Database& db) : db_(db) {}

bool UndoLogManager::clear_undo_log(const std::string& table_name) {
    UnsavedChanges changes(db_);
    return changes.clear_changes(table_name);
}

bool UndoLogManager::clear_all_undo_logs() {
    const char* sql = "DELETE FROM unsaved_changes";
    return db_.execute(sql);
}

bool UndoLogManager::commit_unsaved_changes(const std::string& table_name) {
    // Verify table exists
    MetadataManager mgr(db_);
    if (!mgr.read(table_name).has_value()) {
        return false;
    }

    SaveManager saver(db_, table_name);
    return saver.save();
}

bool UndoLogManager::list_unsaved_changes(const std::string& table_name,
                                           std::ostream& output) {
    UnsavedChanges changes(db_);
    auto records = changes.get_changes(table_name);

    if (records.empty()) {
        output << "No unsaved changes for table: " << table_name << "\n";
        return true;
    }

    output << "Unsaved changes for " << table_name << ":\n";
    output << "-------------------------------------------\n";

    for (const auto& rec : records) {
        output << "ID: " << rec.id;
        output << ", Action: " << rec.action;
        output << ", Active: " << (rec.is_active ? "yes" : "no");

        if (rec.action == "insert") {
            output << ", Position: (" << rec.x.value() << ", " << rec.y.value() << ")";
            output << ", Target: " << rec.new_target.value();
        } else if (rec.action == "delete") {
            output << ", Data ID: " << rec.data_id.value();
            output << ", Position: (" << rec.x.value() << ", " << rec.y.value() << ")";
        } else if (rec.action == "update") {
            output << ", Data ID: " << rec.data_id.value();
            output << ", Old: " << rec.old_target.value();
            output << ", New: " << rec.new_target.value();
        } else if (rec.action == "meta") {
            output << ", Field: " << rec.meta_field.value();
            output << ", Old: " << rec.old_value.value();
            output << ", New: " << rec.new_value.value();
        }

        output << "\n";
    }

    return true;
}

}  // namespace datapainter
