#pragma once

#include "database.h"
#include <string>
#include <ostream>

namespace datapainter {

// Manages undo log operations for non-interactive commands
// Handles clearing, committing, and listing unsaved changes
class UndoLogManager {
public:
    explicit UndoLogManager(Database& db);

    // Clear unsaved changes for a specific table
    bool clear_undo_log(const std::string& table_name);

    // Clear all unsaved changes for all tables
    bool clear_all_undo_logs();

    // Commit unsaved changes for a specific table
    // Applies all changes and clears the undo log
    bool commit_unsaved_changes(const std::string& table_name);

    // List unsaved changes for a specific table
    bool list_unsaved_changes(const std::string& table_name, std::ostream& output);

private:
    Database& db_;
};

}  // namespace datapainter
