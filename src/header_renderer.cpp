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
                           int focused_field) {
    int cols = terminal.cols();

    // Extract filename from database path
    std::string db_filename = extract_filename(db_path);

    // Row 0: Database and table name
    std::ostringstream row0;
    if (focused_field == 0) {
        row0 << "[" << db_filename << "]";
    } else {
        row0 << db_filename;
    }
    row0 << " | ";
    if (focused_field == 1) {
        row0 << "[" << table_name << "]";
    } else {
        row0 << table_name;
    }

    std::string row0_str = row0.str();
    // Truncate or pad to fit width
    if (static_cast<int>(row0_str.length()) > cols) {
        row0_str = row0_str.substr(0, cols);
    }
    for (size_t i = 0; i < row0_str.length(); ++i) {
        terminal.write_char(0, i, row0_str[i]);
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

    // Row 2: Counts and valid ranges
    std::ostringstream row2;
    row2 << "Total: " << total_count;
    row2 << " (x: " << x_count << ", o: " << o_count << ")";
    row2 << " | X: [" << format_value(x_min) << ", " << format_value(x_max) << "]";
    row2 << " Y: [" << format_value(y_min) << ", " << format_value(y_max) << "]";

    std::string row2_str = row2.str();
    if (static_cast<int>(row2_str.length()) > cols) {
        row2_str = row2_str.substr(0, cols);
    }
    for (size_t i = 0; i < row2_str.length(); ++i) {
        terminal.write_char(2, i, row2_str[i]);
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
