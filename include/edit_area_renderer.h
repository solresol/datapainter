#pragma once

#include "terminal.h"
#include "viewport.h"
#include "data_table.h"
#include "unsaved_changes.h"
#include <vector>

namespace datapainter {

// Renders the edit area (viewport) with data points and border
class EditAreaRenderer {
public:
    EditAreaRenderer() = default;

    // Render the edit area to the terminal
    // Parameters:
    //   terminal: Terminal buffer to render to
    //   viewport: Current viewport showing data coordinates
    //   table: Data table containing points
    //   unsaved_changes: List of unsaved changes to apply
    //   cursor_row: Current cursor row position (in screen coordinates)
    //   cursor_col: Current cursor column position (in screen coordinates)
    //   x_target: Target value that represents 'x' points
    //   o_target: Target value that represents 'o' points
    void render(Terminal& terminal, const Viewport& viewport, DataTable& table,
                const std::vector<ChangeRecord>& unsaved_changes, int cursor_row,
                int cursor_col, const std::string& x_target, const std::string& o_target);

private:
    void draw_border(Terminal& terminal);
    void render_points(Terminal& terminal, const Viewport& viewport, DataTable& table,
                       const std::string& x_target, const std::string& o_target);
    void draw_cursor(Terminal& terminal, int cursor_row, int cursor_col);

    // Character to use for different point combinations
    char get_point_char(int x_count, int o_count) const;
};

}  // namespace datapainter
