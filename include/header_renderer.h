#pragma once

#include "terminal.h"
#include <string>

namespace datapainter {

// Renders the header area showing database info, table name, counts, and metadata
class HeaderRenderer {
public:
    HeaderRenderer() = default;

    // Render the header to the terminal
    // Parameters:
    //   terminal: Terminal buffer to render to
    //   db_path: Path to the database file
    //   table_name: Name of the current table
    //   target_col: Name of the target column
    //   x_meaning: Meaning of 'x' points
    //   o_meaning: Meaning of 'o' points
    //   total_count: Total number of points
    //   x_count: Number of x points
    //   o_count: Number of o points
    //   x_min: Minimum valid x value
    //   x_max: Maximum valid x value
    //   y_min: Minimum valid y value
    //   y_max: Maximum valid y value
    //   vp_x_min: Current viewport minimum x
    //   vp_x_max: Current viewport maximum x
    //   vp_y_min: Current viewport minimum y
    //   vp_y_max: Current viewport maximum y
    //   focused_field: Which field has focus (for Tab navigation, 0-based)
    //   unsaved_changes_count: Number of unsaved changes (0 if none)
    void render(Terminal& terminal, const std::string& db_path,
                const std::string& table_name, const std::string& target_col,
                const std::string& x_meaning, const std::string& o_meaning,
                int total_count, int x_count, int o_count,
                double x_min, double x_max, double y_min, double y_max,
                double vp_x_min, double vp_x_max, double vp_y_min, double vp_y_max,
                int focused_field, int unsaved_changes_count = 0);

private:
    // Extract just the filename from a full path
    std::string extract_filename(const std::string& path) const;

    // Format a double value with appropriate precision
    std::string format_value(double value) const;
};

}  // namespace datapainter
