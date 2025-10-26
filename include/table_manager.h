#pragma once

#include "database.h"
#include <string>
#include <vector>
#include <ostream>

namespace datapainter {

// Manages non-interactive table operations
// Used for command-line operations like --create-table, --rename-table, etc.
class TableManager {
public:
    explicit TableManager(Database& db);

    // Table management operations
    bool create_table(const std::string& table_name,
                      const std::string& target_col_name,
                      const std::string& x_axis_name,
                      const std::string& y_axis_name,
                      const std::string& x_meaning,
                      const std::string& o_meaning,
                      double valid_x_min,
                      double valid_x_max,
                      double valid_y_min,
                      double valid_y_max,
                      bool show_zero_bars);

    bool rename_table(const std::string& old_name, const std::string& new_name);
    bool copy_table(const std::string& source, const std::string& destination);
    bool delete_table(const std::string& table_name);

    // Query operations
    std::vector<std::string> list_tables() const;
    bool show_metadata(const std::string& table_name, std::ostream& output) const;

    // Point operations
    bool add_point(const std::string& table_name, double x, double y,
                   const std::string& target);
    bool delete_point(const std::string& table_name, int point_id);

private:
    Database& db_;
};

}  // namespace datapainter
