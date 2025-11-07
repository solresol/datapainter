# Table View Implementation Status

**Date**: 2025-11-07
**Task**: Implementing Table View UI feature (pressing `#` to switch between viewport and table view modes)

## Context

The DataPainter project is 83.6% complete (353/422 tasks) with all tests passing:
- 487 unit tests passing
- 53 integration tests passing (1 skipped)
- Production-ready with full CI/CD, documentation, and release process

## What Was Completed

### 1. Integration Test File Created
**File**: `/Users/gregb/Documents/devel/datapainter/tests/integration/test_table_view.py`

Contains comprehensive tests for:
- `TestTableViewSwitching::test_switch_to_table_view_with_hash_key` - Tests pressing `#` switches to table view
- `TestTableViewSwitching::test_switch_back_to_viewport_from_table` - Tests pressing `#` again returns to viewport
- `TestTableViewDisplay::test_display_filter_at_top` - Tests filter shown at top of table view
- `TestTableViewDisplay::test_table_shows_column_headers` - Tests x, y, target headers displayed
- `TestTableViewNavigation::test_navigate_rows_with_arrow_keys` - Tests arrow key navigation in table

**Note**: Tests are written but currently fail because feature isn't implemented yet. There was also an I/O error in the test framework that needs investigation, but tests are structurally correct.

## What Needs to Be Implemented

### Implementation Plan for src/main.cpp

The `TableView` class already exists in `include/table_view.h` and `src/table_view.cpp` with these methods:
- `get_visible_rows()` - Returns rows based on current filter
- `get_column_headers()` - Returns ["x", "y", "target"]
- `move_up()` / `move_down()` - Navigation
- `set_current_row()` / `current_row()` - Row selection
- `set_filter()` / `get_filter()` - Filter management
- `get_filter_bounds()` - Get viewport bounds for filtered data

### Required Changes to main.cpp

1. **Add ViewMode enum** (near top of main.cpp):
```cpp
enum class ViewMode {
    VIEWPORT,  // Default viewport mode with cursor and axes
    TABLE      // Tabular data editing mode
};
```

2. **Add view_mode variable** (in main() function with other state):
```cpp
ViewMode view_mode = ViewMode::VIEWPORT;
TableView* table_view = nullptr;  // Lazy initialize when needed
```

3. **Add '#' key handler** (in the input handling switch statement):
```cpp
case '#':
    // Toggle between viewport and table view
    if (view_mode == ViewMode::VIEWPORT) {
        // Switch to table view
        view_mode = ViewMode::TABLE;

        // Initialize TableView with current viewport bounds as filter
        if (table_view == nullptr) {
            table_view = new TableView(db, table_name,
                                       viewport.x_min(), viewport.x_max(),
                                       viewport.y_min(), viewport.y_max());
        }

        needs_redraw = true;
    } else {
        // Switch back to viewport
        view_mode = ViewMode::VIEWPORT;

        // Optionally: adjust viewport to fit filtered data
        auto bounds = table_view->get_filter_bounds();
        if (bounds.has_value()) {
            viewport.set_bounds(bounds->x_min, bounds->x_max,
                              bounds->y_min, bounds->y_max);
        }

        needs_redraw = true;
    }
    break;
```

4. **Add table view rendering function** (new function near other rendering functions):
```cpp
void render_table_view(TerminalInterface& term, const TableView& table_view,
                       int width, int height) {
    int row = 1;  // Start below header

    // Display filter at top
    term.move_cursor(1, row++);
    std::string filter = table_view.get_filter();
    if (filter.empty()) {
        term.write("Filter: (all rows)");
    } else {
        term.write("Filter: " + filter);
    }
    row++;  // Blank line

    // Display column headers
    term.move_cursor(1, row++);
    term.write("  x          y          target");
    term.move_cursor(1, row++);
    term.write("─────────────────────────────────");

    // Display rows
    auto rows = table_view.get_visible_rows();
    int current_row_idx = table_view.current_row();

    for (size_t i = 0; i < rows.size() && row < height - 1; i++) {
        term.move_cursor(1, row);

        // Highlight current row
        if (i == current_row_idx) {
            term.set_color(COLOR_BLACK, COLOR_WHITE);
        }

        char buf[100];
        snprintf(buf, sizeof(buf), "  %-10.4f %-10.4f %s",
                 rows[i].x, rows[i].y, rows[i].target.c_str());
        term.write(buf);

        if (i == current_row_idx) {
            term.reset_color();
        }

        row++;
    }

    // Display status line
    term.move_cursor(1, height);
    char status[100];
    snprintf(status, sizeof(status),
             "Table View | Row %d/%d | Press # to return to viewport",
             current_row_idx + 1, (int)rows.size());
    term.write(status);
}
```

5. **Modify main render loop** (where screen is drawn):
```cpp
// In the main loop where rendering happens
if (view_mode == ViewMode::VIEWPORT) {
    // Existing viewport rendering code
    render_viewport(...);
} else {
    // Table view rendering
    if (table_view != nullptr) {
        render_table_view(term, *table_view, screen_width, screen_height);
    }
}
```

6. **Handle arrow keys in table mode** (in input handling):
```cpp
case KEY_UP:
    if (view_mode == ViewMode::TABLE && table_view != nullptr) {
        table_view->move_up();
        needs_redraw = true;
    } else {
        // Existing viewport cursor movement
    }
    break;

case KEY_DOWN:
    if (view_mode == ViewMode::TABLE && table_view != nullptr) {
        table_view->move_down();
        needs_redraw = true;
    } else {
        // Existing viewport cursor movement
    }
    break;
```

7. **Cleanup** (in main() cleanup section):
```cpp
delete table_view;  // Add this with other cleanup
```

## Key Files

- **Main implementation**: `src/main.cpp` (needs modifications above)
- **TableView class**: `include/table_view.h` and `src/table_view.cpp` (already implemented)
- **Integration tests**: `tests/integration/test_table_view.py` (written, currently failing)
- **TODO tracking**: Line 354-374 in `TODO.md` (Phase 11: Tabular View section)

## Estimated Effort

~200-300 lines of new code in main.cpp, approximately 2-3 hours of work including:
- Implementation
- Manual testing
- Debugging integration tests
- Handling edge cases

## Alternative Approaches

If table view is too complex, other TODO items to consider:
1. **Keystroke Playback** (Phase 17.5) - ~18 tasks, enables automated testing
2. **Edge case tests** - Fill in missing test coverage (~6 tasks)
3. **Documentation updates** - Mark remaining items for future releases

## Current Todo List State

1. ✅ Write integration test for switching to table view with # key
2. ⏳ Examine main.cpp to understand current structure
3. ⏸️ Add ViewMode enum (Viewport/Table) to track current view
4. ⏸️ Implement # key handler to toggle view modes
5. ⏸️ Implement table view rendering function
6. ⏸️ Build and test manually
7. ⏸️ Commit and push

## Next Steps When Resuming

1. Read `src/main.cpp` to understand current structure and find insertion points
2. Implement ViewMode enum and state tracking
3. Add `#` key handler
4. Implement `render_table_view()` function
5. Modify main render loop to switch between modes
6. Build with: `cd build && make -j4`
7. Test manually: `./build/datapainter --database test.db --table points`
8. Fix integration test I/O issues if needed
9. Run all tests: `ctest --output-on-failure` and `uv run pytest tests/integration/ -v`
10. Commit and push changes
