#pragma once

#include "database.h"
#include <string>

namespace datapainter {

// Manages undo/redo operations for unsaved changes
// Tracks position in the change history and allows moving backward (undo) and forward (redo)
class UndoManager {
public:
    // Constructor takes database and table name
    UndoManager(Database& db, const std::string& table_name);

    // Refresh the undo manager state from the database
    // Call this after new changes are recorded
    // If clear_inactive is true, removes all inactive (undone) changes
    void refresh(bool clear_inactive = false);

    // Undo the last change (move position backward)
    // Returns true if undo was successful, false if at beginning
    bool undo();

    // Redo the next change (move position forward)
    // Returns true if redo was successful, false if at end
    bool redo();

    // Check if undo is available
    bool can_undo() const;

    // Check if redo is available
    bool can_redo() const;

    // Get current position in change history (0 = all undone, total = all active)
    int current_position() const;

    // Get number of undoable steps
    int undo_count() const;

    // Get number of redoable steps
    int redo_count() const;

private:
    Database& db_;
    std::string table_name_;
    int current_position_;  // Current position in change history
    int total_changes_;     // Total number of changes for this table
};

}  // namespace datapainter
