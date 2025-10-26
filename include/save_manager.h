#pragma once

#include "database.h"
#include <string>

namespace datapainter {

// Manages saving unsaved changes to the database
// Applies all active changes in a transaction and clears them after successful save
class SaveManager {
public:
    // Constructor takes database and table name
    SaveManager(Database& db, const std::string& table_name);

    // Save all active changes to the database
    // Returns true if save was successful, false otherwise
    // Applies changes in a transaction for atomicity
    bool save();

private:
    Database& db_;
    std::string table_name_;

    // Helper methods for applying different change types
    bool apply_insert(int change_id, double x, double y, const std::string& target);
    bool apply_delete(int data_id);
    bool apply_update(int data_id, const std::string& old_target, const std::string& new_target);
    bool apply_metadata_change(const std::string& field, const std::string& old_value,
                                const std::string& new_value);
};

}  // namespace datapainter
