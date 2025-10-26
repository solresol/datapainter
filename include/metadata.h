#pragma once

#include <optional>
#include <string>
#include <vector>

namespace datapainter {

// Forward declaration
class Database;

// Metadata for a data table
struct Metadata {
    std::string table_name;
    std::string x_axis_name;
    std::string y_axis_name;
    std::string target_col_name;
    std::string x_meaning;
    std::string o_meaning;
    std::optional<double> valid_x_min;
    std::optional<double> valid_x_max;
    std::optional<double> valid_y_min;
    std::optional<double> valid_y_max;
    bool show_zero_bars = false;
};

// Metadata operations
class MetadataManager {
public:
    explicit MetadataManager(Database& db);

    // Insert new metadata (returns false if table_name already exists)
    bool insert(const Metadata& meta);

    // Read metadata for a table (returns nullopt if not found)
    std::optional<Metadata> read(const std::string& table_name);

    // Update existing metadata (returns false if not found)
    bool update(const Metadata& meta);

    // Delete metadata (returns false if not found)
    bool remove(const std::string& table_name);

    // List all table names
    std::vector<std::string> list_tables();

    // Create data table with indexes for a given table name
    // Uses metadata.table_name to determine table name
    bool create_data_table(const std::string& table_name);

    // Rename a table (updates both data table and metadata)
    bool rename_table(const std::string& old_name, const std::string& new_name);

    // Copy a table (copies both data and metadata)
    bool copy_table(const std::string& source_name, const std::string& dest_name);

    // Delete a table (removes both data table and metadata)
    bool delete_table(const std::string& table_name);

private:
    Database& db_;
};

} // namespace datapainter
