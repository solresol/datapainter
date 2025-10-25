# Specification Clarifications

This document addresses ambiguities in README.md based on questions raised in chatgpt-questions.md.

## 1. Duplicate Points & Erasure Behavior

**Decision:** Duplicates are allowed. Multiple points can exist at the exact same (x,y) coordinates.

**Erasure with Space key:**
- Pressing Space deletes ALL points that fall under the cursor's screen cell
- No modifier keys needed - space always clears everything in that cell
- This matches the README line 44: "Space erases any points under the cursor"

**Hit-testing:**
- Use exact cell match only
- A point is "under the cursor" if it rounds to the exact same screen cell coordinates as the cursor position
- No tolerance buffer (α) is needed

## 2. Target Column Storage

**Decision:** The target column stores the actual meaning values (from --x-meaning and --o-meaning), NOT the literal characters 'x' and 'o'.

**Example:**
- If `--x-meaning="cat"` and `--o-meaning="dog"`, the target column contains "cat" or "dog"
- The characters 'x' and 'o' are display symbols only, used in the UI
- The metadata table stores the mapping between display symbols and actual meanings

**Display logic:**
- Points with target value matching x_meaning are shown as 'x' on screen
- Points with target value matching o_meaning are shown as 'o' on screen
- When multiple points of the same type overlap: 'X' or 'O'
- When both types overlap: '#'

## 3. Keyboard Mapping Corrections

**README line 52 - TYPO:**
Current text: "Pressing O (shift-o) makes existing o points under the cursor into os."
**Corrected:** "Pressing O (shift-o) makes existing **x points** under the cursor into os."

This creates symmetry:
- `Shift+X`: converts o points → x points
- `Shift+O`: converts x points → o points

**Flip key (g) behavior:**
- When pressing 'g', flip ALL points individually under the cursor
- If there are 2 x-points and 3 o-points under cursor: result is 2 o-points and 3 x-points
- Each point flips independently to its opposite type

**Undo/Redo:**
- Support both undo AND redo
- Redo functionality: after undoing one or more actions, redo becomes available
- Making any new edit clears the redo stack
- Keyboard shortcuts: 'u' for undo (as stated in README)
  - Note: Ctrl+Z/Ctrl+Y support was NOT selected, so stick with single-key shortcuts only

## 4. Viewport Behavior

**Empty table (no data points):**
- If valid ranges are set (via --min-x, --max-x, --min-y, --max-y or in metadata): use those as viewport
- Otherwise: default to (-1, 1, -1, 1) as stated in README line 141

**Table with existing data:**
- Fit viewport to data bounds with 10% padding on each side
- Formula: viewport_min = data_min - 0.1 * (data_max - data_min)
- Formula: viewport_max = data_max + 0.1 * (data_max - data_min)
- If --initial-viewport-range is provided via CLI, that overrides the auto-fit

**Valid ranges enforcement:**
- Valid x/y ranges are HARD CONSTRAINTS
- Attempting to create a point outside the valid ranges is rejected
- The UI should prevent cursor movement or point creation beyond these boundaries
- Out-of-range areas can be shown with '!' wall characters as mentioned in README line 165

## 5. Table View Mode

**Editing capability:**
- Table view (#) is FULLY EDITABLE
- Users can add, edit, and delete rows directly in the three-column table view
- All changes are recorded in the unsaved_changes table, just like viewport edits
- Filter and sort can be applied while editing

**Sync behavior (from README lines 59-62):**
- Switching from viewport → table: filter is set to show points in current viewport range
- Switching from table → viewport: viewport is set to tightly fit all filtered/visible rows

## 6. Save Behavior

**Decision:** Save commits changes and continues running (does NOT exit)

**Save workflow:**
1. Apply all unapplied events from unsaved_changes to the main data table
2. Commit in a single transaction
3. Clear or mark unsaved_changes as applied
4. Return to the TUI - user can continue editing

**Quit workflow:**
- "Quit without saving" button: if unsaved changes exist, confirm; otherwise exit immediately
- Regular quit (without save): same behavior
- Save then quit: two separate operations

## 7. CLI Arguments vs Metadata - Conflict Resolution

**Decision:** If there is ANY discrepancy between CLI arguments and existing metadata, the program must ERROR and refuse to start.

**Examples of conflicts:**
- Metadata says x_axis_name="temperature", CLI says --x-axis-name="pressure" → ERROR
- Metadata says valid_x_min=0, CLI says --min-x=-5 → ERROR
- Table exists with metadata, user provides conflicting --x-meaning → ERROR

**Error message should:**
- Clearly show the conflict: "Metadata has X, you specified Y"
- Suggest resolution: remove the conflicting CLI flag, or use a different table name

**Non-conflicting cases:**
- CLI provides values for a NEW table (no metadata exists yet) → OK, use CLI values
- CLI provides SAME values as metadata → OK, redundant but not contradictory
- CLI omits some values, metadata fills them in → OK, no conflict

## 8. Database Schema

**Decision:** Keep schema minimal for initial version.

**Data table:**
```sql
CREATE TABLE IF NOT EXISTS {table_name} (
  id      INTEGER PRIMARY KEY,
  x       REAL NOT NULL,
  y       REAL NOT NULL,
  target  TEXT NOT NULL
);
CREATE INDEX IF NOT EXISTS {table}_xy      ON {table}(x, y);
CREATE INDEX IF NOT EXISTS {table}_target  ON {table}(target);
```

**Metadata table:**
```sql
CREATE TABLE IF NOT EXISTS metadata (
  table_name        TEXT PRIMARY KEY,
  x_axis_name       TEXT NOT NULL,
  y_axis_name       TEXT NOT NULL,
  target_col_name   TEXT NOT NULL,
  x_meaning         TEXT NOT NULL,
  o_meaning         TEXT NOT NULL,
  valid_x_min       REAL,
  valid_x_max       REAL,
  valid_y_min       REAL,
  valid_y_max       REAL,
  show_zero_bars    INTEGER NOT NULL DEFAULT 0
);
```

**Unsaved changes table:**
```sql
CREATE TABLE IF NOT EXISTS unsaved_changes (
  id            INTEGER PRIMARY KEY,
  table_name    TEXT NOT NULL,
  action        TEXT NOT NULL CHECK (action IN ('insert','delete','update','meta')),
  -- For insert/delete/update actions:
  data_id       INTEGER,  -- the id in the data table (null for insert before save)
  x             REAL,
  y             REAL,
  old_target    TEXT,
  new_target    TEXT,
  -- For meta actions:
  meta_field    TEXT,
  old_value     TEXT,
  new_value     TEXT
);
CREATE INDEX IF NOT EXISTS uc_table ON unsaved_changes(table_name, id);
```

**Deferred features (not in initial version):**
- No `created_at` timestamps
- No `session_id` tracking
- No `weight` column
- No `applied` flag (delete from unsaved_changes on commit instead)

## 9. Zero-Axis Bars (Cartesian Axes)

**Decision:** Zero bars are controlled by the --show-zero-bars flag.

**Behavior:**
- Do NOT automatically show zero axes just because zero is in the viewport
- Only show if:
  - --show-zero-bars is provided via CLI, OR
  - show_zero_bars=1 in the metadata table for this table
- Default is OFF (show_zero_bars=0)

**Visual representation:**
- When enabled and x=0 is in viewport: draw vertical line at x=0
- When enabled and y=0 is in viewport: draw horizontal line at y=0
- Use a distinct character (e.g., '|' for y-axis, '-' or '─' for x-axis) differentiated from grid lines

## 10. Additional Clarifications

**Zoom centering (confirmed from README line 162):**
- Zoom in/out is centered on the current cursor coordinates
- Zoom in: each screen cell represents half the data range it did before
- Zoom out: each screen cell represents double the data range

**Pan behavior (README lines 39-40):**
- Arrow keys move cursor within viewport
- If cursor would go beyond edge AND there is more valid range available: shift viewport
- If first column is already at the pre-defined x minimum: nothing happens
- Pan shift amount: reasonable (to be determined during implementation, suggest 25% of viewport)

**Tabular view filter:**
- Filter syntax: SQL WHERE clause, e.g., `x BETWEEN ? AND ? AND y BETWEEN ? AND ?`
- Default filter when entering table mode: current viewport bounds
- Filter can be edited by user while in table mode

**Study mode (README lines 168-183):**
- Purpose: import an existing three-column table and configure it for datapainter
- Validates: exactly 3 columns, 2 are REAL, one has exactly 2 distinct values, no nulls
- Prompts for: which value is 'x', which is 'o', which columns are x-axis and y-axis
- Suggests min/max based on data, allows override
- Creates metadata entry
- Errors if metadata already exists for that table

---

## Summary of ChatGPT's Key Points - Assessment

ChatGPT raised 16 categories of concerns. Here's the status:

✅ **Addressed:**
1. Data model and schemas - minimal schema defined
2. Floating-point hit-testing - exact cell match clarified
3. Undo/redo - redo supported, single-key shortcuts
4. Keyboard model - typo fixed, flip behavior defined
5. Table view semantics - fully editable, filter/sync rules clear
6. CLI precedence - conflicts are errors
7. Viewport behavior - empty vs data cases defined
8. Save behavior - commit and continue
9. Valid ranges - hard constraints
10. Zero bars - flag-controlled

⏸️ **Deferred or Simplified:**
- Tolerance-based hit testing (using exact cell match instead)
- Schema extras like timestamps, session_id, weight (keeping minimal)
- Ctrl+Z/Ctrl+Y shortcuts (single-key only for now)

📋 **For later specification:**
- Performance targets (1M rows, 30 FPS, etc.) - implementation detail
- Concurrency and crash recovery - can be added after core functionality
- CSV export format details - defer to implementation
- Terminal/platform specifics - defer to implementation
- Accessibility/color - defer to implementation

The critical ambiguities that could cause confusion during development are now resolved.
