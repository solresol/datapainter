#include "footer_renderer.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace datapainter {

void FooterRenderer::render(Terminal& terminal, double cursor_x, double cursor_y,
                           double x_min, double x_max, double y_min, double y_max,
                           double vp_x_min, double vp_x_max, double vp_y_min, double vp_y_max,
                           int focused_button, int unsaved_changes_count) {
    int rows = terminal.rows();
    int cols = terminal.cols();
    int footer_row = rows - 1;

    // Calculate precision based on viewport and screen size
    // Subtract 2 for border
    int content_width = cols - 2;
    int content_height = rows - 4; // Header (3 rows) + footer (1 row)

    int x_precision = calculate_precision(vp_x_max - vp_x_min, content_width);
    int y_precision = calculate_precision(vp_y_max - vp_y_min, content_height);

    // Use the maximum of the two for consistency in display
    int cursor_precision = std::max(x_precision, y_precision);

    // Build footer string
    std::ostringstream footer;

    // Unsaved changes indicator (if any)
    if (unsaved_changes_count > 0) {
        footer << "[Unsaved: " << unsaved_changes_count << "] ";
    }

    // Cursor position with dynamic precision
    footer << "(" << format_coord(cursor_x, cursor_precision)
           << ", " << format_coord(cursor_y, cursor_precision) << ")";

    // Zoom controls
    footer << " | Zoom: ";
    footer << "+ - =";

    // Valid ranges (use fixed precision of 1 for ranges)
    footer << " | X:[" << format_coord(x_min, 1) << "," << format_coord(x_max, 1) << "]";
    footer << " Y:[" << format_coord(y_min, 1) << "," << format_coord(y_max, 1) << "]";

    // Action buttons
    footer << " | ";
    if (focused_button == 1) {
        footer << "[#:Tabular]";
    } else {
        footer << "#:Tabular";
    }
    footer << " ";
    if (focused_button == 2) {
        footer << "[u:Undo]";
    } else {
        footer << "u:Undo";
    }
    footer << " ";
    if (focused_button == 3) {
        footer << "[s:Save]";
    } else {
        footer << "s:Save";
    }
    footer << " ";
    if (focused_button == 4) {
        footer << "[q:Quit]";
    } else {
        footer << "q:Quit";
    }
    footer << " ?:Help";

    std::string footer_str = footer.str();

    // Truncate if too long
    if (static_cast<int>(footer_str.length()) > cols) {
        const std::string help_segment = " ?:Help";
        auto help_pos = footer_str.rfind(help_segment);
        if (help_pos != std::string::npos) {
            footer_str.erase(help_pos, help_segment.length());
        }
        footer_str = footer_str.substr(0, cols);
    }

    auto ensure_visible = [&](int button_index, const std::string& label) {
        if (focused_button == button_index && footer_str.find(label) == std::string::npos) {
            if (static_cast<int>(footer_str.length()) < cols) {
                footer_str.resize(cols, ' ');
            }
            int start = std::max(0, cols - static_cast<int>(label.length()));
            footer_str.replace(start, std::min<int>(label.length(), cols - start), label);
        }
    };

    ensure_visible(1, "[#:Tabular]");
    ensure_visible(2, "[u:Undo]");
    ensure_visible(3, "[s:Save]");
    ensure_visible(4, "[q:Quit]");

    // Write to terminal
    for (int col = 0; col < cols; ++col) {
        terminal.write_char(footer_row, col, ' ');
    }

    for (size_t i = 0; i < footer_str.length(); ++i) {
        terminal.write_char(footer_row, i, footer_str[i]);
    }
}

int FooterRenderer::calculate_precision(double range, int screen_size) const {
    if (screen_size <= 0 || range <= 0.0) {
        return 1; // Default precision
    }

    // Calculate data per cell
    double data_per_cell = range / static_cast<double>(screen_size);

    // Calculate required precision: we need enough decimal places such that
    // adjacent cells will show different values
    // precision = -floor(log10(data_per_cell)) + 1
    int precision = 1;
    if (data_per_cell > 0.0) {
        precision = static_cast<int>(-std::floor(std::log10(data_per_cell))) + 1;
    }

    // Clamp to reasonable range: minimum 1, maximum 8 decimal places
    precision = std::max(1, std::min(8, precision));

    return precision;
}

std::string FooterRenderer::format_coord(double value, int precision) const {
    std::ostringstream oss;

    // Use scientific notation for very large or very small numbers
    if (std::abs(value) >= 10000.0 || (std::abs(value) < 0.0001 && value != 0.0)) {
        // For scientific notation, use precision-1 (since one digit before decimal)
        oss << std::scientific << std::setprecision(std::max(0, precision - 1)) << value;
    } else {
        // Use fixed notation with calculated precision
        oss << std::fixed << std::setprecision(precision) << value;
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
