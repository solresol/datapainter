#include "table_manager.h"
#include "metadata.h"
#include "data_table.h"
#include <sqlite3.h>
#include <sstream>

namespace datapainter {

TableManager::TableManager(Database& db) : db_(db) {}

bool TableManager::create_table(const std::string& table_name,
                                 const std::string& target_col_name,
                                 const std::string& x_axis_name,
                                 const std::string& y_axis_name,
                                 const std::string& x_meaning,
                                 const std::string& o_meaning,
                                 double valid_x_min,
                                 double valid_x_max,
                                 double valid_y_min,
                                 double valid_y_max,
                                 bool show_zero_bars) {
    MetadataManager mgr(db_);

    // Create the data table first
    if (!mgr.create_data_table(table_name)) {
        return false;
    }

    // Create metadata
    Metadata meta;
    meta.table_name = table_name;
    meta.target_col_name = target_col_name;
    meta.x_axis_name = x_axis_name;
    meta.y_axis_name = y_axis_name;
    meta.x_meaning = x_meaning;
    meta.o_meaning = o_meaning;
    meta.valid_x_min = valid_x_min;
    meta.valid_x_max = valid_x_max;
    meta.valid_y_min = valid_y_min;
    meta.valid_y_max = valid_y_max;
    meta.show_zero_bars = show_zero_bars;

    return mgr.insert(meta);
}

bool TableManager::rename_table(const std::string& old_name, const std::string& new_name) {
    MetadataManager mgr(db_);
    return mgr.rename_table(old_name, new_name);
}

bool TableManager::copy_table(const std::string& source, const std::string& destination) {
    MetadataManager mgr(db_);
    return mgr.copy_table(source, destination);
}

bool TableManager::delete_table(const std::string& table_name) {
    MetadataManager mgr(db_);
    // MetadataManager::delete_table handles both data and metadata
    return mgr.delete_table(table_name);
}

std::vector<std::string> TableManager::list_tables() const {
    MetadataManager mgr(db_);
    return mgr.list_tables();
}

bool TableManager::show_metadata(const std::string& table_name, std::ostream& output) const {
    MetadataManager mgr(db_);
    auto meta = mgr.read(table_name);

    if (!meta.has_value()) {
        return false;
    }

    output << "Table: " << meta->table_name << "\n";
    output << "Target column: " << meta->target_col_name << "\n";
    output << "X axis: " << meta->x_axis_name << "\n";
    output << "Y axis: " << meta->y_axis_name << "\n";
    output << "X meaning: " << meta->x_meaning << "\n";
    output << "O meaning: " << meta->o_meaning << "\n";
    output << "Valid X range: ["
           << (meta->valid_x_min.has_value() ? meta->valid_x_min.value() : 0.0) << ", "
           << (meta->valid_x_max.has_value() ? meta->valid_x_max.value() : 0.0) << "]\n";
    output << "Valid Y range: ["
           << (meta->valid_y_min.has_value() ? meta->valid_y_min.value() : 0.0) << ", "
           << (meta->valid_y_max.has_value() ? meta->valid_y_max.value() : 0.0) << "]\n";
    output << "Show zero bars: " << (meta->show_zero_bars ? "yes" : "no") << "\n";

    return true;
}

bool TableManager::add_point(const std::string& table_name, double x, double y,
                              const std::string& target) {
    // Verify table exists
    MetadataManager mgr(db_);
    if (!mgr.read(table_name).has_value()) {
        return false;
    }

    DataTable dt(db_, table_name);
    auto result = dt.insert_point(x, y, target);
    return result.has_value();
}

bool TableManager::delete_point(const std::string& table_name, int point_id) {
    // Verify table exists
    MetadataManager mgr(db_);
    if (!mgr.read(table_name).has_value()) {
        return false;
    }

    DataTable dt(db_, table_name);
    return dt.delete_point(point_id);
}

}  // namespace datapainter
