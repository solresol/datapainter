#pragma once

#include "viewport.h"

namespace datapainter {

// Check if a cursor position (in absolute screen coordinates) is within valid data ranges
//
// cursor_screen_row: Absolute screen row position of cursor
// cursor_screen_col: Absolute screen column position of cursor
// edit_area_start_row: Row where edit area starts (after header)
//
// Returns true if the cursor position, when converted to data coordinates,
// falls within the viewport's valid x and y ranges.
// Returns false if the cursor position would map to coordinates outside valid ranges.
bool is_cursor_position_valid(const Viewport& viewport,
                              int cursor_screen_row,
                              int cursor_screen_col,
                              int edit_area_start_row);

}  // namespace datapainter
