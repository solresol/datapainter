# datapainter

TUI for creating two-dimensional datasets and saving them into a SQLite database

# Installation

## APT Repository (Ubuntu/Debian)

Add the DataPainter repository and install:

```bash
# Add the GPG key and verify the fingerprint
wget -qO- https://packages.industrial-linguistics.com/datapainter/PUBLIC.KEY | \
  gpg --import --import-options show-only

# If the fingerprint looks correct, import it to your system
wget -qO- https://packages.industrial-linguistics.com/datapainter/PUBLIC.KEY | \
  sudo gpg --dearmor -o /usr/share/keyrings/datapainter-archive-keyring.gpg

# Add the repository
echo "deb [signed-by=/usr/share/keyrings/datapainter-archive-keyring.gpg] https://packages.industrial-linguistics.com/datapainter stable main" | \
  sudo tee /etc/apt/sources.list.d/datapainter.list

# Update package list
sudo apt-get update

# Install DataPainter
sudo apt-get install datapainter
```

The repository is GPG-signed for security. Verify the key fingerprint matches the official key for `packages@industrial-linguistics.com` before importing.

## Pre-built Binaries

Download the latest release for your platform:
- **Linux tarball**: https://packages.industrial-linguistics.com/datapainter/datapainter-linux-latest.tar.gz
- **Debian package**: https://packages.industrial-linguistics.com/datapainter/pool/main/
- **All releases**: https://github.com/solresol/datapainter/releases

## Building from Source

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y cmake g++ libsqlite3-dev libncurses-dev

git clone https://github.com/solresol/datapainter.git
cd datapainter
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

### macOS
```bash
brew install cmake sqlite3

git clone https://github.com/solresol/datapainter.git
cd datapainter
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

For more information about deployment infrastructure, see [DEPLOYMENT.md](DEPLOYMENT.md).

# UI Design

You can cursor around and put an X or an O anywhere on a 2D grid which gets saved to a 
SQLite database

On screen you have something that looks like this:


```
[Database filename]    [Database tablename] [Total points=___, x=__,o=__,]
[Target column name]   x = [_______]  o = [_________]
 5  +--------------------------------------------------------------------------------------+
    |                                                                                      |
 4 --                 x                                       o                            |
    |                                                                                      |
 3 --                                                                                      |
    |                 x                                                                    |
 2 --                                               #                                      |
    |                                                                                      |
 1 --                             x                                                        |
    |                                                                                      |
 0 --                                                                                      |
    |                                                                                      |
-1 --                                                                                      |
    |                                                                                      |
-2  +----|----|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|+
[Y-axis column name] 
   -10  -9   -8  -7  -6  -5  -4  -3  -2  -1   0   1   2   3   4   5   6   7   8   9  10  11
                                      [X-axis column name]
    + zoom in    - zoom out    = full viewport   cursor pos = (2.25,1.5) 
Valid x-range [   ] to [    ]. Valid y-range [   ] to [    ]  [Unsaved: 3]
                                   [Tabular view]  [Undo]     [Quit]  [Save]
```

You move around the edit area with cursor arrows. Arrow keys move cursor within viewport.
If cursor would go beyond edge AND there is more valid range available: shift viewport.
If the first column is already at the pre-defined x minimum, nothing happens: otherwise
it shifts the viewport.

TAB gets you to the other places (e.g. valid x-ranges, quit, undo and save. u, q and s

**Note:** When attempting to quit (press 'q' or ESC) with unsaved changes, a confirmation dialog
appears asking "Save changes before quitting? (y/n/cancel)". This prevents accidental data loss.
are short cuts.

Space erases any points under the cursor. Multiple points can exist at the exact same (x,y)
coordinates (duplicates are allowed). A point is considered "under the cursor" if it rounds to
the exact same screen cell coordinates as the cursor position. Points on screen can only be an
approximation of the values in the dataset. If there are several x points under the cursor,
it is shown as an X. If there are several o points, they are shown as an O. If there are
both x and o points, it is shown as a #.

Pressing x creates an x point. Pressing X (shift-X) makes existing o points under the 
cursor into xs.

Pressing o creates an o point. Pressing O (shift-o) makes existing x points under the
cursor into os.

Pressing g flips all points individually under the cursor: if it was an o, it's now an x and
vice versa. Each point flips independently to its opposite type.

Pressing k dumps the entire screen to stdout (useful for debugging and testing).
Pressing K (shift-K) dumps only the edit area contents to stdout.

Pressing the # key switches the system into tabular view mode, which is a three
column TUI. (The x column, the y column and the target column). The table view is fully
editable - users can add, edit, and delete rows directly. All changes are recorded in the
unsaved_changes table, just like viewport edits. Filter and sort can be applied while editing.

By default it has a filter on which limits the points shown. When switching into table mode,
the filter is set to whatever the viewport was showing. When switching from tabular mode to
viewport mode, it sets the viewport to tightly fit all filtered/visible rows.

**Filter syntax:** SQL WHERE clause, e.g., `x BETWEEN ? AND ? AND y BETWEEN ? AND ?`. Default
filter when entering table mode is the current viewport bounds. Filter can be edited by user
while in table mode.


# CLI arguments

It can be launched with or without CLI arguments. The args are:
  - --x-axis-name
  - --y-axis-name
  - --database
  - --table
  - --target-column-name
  - --x-meaning
  - --o-meaning
  - --min-x
  - --max-x
  - --min-y
  - --max-y
  

If it is just given the database filename, it looks for a tablename called `metadata`. If it finds
it, then it will have rows containing table/axis name/meanings. It provides a dialog box to select
among them (with an option to copy to a new table, create a blank table, and delete or rename a table).
If --database and --table are given, then it looks for metadata to fill in the rest of
the information. If there is no metadata available for a new table, it prompts for the
x-axis name, y-axis name, target name, etc.

**CLI Conflict Resolution:** If there is ANY discrepancy between CLI arguments and existing metadata,
the program will ERROR and refuse to start. The error message will clearly show the conflict and
suggest resolution (remove the conflicting CLI flag, or use a different table name).

**Target Column Storage:** The target column stores the actual meaning values (from --x-meaning and
--o-meaning), NOT the literal characters 'x' and 'o'. For example, if --x-meaning="cat" and
--o-meaning="dog", the target column contains "cat" or "dog". The characters 'x' and 'o' are display
symbols only, used in the UI.

If there is no metadata table, it creates it. If there is no table called `unsaved_changes` it creates
that too.

If it has to create a data table, it makes sure that there are indexes on the x-axis column, the y-axis
column and the target column.

## Database Schema

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

# Non-interactive mode

These don't start up the TUI.
  - --create-table (+ args for the metadata)
  - --rename-table
  - --copy-table
  - --delete-table
  - --list-tables
  - --show-metadata
  - --add-point
  - --delete-point
  - --to-csv
  - --key-stroke-at-point (x,y) [key] -- pretend that we've pressed that key at those coordinates

Then there are some args which will be useful for testing. They start the TUI
  - --dump-screen = non-interactive mode, usually paired with an action like key-stroke-at-point
  - --dump-edit-area-contents  = ditto, but just show the edit area, not the whole screen

# Automated TUI Testing with Keystroke Playback

For integration and end-to-end testing of the TUI, datapainter supports automated keystroke playback:

**--keystroke-file <filename>**
  - Starts the TUI in automated test mode
  - Reads a sequence of keystrokes from the specified file
  - Each time the TUI is ready for input, processes the next keystroke from the file
  - Useful for scripted testing of interactive workflows

**Screen dump keystrokes (available in both interactive and automated modes):**
  - `k` - Dump the entire screen to stdout
  - `K` - Dump only the edit area contents to stdout

**Keystroke file format:**
  - One keystroke per line
  - Special keys represented as text:
    - `<up>`, `<down>`, `<left>`, `<right>` - Arrow keys
    - `<space>` - Space bar
    - `<tab>` - Tab key
    - `<enter>` - Enter/Return key
    - `<esc>` - Escape key
  - Regular keys: just the character (e.g., `x`, `o`, `+`, `-`, `=`)
  - Test commands: `k` for screen dump, `K` for edit area dump
  - Comments: lines starting with `#` are ignored
  - Empty lines are ignored

**Example keystroke file:**
```
# Move cursor and add a point
<right>
<right>
<down>
x
# Dump the screen to verify
k
# Save and quit
s
q
```

**Usage example:**
```bash
# Create test keystroke sequence
cat > test_keystrokes.txt <<EOF
x
<right>
o
k
s
q
EOF

# Run automated test
datapainter --database test.db --table data \
  --keystroke-file test_keystrokes.txt > test_output.txt

# Verify output
grep "x" test_output.txt
grep "o" test_output.txt
```

This enables comprehensive automated testing of TUI workflows including navigation,
point editing, zoom/pan operations, view switching, and save operations.


# User interface options

  - --show-zero-bars = Controls display of Cartesian axes (zero bars). Do NOT automatically show
    zero axes just because zero is in the viewport. Only show if --show-zero-bars is provided via
    CLI OR show_zero_bars=1 in the metadata table. Default is OFF (show_zero_bars=0). When enabled
    and x=0 is in viewport: draw vertical line at x=0. When enabled and y=0 is in viewport: draw
    horizontal line at y=0. Use distinct characters (e.g., '|' for y-axis, '-' or '─' for x-axis)
    differentiated from grid lines.
  - --override-screen-height and --override-screen-width -- by default it detects the terminal size
  (and rescales if the terminal is resized) This is useful for testing as well as screen ergonomics. If the
  override is bigger than the screen and we aren't just dumping output, exit with an error message
  - --start-tabular = start with the tabular view instead

# Undo/Redo mechanism

CLI arguments
 - --clear-undo-log (tablename)
 - --clear-all-undo-log (clears all tables' unsaved changes)
 - --commit-unsaved-changes (tablename)
 - --list-unsaved-changes

The data displayed on screen is a combination of whatever was in the relevant data table, and whatever is
stored in the `unsaved_changes` table. Point changes get added and deleted by appending to the `unsaved_changes`
table. Pressing the save button commits those changes into the relevant table.

The `unsaved_changes` table also tracks changes to the meaning of x and o values in the target column,
and the min and max x and y values.

**Undo and Redo:** The 'u' key undoes the last action. After undoing one or more actions, redo becomes
available. Making any new edit clears the redo stack. Note: Only single-key shortcuts are supported
(u for undo), not Ctrl+Z/Ctrl+Y.

**Save Behavior:** Saving commits all unapplied events from unsaved_changes to the main data table
in a single transaction, then clears or marks unsaved_changes as applied. The program continues
running after save - it does NOT exit. The user can continue editing after saving.

# Zoom

The default viewport behavior depends on whether the table is empty or has data:

**Empty table (no data points):**
- If valid ranges are set (via --min-x, --max-x, --min-y, --max-y or in metadata): use those as viewport
- Otherwise: default to (-1, 1, -1, 1)

**Table with existing data:**
- Fit viewport to data bounds with 10% padding on each side
- Formula: viewport_min = data_min - 0.1 * (data_max - data_min)
- Formula: viewport_max = data_max + 0.1 * (data_max - data_min)
- If --initial-viewport-range (xlow, xhigh, ylow, yhigh) is provided via CLI, that overrides the auto-fit

**Valid ranges enforcement:**
- Valid x/y ranges (--min-x, --max-x, --min-y, --max-y) are HARD CONSTRAINTS
- Attempting to create a point outside the valid ranges is rejected
- The UI should prevent cursor movement or point creation beyond these boundaries
- Out-of-range areas can be shown with '!' wall characters

There is also an option - --zoom-in / --zoom-out ... just for testing, and used in conjunction with the 
   CLI --list-x-axis-marks and --list-y-axis-marks ... this causes a zoom event to happen and we can confirm 
   that the resulting axis marks for a screen that size would be handled correctly.

The internal zoom state consists of:
 - the minimum and maximum numbers (in the x and y range) that we have to keep visible
 - the decimal place that we are showing major tick marks for. 
     0 means that we are showing integer major tick marks.
     1 means that we are showing tick marks for every 10
     -1 means that we are showing tick marks for every 0.1
     It is simply the base 10 log of the range in that axis
 - the number of spaces on screen in the viewport in each axis
 - how many spaces on screen will be needed for each tick mark (not counting the space after it)
 - (therefore) how many characters on screen the major tick marks can be visible on screen
 - (therefore) whether there is room in the gaps between major tick marks for half minor tick marks 
   (e.g. at 0.05 when the tick marks are at 0.1 steps) and tenth minor tick marks (at 0.01 when the 
   major ticks are at 0.1)

When the user zooms in, each space on screen halves in data space; when they zoom out, it's
double. Zoom in/out is centered on the current cursor coordinates (not the screen center).

There are situations where there will be spaces on screen below the minimum and maximum x values.
These are shown by a wall of ! characters.

# Study mode

If invoked with --database, --table and --study it will:

**Purpose:** Import an existing three-column table and configure it for datapainter.

**Validation (exits with error if any fail):**
- Check if metadata already exists for that table → ERROR if exists
- Validate exactly 3 columns exist
- Validate 2 columns are REAL type
- Validate the third column has exactly 2 distinct values
- Validate no nulls in any column

**Interactive configuration:**
- Prompt: Which distinct value should be displayed as 'x' and which as 'o'?
- Prompt: Which column is the x-axis and which is the y-axis?
- Suggest: minimum x and y and maximum x and y based on the dataset
- Allow: user to override suggested min/max values

**Result:** Creates metadata entry for the table, making it ready for use with datapainter.

# Random init

Options for random init:
 - --random-count = how many points to create
 - --random-target = what target value to use for those random points
 - --mean-x, --mean-y where to centre the points
 - --normal-x and --normal-y (together with --std-x, --std-y) draw from a normal distribution
 - --uniform-x and --uniform-y (together with --range-x, --range-y) draw from a uniform distribution of width `range`

Note that you would normally run random init twice, once for the "x" and once for the "o" outputs.

# Implementation Details & Clarifications

## Concurrency & SQLite Configuration
**Decision:** Simple single-process mode only
- No SQLite WAL mode required
- No busy_timeout configuration needed
- Assume single process access to each database
- No multi-process conflict detection or handling
- Simpler implementation without concurrency complexity

## Color & Accessibility
**Decision:** Monochrome mode only (no colors)
- No color support in initial version
- Use only ASCII characters for rendering
- No --no-colour flag needed
- No NO_COLOR environment variable support
- Keep implementation simple with monochrome output

## Security & File Permissions
**Decision:** Warn but allow world-writable files
- Check file permissions on database open
- If database file is world-writable, display a warning message
- Allow operation to proceed despite warning
- No --i-know-what-i-am-doing flag required
- User is responsible for file security management

## Performance Requirements
**Decision:** Best effort, no specific targets
- Optimize as needed during development
- No hard performance requirements (FPS, latency, save time)
- Profile and improve performance issues as they arise
- No baseline targets for 1M row tables

## Help System
**Decision:** '?' key shows keyboard shortcuts and current state
- Pressing '?' brings up an in-app help cheatsheet overlay
- Display all keyboard shortcuts
- Show current zoom level
- Show pan step percentage
- Show current hit-test behavior
- Dismiss help overlay to return to normal operation

## Exit Codes
**Decision:** Detailed exit codes for different error conditions

Standard exit codes:
- **0** - Success
- **2** - Invalid command-line arguments
- **64** - Screen too small (when --override-screen-height/width exceeds terminal size)
- **65** - Database I/O error
- **66** - Database lock timeout
- **67** - CSV write error

Non-interactive commands return machine-readable messages on stdout; errors on stderr.

## CSV Export Format (--to-csv)
**Decision:** Basic CSV output, no special options
- Simple CSV format: headers + data rows
- Rows ordered by id (insertion order)
- UTF-8 encoding
- No optional BOM support
- No --filter support for selective export
- Format: three columns (x, y, target) with header row

## Terminal Resize Handling (SIGWINCH)
**Decision:** Handle SIGWINCH with small-screen fallback
- Detect terminal resize events (SIGWINCH)
- Re-render UI at new terminal size
- If screen becomes too small to render header + 3 rows:
  - Pause canvas rendering
  - Show a status line prompting user to enlarge terminal
- Resume normal rendering when adequate size restored

## Table Name Validation
**Decision:** Restrict to [A-Za-z0-9_]+ only
- Table names must match regex: `[A-Za-z0-9_]+`
- Alphanumeric characters and underscores only
- Reject table names with spaces, special characters, or other symbols
- Display clear error message for invalid table names
- Non-interactive mode: exit with code 2
- Interactive mode: show inline error and prompt again

## Decimal Point Display (Locale)
**Decision:** Use default C++ formatting (printf/iostream)
- Let standard library decide decimal separator
- Use standard printf or iostream formatting for axis labels
- No explicit locale override
- No forced '.' period regardless of locale

## Wide Character Handling
**Decision:** Block wide characters entirely
- Do not support emoji or multi-byte wide characters
- Strip or reject wide characters from input
- Ensure terminal alignment with fixed-width ASCII only
- Prevents rendering issues with variable-width characters

## Validation Rules Summary

### Range Validation
- Valid ranges: `min <= max` required
- Invalid ranges:
  - Non-interactive: exit with code 2
  - Interactive: show inline error message

### CLI Conflict Resolution
Already specified above in CLI arguments section.

## Implementation Notes

### Tick Algorithm
- Let `range = x_high - x_low`
- Choose `step = 10^k * {1,2,5}` to prevent label collision
- Minor ticks appear if ≥ 3 characters between majors
- Tenth ticks if ≥ 6 characters between majors
- Scientific notation for `|k| ≥ 4`

### Focus Order (Tab key)
Tab cycles through header fields left-to-right:
1. Database name
2. Table name
3. Target name
4. X/O meanings
5. Valid ranges
6. Action buttons (Tabular view, Undo, Quit, Save)
7. Then into viewport

Enter key activates focused button or confirms edited field.

### Parameterized SQL
- Always use parameterized queries for dynamic values
- For table names: whitelist against metadata.table_name
- Prevents SQL injection vulnerabilities

## Test Coverage Notes
Acceptance test scenarios are implementation details and not specified as requirements. Developers should design appropriate tests during development following TDD practices as specified in CLAUDE.md.

# Future Features & Optional Implementation Details

This section documents potential future enhancements and optional implementation details that are not currently specified as requirements.

## Table View Sort Functionality

Table view is described above. Optional enhancement:

* **Sort:** default by `id` (insertion order); toggle sort by any column.

## Undo/Redo Granularity & Crash Recovery

Undo/redo is described above. Optional enhancements:

* **Undo/redo granularity:** one key action = one event (or batch when holding a key repeats; batch by 50ms coalescing).
* **Crash recovery:** on startup, detect unapplied events for this table; prompt: "Commit, discard, or review."

## Multi-Process Concurrency

README.md currently specifies simple single-process mode only. Future multi-process support could include:

* **WAL mode ON** per database; **busy_timeout** ≥ 5s.
* **Multiple processes:** support read-share, single writer. If two TUIs open the same table, only the process that "Saves" first succeeds; the other must rebase (replay) its local `unsaved_changes` against the new table state.

## CSV Export Enhancements

README.md currently specifies basic CSV export. Future enhancements could include:

* **RFC 4180 compliance:** full adherence to CSV specification
* **Optional BOM support:** allow UTF-8 BOM when needed
* **Filter support:** optional `--filter "x>=… AND …"` for selective export

## Test Hook Details

Non-interactive mode test hooks are described above. Optional enhancement:

* **`--dump-screen` / `--dump-edit-area-contents`:** could include a first line comment with viewport bounds and cursor pos for regression testing.

## Performance Baseline Targets

README.md currently specifies "best effort, no specific targets." Future performance targets could include:

* 1M rows table: pan/zoom keeps UI > 30 FPS for viewport rendering (because rendering is density-based, not point-based).
* Inserts/deletes in the viewport: amortised < 20 ms per action; Save (100k unapplied events): < 2 s with WAL.

(These are reasonable baselines; tune after profiling.)

# Documentation

This README.md, CLAUDE.md talks about coding standards and styles.

man pages are in ....

# Packages

Github actions compile this for Windows, MacOS, Linux and Haiku. Packaging scripts are in scripts/
MacOS is only built on releases. 
After being built, they are copied into 
  datapainter@merah.cassia.ifost.org.au:/var/www/vhosts/packages.industrial-linguistics.com/htdocs/datapainter
The SSH key DEPLOYMENT_SSH_KEY allows access as that user.
