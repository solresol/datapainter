#include "footer_renderer.h"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace datapainter {

void FooterRenderer::render(Terminal& terminal, double cursor_x, double cursor_y,
                           double x_min, double x_max, double y_min, double y_max,
                           int focused_button) {
    int rows = terminal.rows();
    int cols = terminal.cols();
    int footer_row = rows - 1;

    // Build footer string
    std::ostringstream footer;

    // Cursor position
    footer << "(" << format_coord(cursor_x) << ", " << format_coord(cursor_y) << ")";

    // Zoom controls
    footer << " | Zoom: ";
    footer << "+ - =";

    // Valid ranges
    footer << " | X:[" << format_coord(x_min) << "," << format_coord(x_max) << "]";
    footer << " Y:[" << format_coord(y_min) << "," << format_coord(y_max) << "]";

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
        footer_str = footer_str.substr(0, cols);
    }

    // Write to terminal
    for (size_t i = 0; i < footer_str.length(); ++i) {
        terminal.write_char(footer_row, i, footer_str[i]);
    }
}

std::string FooterRenderer::format_coord(double value) const {
    std::ostringstream oss;

    // Use scientific notation for very large or very small numbers
    if (std::abs(value) >= 1000.0 || (std::abs(value) < 0.01 && value != 0.0)) {
        oss << std::scientific << std::setprecision(1) << value;
    } else {
        // Use fixed notation with 1 decimal place
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
