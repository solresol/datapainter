#pragma once

#include "database.h"
#include <string>
#include <vector>
#include <optional>

namespace datapainter {

// Represents a single row in the table view
struct TableRow {
    int id;
    double x;
    double y;
    std::string target;
};

// Represents viewport bounds for filtering
struct ViewportBounds {
    double x_min;
    double x_max;
    double y_min;
    double y_max;
};

// Manages the tabular view of data points
// Displays data in a table format with columns for x, y, and target
// Supports filtering and navigation
class TableView {
public:
    // Constructor with optional viewport bounds for initial filter
    TableView(Database& db, const std::string& table_name,
              double x_min = -1e9, double x_max = 1e9,
              double y_min = -1e9, double y_max = 1e9);

    // Get all visible rows (applying current filter)
    std::vector<TableRow> get_visible_rows() const;

    // Get row count
    int row_count() const;

    // Get specific row by index (0-based)
    std::optional<TableRow> get_row(int index) const;

    // Get column headers
    std::vector<std::string> get_column_headers() const;

    // Navigation
    int current_row() const { return current_row_; }
    void set_current_row(int row);
    void move_up();
    void move_down();

    // Filter management
    void set_filter(const std::string& filter);
    std::string get_filter() const { return filter_; }

    // Get bounds of filtered data (for returning to viewport)
    std::optional<ViewportBounds> get_filter_bounds() const;

private:
    Database& db_;
    std::string table_name_;
    std::string filter_;
    int current_row_;

    // Build SQL query with current filter
    std::string build_query() const;

    // Refresh cached row count
    void refresh_row_count();
    mutable int cached_row_count_;
};

}  // namespace datapainter
