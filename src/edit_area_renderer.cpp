#include "edit_area_renderer.h"
#include <map>
#include <iostream>

#ifdef __unix__
#include <ncurses.h>
#endif

namespace datapainter {

void EditAreaRenderer::render(Terminal& terminal, const Viewport& viewport, DataTable& table,
                              const std::vector<ChangeRecord>& unsaved_changes, int start_row,
                              int height, int width, int cursor_row, int cursor_col,
                              const std::string& x_target, const std::string& o_target) {
    // Suppress unused parameter warnings for now
    (void)unsaved_changes;
    (void)cursor_row;
    (void)cursor_col;

    // Draw the border for the edit area
    draw_border(terminal, start_row, height, width);

    // Render all points in the viewport
    render_points(terminal, viewport, table, start_row, height, width, x_target, o_target);

    // Draw cursor (optional - for now we'll just verify it doesn't crash)
    // draw_cursor(terminal, cursor_row, cursor_col);
}

void EditAreaRenderer::draw_border(Terminal& terminal, int start_row, int height, int width) {
    // Calculate border position
    int end_row = start_row + height - 1;
    int end_col = width - 1;

#ifdef __unix__
    // Use ncurses line-drawing characters on Unix
    // Draw corners
    mvaddch(start_row, 0, ACS_ULCORNER);
    mvaddch(start_row, end_col, ACS_URCORNER);
    mvaddch(end_row, 0, ACS_LLCORNER);
    mvaddch(end_row, end_col, ACS_LRCORNER);

    // Draw top and bottom edges
    for (int col = 1; col < end_col; ++col) {
        mvaddch(start_row, col, ACS_HLINE);
        mvaddch(end_row, col, ACS_HLINE);
    }

    // Draw left and right edges
    for (int row = start_row + 1; row < end_row; ++row) {
        mvaddch(row, 0, ACS_VLINE);
        mvaddch(row, end_col, ACS_VLINE);
    }

    // Write to terminal buffer as well (for compatibility)
    terminal.write_char(start_row, 0, '+');
    terminal.write_char(start_row, end_col, '+');
    terminal.write_char(end_row, 0, '+');
    terminal.write_char(end_row, end_col, '+');
    for (int col = 1; col < end_col; ++col) {
        terminal.write_char(start_row, col, '-');
        terminal.write_char(end_row, col, '-');
    }
    for (int row = start_row + 1; row < end_row; ++row) {
        terminal.write_char(row, 0, '|');
        terminal.write_char(row, end_col, '|');
    }
#else
    // Use ASCII characters on other platforms
    terminal.write_char(start_row, 0, '+');
    terminal.write_char(start_row, end_col, '+');
    terminal.write_char(end_row, 0, '+');
    terminal.write_char(end_row, end_col, '+');

    for (int col = 1; col < end_col; ++col) {
        terminal.write_char(start_row, col, '-');
        terminal.write_char(end_row, col, '-');
    }

    for (int row = start_row + 1; row < end_row; ++row) {
        terminal.write_char(row, 0, '|');
        terminal.write_char(row, end_col, '|');
    }
#endif
}

void EditAreaRenderer::render_points(Terminal& terminal, const Viewport& viewport,
                                     DataTable& table, int start_row, int height, int width,
                                     const std::string& x_target, const std::string& o_target) {
    // Calculate content area (inside border)
    int content_height = height - 2;  // Exclude top and bottom border
    int content_width = width - 2;    // Exclude left and right border

    // First pass: Fill forbidden areas with '!' characters
    // These are areas outside the valid range
    for (int screen_row = 0; screen_row < content_height; ++screen_row) {
        for (int screen_col = 0; screen_col < content_width; ++screen_col) {
            ScreenCoord screen{screen_row, screen_col};
            DataCoord data = viewport.screen_to_data(screen);

            // Check if this cell is outside the valid range
            bool outside_valid_range = (data.x < viewport.valid_x_min() ||
                                       data.x > viewport.valid_x_max() ||
                                       data.y < viewport.valid_y_min() ||
                                       data.y > viewport.valid_y_max());

            if (outside_valid_range) {
                // Mark forbidden area with '!'
                terminal.write_char(start_row + 1 + screen_row, 1 + screen_col, '!');
            }
        }
    }

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
            // Ensure point is within content area bounds (viewport coordinates are 0-based)
            if (screen.row >= 0 && screen.row < content_height &&
                screen.col >= 0 && screen.col < content_width) {
                auto key = std::make_pair(screen.row, screen.col);
                if (point.target == x_target) {
                    cell_counts[key].first++;  // x count
                } else if (point.target == o_target) {
                    cell_counts[key].second++;  // o count
                }
            }
        }
    }

    // Second pass: Render points (will override '!' if points exist in forbidden areas)
    for (const auto& [coord, counts] : cell_counts) {
        auto [screen_row, screen_col] = coord;
        auto [x_count, o_count] = counts;

        // Adjust for border and start_row offset
        // Border is 1 char wide, so content starts at start_row+1, col 1
        char ch = get_point_char(x_count, o_count);
        terminal.write_char(start_row + 1 + screen_row, 1 + screen_col, ch);
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
