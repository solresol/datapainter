#include "save_manager.h"
#include "data_table.h"
#include "metadata.h"
#include "unsaved_changes.h"
#include <sqlite3.h>
#include <iostream>

namespace datapainter {

SaveManager::SaveManager(Database& db, const std::string& table_name)
    : db_(db), table_name_(table_name) {}

bool SaveManager::save() {
    // Begin transaction
    if (!db_.execute("BEGIN TRANSACTION")) {
        return false;
    }

    // Get all active changes for this table
    UnsavedChanges changes(db_);
    auto records = changes.get_changes(table_name_);

    // Apply each change
    for (const auto& rec : records) {
        if (!rec.is_active) {
            continue;  // Skip inactive (undone) changes
        }

        bool success = false;
        if (rec.action == "insert") {
            success = apply_insert(rec.id, rec.x.value(), rec.y.value(), rec.new_target.value());
        } else if (rec.action == "delete") {
            success = apply_delete(rec.data_id.value());
        } else if (rec.action == "update") {
            success = apply_update(rec.data_id.value(), rec.old_target.value(),
                                   rec.new_target.value());
        } else if (rec.action == "meta") {
            success = apply_metadata_change(rec.meta_field.value(), rec.old_value.value(),
                                            rec.new_value.value());
        }

        if (!success) {
            db_.execute("ROLLBACK");
            return false;
        }
    }

    // Clear unsaved changes for this table
    if (!changes.clear_changes(table_name_)) {
        db_.execute("ROLLBACK");
        return false;
    }

    // Commit transaction
    return db_.execute("COMMIT");
}

bool SaveManager::apply_insert(int /* change_id */, double x, double y,
                                const std::string& target) {
    DataTable dt(db_, table_name_);
    auto data_id = dt.insert_point(x, y, target);
    return data_id.has_value() && data_id.value() > 0;
}

bool SaveManager::apply_delete(int data_id) {
    DataTable dt(db_, table_name_);
    return dt.delete_point(data_id);
}

bool SaveManager::apply_update(int data_id, const std::string& /* old_target */,
                                const std::string& new_target) {
    DataTable dt(db_, table_name_);
    return dt.update_point_target(data_id, new_target);
}

bool SaveManager::apply_metadata_change(const std::string& field,
                                        const std::string& /* old_value */,
                                        const std::string& new_value) {
    MetadataManager mgr(db_);
    auto meta = mgr.read(table_name_);

    if (!meta.has_value()) {
        return false;
    }

    // Update the appropriate field
    if (field == "x_axis_name") {
        meta->x_axis_name = new_value;
    } else if (field == "y_axis_name") {
        meta->y_axis_name = new_value;
    } else if (field == "target_col_name") {
        meta->target_col_name = new_value;
    } else if (field == "x_meaning") {
        meta->x_meaning = new_value;
    } else if (field == "o_meaning") {
        meta->o_meaning = new_value;
    } else if (field == "valid_x_min") {
        meta->valid_x_min = std::stod(new_value);
    } else if (field == "valid_x_max") {
        meta->valid_x_max = std::stod(new_value);
    } else if (field == "valid_y_min") {
        meta->valid_y_min = std::stod(new_value);
    } else if (field == "valid_y_max") {
        meta->valid_y_max = std::stod(new_value);
    } else if (field == "show_zero_bars") {
        meta->show_zero_bars = (new_value == "1" || new_value == "true");
    } else {
        return false;  // Unknown field
    }

    return mgr.update(*meta);
}

}  // namespace datapainter
