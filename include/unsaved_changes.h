#pragma once

#include <optional>
#include <string>
#include <vector>

namespace datapainter {

// Forward declaration
class Database;

// Represents a single change record
struct ChangeRecord {
    int id;
    std::string table_name;
    std::string action;  // "insert", "delete", "update", "meta"
    std::optional<int> data_id;
    std::optional<double> x;
    std::optional<double> y;
    std::optional<std::string> old_target;
    std::optional<std::string> new_target;
    std::optional<std::string> meta_field;
    std::optional<std::string> old_value;
    std::optional<std::string> new_value;
    bool is_active;  // Whether this change is currently active (not undone)
};

// Unsaved changes tracking
class UnsavedChanges {
public:
    explicit UnsavedChanges(Database& db);

    // Record actions (returns change id on success, nullopt on failure)
    std::optional<int> record_insert(const std::string& table_name,
                                     double x, double y, const std::string& target);

    std::optional<int> record_delete(const std::string& table_name, int data_id,
                                     double x, double y, const std::string& target);

    std::optional<int> record_update(const std::string& table_name, int data_id,
                                     const std::string& old_target,
                                     const std::string& new_target);

    std::optional<int> record_metadata_change(const std::string& table_name,
                                              const std::string& meta_field,
                                              const std::string& old_value,
                                              const std::string& new_value);

    // Retrieve changes
    std::vector<ChangeRecord> get_changes(const std::string& table_name);
    std::vector<ChangeRecord> get_all_changes();

    // Clear changes without applying
    bool clear_changes(const std::string& table_name);
    bool clear_all_changes();

private:
    Database& db_;
};

} // namespace datapainter
