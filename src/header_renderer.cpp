#include "header_renderer.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace datapainter {

void HeaderRenderer::render(Terminal& terminal, const std::string& db_path,
                           const std::string& table_name, const std::string& target_col,
                           const std::string& x_meaning, const std::string& o_meaning,
                           int total_count, int x_count, int o_count,
                           double x_min, double x_max, double y_min, double y_max,
                           double vp_x_min, double vp_x_max, double vp_y_min, double vp_y_max,
                           int focused_field, int unsaved_changes_count) {
    int cols = terminal.cols();

    // Extract filename from database path
    std::string db_filename = extract_filename(db_path);

    // Row 0: Database and table name, with unsaved changes indicator
    std::ostringstream row0_left;
    if (focused_field == 0) {
        row0_left << "[" << db_filename << "]";
    } else {
        row0_left << db_filename;
    }
    row0_left << " | ";
    if (focused_field == 1) {
        row0_left << "[" << table_name << "]";
    } else {
        row0_left << table_name;
    }

    std::string row0_left_str = row0_left.str();

    // Add unsaved changes indicator on the right side if there are unsaved changes
    std::string row0_right_str;
    if (unsaved_changes_count > 0) {
        std::ostringstream row0_right;
        row0_right << "[Unsaved: " << unsaved_changes_count << "]";
        row0_right_str = row0_right.str();
    }

    // Write left side
    int left_len = std::min(static_cast<int>(row0_left_str.length()),
                            cols - static_cast<int>(row0_right_str.length()) - 1);
    for (int i = 0; i < left_len; ++i) {
        terminal.write_char(0, i, row0_left_str[i]);
    }

    // Write right side (right-aligned) if there are unsaved changes
    if (!row0_right_str.empty()) {
        int right_start = cols - row0_right_str.length();
        if (right_start > left_len) {
            for (size_t i = 0; i < row0_right_str.length(); ++i) {
                terminal.write_char(0, right_start + i, row0_right_str[i]);
            }
        }
    }

    // Row 1: Target column and meanings
    std::ostringstream row1;
    if (focused_field == 2) {
        row1 << "[" << target_col << "]";
    } else {
        row1 << target_col;
    }
    row1 << ": ";
    if (focused_field == 3) {
        row1 << "x=[" << x_meaning << "]";
    } else {
        row1 << "x=" << x_meaning;
    }
    row1 << " ";
    if (focused_field == 4) {
        row1 << "o=[" << o_meaning << "]";
    } else {
        row1 << "o=" << o_meaning;
    }

    std::string row1_str = row1.str();
    if (static_cast<int>(row1_str.length()) > cols) {
        row1_str = row1_str.substr(0, cols);
    }
    for (size_t i = 0; i < row1_str.length(); ++i) {
        terminal.write_char(1, i, row1_str[i]);
    }

    // Row 2: Counts on left, viewport range and zoom on right
    std::ostringstream row2_left;
    row2_left << "Total: " << total_count;
    row2_left << " (x: " << x_count << ", o: " << o_count << ")";
    row2_left << " Valid X: [" << format_value(x_min) << ", " << format_value(x_max) << "]";
    row2_left << " Y: [" << format_value(y_min) << ", " << format_value(y_max) << "]";

    // Calculate zoom percentage based on viewport size vs valid range size
    double valid_x_range = x_max - x_min;
    double valid_y_range = y_max - y_min;
    double vp_x_range = vp_x_max - vp_x_min;
    double vp_y_range = vp_y_max - vp_y_min;

    // Use the smaller of the two percentages (most zoomed axis)
    double x_pct = (valid_x_range > 0) ? (vp_x_range / valid_x_range * 100.0) : 100.0;
    double y_pct = (valid_y_range > 0) ? (vp_y_range / valid_y_range * 100.0) : 100.0;
    double zoom_pct = std::min(x_pct, y_pct);

    std::ostringstream row2_right;
    row2_right << "View X: [" << format_value(vp_x_min) << ", " << format_value(vp_x_max) << "]";
    row2_right << " Y: [" << format_value(vp_y_min) << ", " << format_value(vp_y_max) << "]";
    row2_right << " Zoom: " << std::fixed << std::setprecision(0) << zoom_pct << "%";

    std::string left_str = row2_left.str();
    std::string right_str = row2_right.str();

    // Write left side
    int row2_left_len = std::min(static_cast<int>(left_str.length()), cols - static_cast<int>(right_str.length()) - 2);
    for (int i = 0; i < row2_left_len; ++i) {
        terminal.write_char(2, i, left_str[i]);
    }

    // Write right side (right-aligned)
    int right_start = cols - right_str.length();
    if (right_start > row2_left_len) {
        for (size_t i = 0; i < right_str.length(); ++i) {
            terminal.write_char(2, right_start + i, right_str[i]);
        }
    }
}

std::string HeaderRenderer::extract_filename(const std::string& path) const {
    // Find the last path separator
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return path;
}

std::string HeaderRenderer::format_value(double value) const {
    std::ostringstream oss;

    // Use scientific notation for very large or very small numbers
    if (std::abs(value) >= 10000.0 || (std::abs(value) < 0.001 && value != 0.0)) {
        oss << std::scientific << std::setprecision(2) << value;
    } else {
        // Use fixed notation with appropriate precision
        // Remove trailing zeros
        oss << std::fixed << std::setprecision(1) << value;
        std::string result = oss.str();

        // Remove trailing zeros after decimal point
        if (result.find('.') != std::string::npos) {
            result.erase(result.find_last_not_of('0') + 1, std::string::npos);
            if (result.back() == '.') {
                result.pop_back();
            }
        }
        return result;
    }

    return oss.str();
}

}  // namespace datapainter
