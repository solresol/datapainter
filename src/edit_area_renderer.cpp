#include "edit_area_renderer.h"
#include <map>
#include <iostream>

namespace datapainter {

void EditAreaRenderer::render(Terminal& terminal, const Viewport& viewport, DataTable& table,
                              const std::vector<ChangeRecord>& unsaved_changes, int cursor_row,
                              int cursor_col, const std::string& x_target,
                              const std::string& o_target) {
    // Suppress unused parameter warnings for now
    (void)unsaved_changes;
    (void)cursor_row;
    (void)cursor_col;

    // Clear the terminal first
    terminal.clear_buffer();

    // Draw the border
    draw_border(terminal);

    // Render all points in the viewport
    render_points(terminal, viewport, table, x_target, o_target);

    // Draw cursor (optional - for now we'll just verify it doesn't crash)
    // draw_cursor(terminal, cursor_row, cursor_col);
}

void EditAreaRenderer::draw_border(Terminal& terminal) {
    int rows = terminal.rows();
    int cols = terminal.cols();

    // Draw corners
    terminal.write_char(0, 0, '+');
    terminal.write_char(0, cols - 1, '+');
    terminal.write_char(rows - 1, 0, '+');
    terminal.write_char(rows - 1, cols - 1, '+');

    // Draw top and bottom edges
    for (int col = 1; col < cols - 1; ++col) {
        terminal.write_char(0, col, '-');
        terminal.write_char(rows - 1, col, '-');
    }

    // Draw left and right edges
    for (int row = 1; row < rows - 1; ++row) {
        terminal.write_char(row, 0, '|');
        terminal.write_char(row, cols - 1, '|');
    }
}

void EditAreaRenderer::render_points(Terminal& terminal, const Viewport& viewport,
                                     DataTable& table, const std::string& x_target,
                                     const std::string& o_target) {
    // Query all points within the viewport bounds
    auto points = table.query_viewport(viewport.data_x_min(), viewport.data_x_max(),
                                       viewport.data_y_min(), viewport.data_y_max());

    // Map from screen coordinates to counts of x and o points
    std::map<std::pair<int, int>, std::pair<int, int>> cell_counts;

    // Count points at each screen cell
    for (const auto& point : points) {
        DataCoord data{point.x, point.y};
        auto screen_opt = viewport.data_to_screen(data);

        // Check if point maps to valid screen coordinates
        if (screen_opt.has_value()) {
            auto screen = screen_opt.value();
            // Ensure point is within screen bounds (excluding border)
            if (screen.row >= 0 && screen.row < viewport.screen_height() &&
                screen.col >= 0 && screen.col < viewport.screen_width()) {
                auto key = std::make_pair(screen.row, screen.col);
                if (point.target == x_target) {
                    cell_counts[key].first++;  // x count
                } else if (point.target == o_target) {
                    cell_counts[key].second++;  // o count
                }
            }
        }
    }

    // Render each cell
    for (const auto& [coord, counts] : cell_counts) {
        auto [screen_row, screen_col] = coord;
        auto [x_count, o_count] = counts;

        // Adjust for border (add 1 to both row and col)
        char ch = get_point_char(x_count, o_count);
        terminal.write_char(screen_row + 1, screen_col + 1, ch);
    }
}

void EditAreaRenderer::draw_cursor(Terminal& terminal, int cursor_row, int cursor_col) {
    // Suppress unused parameter warnings
    (void)terminal;
    (void)cursor_row;
    (void)cursor_col;
    // Cursor rendering to be implemented
    // For now, this is a placeholder
}

char EditAreaRenderer::get_point_char(int x_count, int o_count) const {
    if (x_count > 0 && o_count > 0) {
        return '#';  // Mixed
    } else if (x_count > 1) {
        return 'X';  // Multiple x's
    } else if (o_count > 1) {
        return 'O';  // Multiple o's
    } else if (x_count == 1) {
        return 'x';  // Single x
    } else if (o_count == 1) {
        return 'o';  // Single o
    }
    return ' ';  // Empty
}

}  // namespace datapainter
