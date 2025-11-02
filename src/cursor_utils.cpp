#include "cursor_utils.h"

namespace datapainter {

bool is_cursor_position_valid(const Viewport& viewport,
                              int cursor_screen_row,
                              int cursor_screen_col,
                              int edit_area_start_row) {
    // Convert absolute screen coordinates to content area coordinates
    // Content area is inside the border of the edit area
    // Border is 1 row at top, 1 row at bottom, 1 col at left, 1 col at right
    int cursor_content_row = cursor_screen_row - edit_area_start_row - 1;  // -1 for border
    int cursor_content_col = cursor_screen_col - 1;  // -1 for left border

    // Convert content area coordinates to data coordinates
    DataCoord data = viewport.screen_to_data({cursor_content_row, cursor_content_col});

    // Check if data coordinates are within valid ranges
    bool x_valid = (data.x >= viewport.valid_x_min() && data.x <= viewport.valid_x_max());
    bool y_valid = (data.y >= viewport.valid_y_min() && data.y <= viewport.valid_y_max());

    return x_valid && y_valid;
}

}  // namespace datapainter
