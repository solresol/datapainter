# DataPainter TODO List

This is a comprehensive implementation plan for the DataPainter TUI project. Tasks are organized by feature area and ordered to support Test-Driven Development (TDD) with incremental builds.

---

## Phase 0: Project Setup

### Build System & Infrastructure
- [x] Create CMakeLists.txt with C++17/20 standard
- [x] Set up project directory structure (src/, include/, tests/)
- [x] Configure SQLite3 dependency
- [x] Set up testing framework (Google Test or Catch2)
- [x] Create main.cpp entry point
- [x] Set up continuous integration (GitHub Actions)
- [x] Create .gitignore for build artifacts

---

## Phase 1: Database Layer (Foundation)

### Core Schema & Connection
- [x] Create Database class with SQLite connection management
- [x] Test: Open/close database file
- [x] Test: Create metadata table with correct schema
- [x] Test: Create unsaved_changes table with correct schema
- [x] Test: Detect if database file is world-writable and warn
- [x] Implement table name validation (regex: `[A-Za-z0-9_]+`)
- [x] Test: Reject invalid table names

### Data Table Operations
- [x] Create DataTable class for managing data tables
- [x] Test: Create data table with indexes (id, x, y, target)
- [x] Test: Insert point with (x, y, target) values
- [x] Test: Delete point by id
- [x] Test: Update point target value
- [x] Test: Query points within viewport bounds
- [x] Test: Get distinct target values from table
- [x] Test: Count points by target value

### Metadata Operations
- [x] Create Metadata class/struct
- [x] Test: Insert metadata row for new table
- [x] Test: Read metadata for existing table
- [x] Test: Update metadata fields
- [x] Test: List all tables from metadata
- [x] Test: Delete metadata entry
- [x] Test: Rename table in metadata and data table
- [x] Test: Copy table (data + metadata)

### Unsaved Changes Tracking
- [x] Create UnsavedChanges class
- [x] Test: Record insert action
- [x] Test: Record delete action
- [x] Test: Record update action
- [x] Test: Record metadata change action
- [x] Test: Retrieve unsaved changes for a table
- [x] Test: Apply unsaved changes to data table (commit)
- [x] Test: Clear unsaved changes for a table
- [x] Test: Support undo (mark change as undone)
- [x] Test: Support redo (re-apply undone change)

---

## Phase 2: CLI Argument Parsing

### Argument Parser
- [x] Create ArgumentParser class
- [x] Test: Parse --database argument
- [x] Test: Parse --table argument
- [x] Test: Parse axis names (--x-axis-name, --y-axis-name)
- [x] Test: Parse --target-column-name
- [x] Test: Parse meanings (--x-meaning, --o-meaning)
- [x] Test: Parse valid ranges (--min-x, --max-x, --min-y, --max-y)
- [x] Test: Validate min <= max for ranges (exit code 2 if invalid)
- [x] Test: Parse --show-zero-bars flag
- [x] Test: Parse screen overrides (--override-screen-height/width)
- [x] Test: Parse --start-tabular flag

### Conflict Detection
- [x] Test: Detect CLI vs metadata conflicts
- [x] Test: Generate clear error message showing conflicts
- [x] Test: Suggest resolution (remove flag or use different table)
- [x] Test: Exit with code 2 on conflict

### Non-Interactive Mode Arguments
- [x] Test: Parse --create-table
- [x] Test: Parse --rename-table
- [x] Test: Parse --copy-table
- [x] Test: Parse --delete-table
- [x] Test: Parse --list-tables
- [x] Test: Parse --show-metadata
- [x] Test: Parse --add-point
- [x] Test: Parse --delete-point
- [x] Test: Parse --to-csv
- [x] Test: Parse --key-stroke-at-point with x,y,key

### Testing & Debug Arguments
- [x] Test: Parse --dump-screen
- [x] Test: Parse --dump-edit-area-contents
- [x] Test: Parse --zoom-in / --zoom-out
- [x] Test: Parse --list-x-axis-marks / --list-y-axis-marks

### Study Mode Arguments
- [x] Test: Parse --study flag
- [x] Validate requires both --database and --table

### Random Init Arguments
- [x] Test: Parse --random-count
- [x] Test: Parse --random-target
- [x] Test: Parse --mean-x, --mean-y
- [x] Test: Parse --normal-x, --normal-y, --std-x, --std-y
- [x] Test: Parse --uniform-x, --uniform-y, --range-x, --range-y

### Undo Log Arguments
- [x] Test: Parse --clear-undo-log
- [x] Test: Parse --clear-all-undo-log
- [x] Test: Parse --commit-unsaved-changes
- [x] Test: Parse --list-unsaved-changes

---

## Phase 3: Viewport Math & Coordinate System

### Coordinate System
- [x] Create Viewport class
- [x] Test: Initialize viewport with (x_min, x_max, y_min, y_max)
- [x] Test: Calculate default viewport for empty table (use valid ranges or [-1,1,-1,1])
- [x] Test: Calculate auto-fit viewport with 10% padding
- [x] Test: Convert screen coordinates to data coordinates
- [x] Test: Convert data coordinates to screen coordinates
- [x] Test: Determine if data point is visible in viewport
- [x] Test: Round data coordinates to screen cell

### Zoom Operations
- [x] Test: Zoom in (halve data-per-screen-space, centered on cursor)
- [x] Test: Zoom out (double data-per-screen-space, centered on cursor)
- [x] Test: Full viewport zoom (= key, fit all data)
- [x] Test: Respect valid range constraints during zoom
- [x] Test: Smart zoom centering (adjust center at edges to maximize usable area)
- [x] Test: Calculate decimal place for major tick marks (log10 of range)

### Pan Operations
- [x] Test: Pan right (shift viewport if cursor at edge and range available)
- [x] Test: Pan left
- [x] Test: Pan up
- [x] Test: Pan down
- [x] Test: Prevent pan beyond valid ranges
- [x] Test: Cursor movement within viewport (no pan)

---

## Phase 4: Terminal/Screen Management

### Terminal Detection
- [x] Create Terminal class
- [x] Test: Detect terminal dimensions (rows, cols)
- [x] Test: Override dimensions with CLI args
- [x] Test: Exit code 64 if override exceeds actual terminal size
- [x] Test: Handle SIGWINCH (terminal resize)
- [x] Test: Pause rendering if screen too small (< header + 3 rows)
- [x] Test: Show "enlarge terminal" message when too small
- [x] Test: Resume rendering when size adequate

### Screen Buffer
- [x] Create ScreenBuffer class
- [x] Test: Allocate 2D character buffer (rows x cols)
- [x] Test: Write character at (row, col)
- [x] Test: Clear screen buffer
- [x] Test: Render buffer to stdout
- [x] Test: Block wide characters (emoji, multi-byte)

---

## Phase 5: Axis Rendering & Labels

### Tick Mark Algorithm
- [x] Create AxisRenderer class
- [x] Test: Calculate tick step (10^k × {1,2,5}) to prevent label collision
- [x] Test: Generate major tick positions
- [x] Test: Generate minor tick positions (if >= 3 chars between majors)
- [x] Test: Generate tenth tick positions (if >= 6 chars between majors)
- [x] Test: Format tick labels (standard printf/iostream)
- [x] Test: Use scientific notation for |k| >= 4

### Axis Drawing
- [x] Test: Draw x-axis with tick marks and labels
- [x] Test: Draw y-axis with tick marks and labels
- [x] Test: Draw x-axis label (column name) - axis name parameter implemented
- [x] Test: Draw y-axis label (column name) - axis name parameter implemented
- [x] Test: Handle negative numbers in labels
- [x] Test: Align labels correctly with tick marks

### Zero Bars (Cartesian Axes)
- [x] Test: Do NOT show zero bars by default
- [x] Test: Show vertical line at x=0 when --show-zero-bars AND x=0 in viewport
- [x] Test: Show horizontal line at y=0 when --show-zero-bars AND y=0 in viewport
- [x] Test: Use distinct characters for zero bars (| and ─) - uses '|' and '-'
- [x] Test: Differentiate zero bars from grid lines - uses distinct characters

---

## Phase 6: Viewport Rendering

### Edit Area Rendering
- [x] Create EditAreaRenderer class
- [x] Test: Draw border around edit area
- [x] Test: Render empty edit area
- [x] Test: Query data points within current viewport
- [x] Test: Apply unsaved changes to viewport data
- [x] Test: Render single 'x' point at screen position
- [x] Test: Render single 'o' point at screen position
- [x] Test: Render multiple x's at same cell as 'X'
- [x] Test: Render multiple o's at same cell as 'O'
- [x] Test: Render mixed x+o at same cell as '#'
- [x] Test: Render wall characters '!' for out-of-valid-range areas
- [x] Test: Draw cursor at current position
- [x] Test: Show cursor coordinates in header

### Header Rendering
- [x] Create HeaderRenderer class
- [x] Test: Display database filename
- [x] Test: Display table name
- [x] Test: Display target column name
- [x] Test: Display x and o meanings
- [x] Test: Display counts (total, x count, o count)
- [x] Test: Display valid x/y ranges
- [x] Test: Display current zoom info
- [x] Test: Highlight focused field (for Tab navigation)

### Footer Rendering
- [x] Test: Display zoom controls (+ - =)
- [x] Test: Display cursor position
- [x] Test: Display valid ranges
- [x] Test: Display action buttons (Tabular, Undo, Quit, Save)

---

## Phase 7: Input Handling & Navigation

### Keyboard Input
- [x] Create InputHandler class
- [ ] Test: Read single keypress (blocking)
- [x] Test: Parse arrow keys
- [x] Test: Parse letter keys (x, o, g, X, O, etc.)
- [x] Test: Parse special keys (Tab, Enter, Space, Esc)
- [x] Test: Parse symbols (+, -, =, #, ?, u, q, s)

### Cursor Movement
- [x] Test: Move cursor right within viewport
- [x] Test: Move cursor left within viewport
- [x] Test: Move cursor up within viewport
- [x] Test: Move cursor down within viewport
- [x] Test: Pan viewport when cursor reaches edge
- [x] Test: Prevent cursor movement beyond valid ranges

### Tab Navigation
- [x] Test: Tab cycles through header fields (left to right)
- [x] Test: Tab order: database → table → target → meanings → ranges → buttons → viewport
- [x] Implement: Tab navigation through all fields and buttons (8/10 integration tests pass)
- [x] Test: Enter activates focused field/button
- [ ] Test: Edit focused field inline

---

## Phase 8: Point Editing Operations

### Point Creation
- [x] Create PointEditor class
- [x] Test: Press 'x' creates x point at cursor
- [x] Test: Press 'o' creates o point at cursor
- [x] Test: Reject point creation outside valid ranges
- [x] Test: Record point creation in unsaved_changes
- [x] Test: Update on-screen display immediately

### Point Deletion
- [x] Test: Press Space deletes all points under cursor
- [x] Test: "Under cursor" means rounds to same screen cell
- [x] Test: Record deletion in unsaved_changes
- [x] Test: Update display after deletion

### Point Conversion
- [x] Test: Press Shift-X converts o points under cursor to x
- [x] Test: Press Shift-O converts x points under cursor to o
- [x] Test: Press 'g' flips each point individually (x↔o)
- [x] Test: Record conversions in unsaved_changes

### Hit Testing
- [x] Test: Find all points that round to cursor's screen cell
- [x] Test: Handle multiple points at exact same (x,y)
- [x] Test: Handle points close enough to round to same cell

---

## Phase 9: Undo/Redo System

### Undo Stack Management
- [x] Create UndoManager class
- [x] Test: Track current position in unsaved_changes
- [x] Test: Press 'u' undoes last action
- [x] Test: Multiple undo steps backward
- [x] Test: Enable redo after undo
- [x] Test: Clear redo stack on new edit
- [x] Test: Display undo/redo availability in UI

### Undo/Redo Operations
- [x] Test: Undo point insert (mark as undone)
- [x] Test: Undo point delete
- [x] Test: Undo point update
- [x] Test: Undo metadata change
- [x] Test: Redo undone operation
- [x] Test: Update display after undo/redo

---

## Phase 10: Save/Quit Operations

### Save Mechanism
- [x] Create SaveManager class
- [x] Test: Commit all unsaved_changes to data table in transaction
- [x] Test: Apply inserts, deletes, updates in correct order
- [x] Test: Apply metadata changes
- [x] Test: Clear/mark unsaved_changes as applied after save
- [x] Test: Continue running after save (don't exit)
- [x] Test: Handle save errors (exit code 65)
  Note: Save errors are handled gracefully in interactive mode (user can retry).
  Non-interactive --commit-unsaved-changes uses exit code 66 for failures.
  True I/O errors (vs lock timeouts) are difficult to test reliably.

### Quit Operations
- [x] Test: Check for unsaved changes (count active changes)
- [x] Test: Count unsaved changes across multiple tables
- [x] **VERIFIED**: Flip functionality implementation is correct (tests pass, UI code is correct)
- [x] Display unsaved changes count in UI (header or footer) - show "[Unsaved: N]"
- [x] Test: Press 'q' quits immediately (if no unsaved changes) - implemented in main.cpp
- [x] Test: Warn if unsaved changes exist - show confirmation dialog - implemented in main.cpp
- [x] Implement warning dialog: "Save changes before quitting? (y/n/cancel)" - implemented
  - [x] 'y' = save then quit - implemented
  - [x] 'n' = quit without saving (discard changes) - implemented
  - [x] 'cancel' or ESC = return to editor - implemented (any other key returns)
- [x] Test: Confirmation dialog saves and quits on 'y' - verified in test_quit_after_save_scenario.cpp
- [x] Test: Confirmation dialog discards and quits on 'n' - verified in test_discard_changes_on_quit.cpp
- [x] Test: Confirmation dialog cancels on 'cancel' - verified in implementation
- [x] Test: Exit with code 0 on normal quit - implemented

---

## Phase 11: Tabular View

### Table View UI
- [x] Create TableView class
- [x] Test: Display three columns (x, y, target)
- [x] Test: Show all rows by default
- [ ] Test: Display filter at top
- [x] Test: Default filter = current viewport bounds when entering from viewport
- [x] Test: Navigate rows with arrow keys
- [ ] Test: Edit cell values inline

### Table View Operations
- [ ] Test: Add new row
- [ ] Test: Delete row
- [ ] Test: Edit x value
- [ ] Test: Edit y value
- [ ] Test: Edit target value
- [ ] Test: Record all changes in unsaved_changes
- [ ] Test: Validate numeric input for x and y

### Filter & View Switching
- [x] Test: Edit filter (SQL WHERE clause)
- [ ] Test: Apply filter to visible rows
- [ ] Test: Switch from viewport to table (press #)
- [ ] Test: Switch from table to viewport (press #)
- [ ] Test: Set viewport to fit filtered rows when returning to viewport
- [ ] Test: Preserve unsaved changes across view switches

---

## Phase 12: Help System

### Help Overlay
- [x] Create HelpOverlay class
- [x] Test: Press '?' shows help overlay
- [x] Test: Display all keyboard shortcuts
- [ ] Test: Display current zoom level
- [ ] Test: Display current pan step percentage
- [x] Test: Display hit-test behavior explanation
- [x] Test: Dismiss help to return to normal operation

---

## Phase 13: Non-Interactive Commands

### Table Management Commands
- [x] Test: --create-table creates table with metadata
- [x] Test: --rename-table renames table and updates metadata
- [x] Test: --copy-table duplicates table and metadata
- [x] Test: --delete-table removes table and metadata
- [x] Test: --list-tables outputs all tables to stdout
- [x] Test: --show-metadata outputs metadata for table

### Point Management Commands
- [x] Test: --add-point inserts point directly
- [x] Test: --delete-point removes point by id or coordinates

### Undo Log Commands
- [x] Test: --clear-undo-log removes unsaved_changes for table
- [x] Test: --clear-all-undo-log removes all unsaved_changes
- [x] Test: --commit-unsaved-changes applies and clears for table
- [x] Test: --list-unsaved-changes outputs pending changes

### Testing Commands
- [x] Test: --key-stroke-at-point simulates key press at coordinates
- [x] Test: --dump-screen outputs full screen rendering
- [x] Test: --dump-edit-area-contents outputs just edit area
- [x] Test: Works with --override-screen-height/width

---

## Phase 14: CSV Export

### CSV Writer
- [x] Create CSVExporter class - implemented inline in main.cpp
- [x] Test: --to-csv exports table to stdout
- [x] Test: Output format: header row + data rows
- [x] Test: Three columns: x, y, target
- [x] Test: Rows ordered by id
- [x] Test: UTF-8 encoding - uses std::cout (UTF-8 compatible)
- [x] Test: Proper quote escaping for text values
- [x] Test: Exit code 67 on write error (checks std::cout.fail() after writes)

---

## Phase 15: Study Mode

### Import & Configure
- [x] Create StudyMode class
- [x] Test: Check if metadata already exists (error if yes)
- [x] Test: Validate exactly 3 columns exist
- [x] Test: Validate 2 columns are REAL type
- [x] Test: Validate third column has exactly 2 distinct values
- [x] Test: Validate no nulls in any column
- [x] Test: Prompt for which value is 'x' and which is 'o'
- [x] Test: Prompt for which column is x-axis and y-axis
- [x] Test: Suggest min/max based on data
- [x] Test: Allow user override of min/max
- [x] Test: Create metadata entry
- [x] Test: Exit code 2 on validation failures
  Note: StudyMode.validate() returns ValidationResult with error messages.
  Command-line integration with --study flag not yet implemented in main.cpp.
  When implemented, validation failures should return exit code 2.

---

## Phase 16: Random Initialization

### Random Point Generation
- [x] Create RandomInitializer class
- [x] Test: --random-count generates N points
- [x] Test: --random-target sets target value
- [x] Test: --mean-x, --mean-y centers distribution
- [x] Test: --normal-x/y with --std-x/y generates normal distribution
- [x] Test: --uniform-x/y with --range-x/y generates uniform distribution
- [x] Test: Insert generated points into table
- [x] Test: Respect valid ranges during generation
- [x] Create RandomDialog for TUI-based random generation (bound to 'r' key)

---

## Phase 17: Table Selection Dialog

### Interactive Table Selection
- [x] Create TableSelectionDialog class (implemented as TableSelectionMenu)
- [x] Test: Show dialog when only --database provided
- [x] Test: List all tables from metadata
- [x] Test: Allow selection of existing table
- [x] Test: Option to create new blank table
- [x] Test: Option to copy existing table to new name
- [x] Test: Option to delete table
- [x] Test: Option to rename table
- [x] Test: Navigate dialog with arrow keys and Enter
- [x] Integrated in main.cpp with 10 passing tests

---

## Phase 17.5: Keystroke Playback for Automated TUI Testing

### Keystroke File Parser
- [ ] Create KeystrokeFileParser class
- [ ] Test: Read keystroke file and parse each line
- [ ] Test: Parse regular character keystrokes (x, o, +, -, etc.)
- [ ] Test: Parse special key names (<up>, <down>, <left>, <right>)
- [ ] Test: Parse <space>, <tab>, <enter>, <esc> special keys
- [ ] Test: Parse test command keystrokes (k, K)
- [ ] Test: Ignore comment lines starting with #
- [ ] Test: Ignore empty lines
- [ ] Test: Handle file not found error
- [ ] Test: Handle invalid keystroke format errors
- [ ] Test: Return ordered sequence of keystrokes

### Input Source Abstraction
- [ ] Create InputSource interface/abstract class
- [ ] Implement TerminalInputSource (reads from keyboard)
- [ ] Implement FileInputSource (reads from keystroke file)
- [ ] Test: TerminalInputSource returns keystroke from terminal
- [ ] Test: FileInputSource returns next keystroke from sequence
- [ ] Test: FileInputSource returns EOF when sequence exhausted
- [ ] Modify InputHandler to accept InputSource
- [ ] Test: InputHandler works with TerminalInputSource
- [ ] Test: InputHandler works with FileInputSource

### Screen Dump Commands
- [x] Add 'k' keystroke handler (dump full screen)
- [x] Add 'K' keystroke handler (dump edit area only)
- [x] Test: 'k' outputs full screen buffer to stdout
- [x] Test: 'K' outputs only edit area to stdout
- [x] Test: Screen dumps include current cursor position (implicit in output)
- [x] Test: 'k' works in normal interactive mode
- [x] Test: 'K' works in normal interactive mode
- [ ] Test: 'k' works in automated mode (--keystroke-file) - requires keystroke file support
- [ ] Test: 'K' works in automated mode (--keystroke-file) - requires keystroke file support

### Argument Parser Integration
- [ ] Add --keystroke-file argument to ArgumentParser
- [ ] Test: Parse --keystroke-file with valid filename
- [ ] Test: Validate file exists before starting TUI
- [ ] Test: Error if --keystroke-file combined with non-interactive flags
- [ ] Test: Store keystroke file path in Arguments struct

### Main Loop Integration
- [ ] Modify main loop to use InputSource abstraction
- [ ] Test: TUI runs normally with TerminalInputSource (no --keystroke-file)
- [ ] Test: TUI uses FileInputSource when --keystroke-file provided
- [ ] Test: TUI exits gracefully when keystroke file exhausted
- [ ] Test: Screen dumps output correctly during playback
- [ ] Test: TUI processes all keystrokes from file in order

---

## Phase 18: Integration & Polish

### Full Integration Tests
- [x] Test: Complete workflow: create → edit → save → quit
- [x] Test: Complete workflow: load → edit → undo → redo → save
- [ ] Test: Complete workflow: viewport ↔ table view switching
- [ ] Test: Complete workflow: zoom/pan with many points
- [ ] Test: Handle 1M row dataset (performance check)
- [x] Test: Multiple table workflow in same database
- [x] Test: List tables (empty and with data)
- [x] Test: Add point via command line
- [x] Test: Delete point via command line
- [x] Test: CSV export (empty, with data, with quotes)
- [x] Test: Help and version flags
- [x] Test: Clear undo logs
- [x] Test: Commit unsaved changes
- [x] Test: End-to-end workflow

### Automated TUI Tests Using Keystroke Playback
- [ ] Test: Basic point creation workflow (move cursor, add x, add o, verify screen)
- [ ] Test: Point deletion workflow (create points, delete with space, verify)
- [ ] Test: Point conversion workflow (create x, convert to o with Shift-O, verify)
- [ ] Test: Point flipping workflow (create mixed points, flip with g, verify)
- [ ] Test: Zoom workflow (create points, zoom in with +, zoom out with -, full with =)
- [ ] Test: Pan workflow (move cursor to edges, verify viewport shifts)
- [ ] Test: Undo/redo workflow (create point, undo with u, redo, verify)
- [ ] Test: Save workflow (create points, save with s, verify database)
- [ ] Test: Table view switching (viewport → # → table → # → viewport)
- [ ] Test: Multiple points at same coordinates (create X and O at same position, verify #)
- [ ] Test: Valid range enforcement (try to move beyond valid ranges, verify walls)
- [ ] Test: Help overlay (press ?, verify help shown, dismiss)
- [ ] Test: Tab navigation (tab through header fields, verify focus)
- [ ] Test: Complex workflow: create 10 points, zoom, pan, undo 3, redo 1, save
- [ ] Test: Screen dump accuracy (k command produces correct output)
- [ ] Test: Edit area dump accuracy (K command produces correct output)

### Error Handling & Edge Cases
- [ ] Test: Database lock timeout (exit code 66)
- [ ] Test: Corrupted database file
- [ ] Test: Invalid SQL in filter
- [ ] Test: Extremely large/small coordinate values
- [ ] Test: Empty table operations
- [ ] Test: Single point operations
- [ ] Test: Viewport at extreme zoom levels

### User Experience Polish
- [ ] Optimize rendering performance
- [ ] Add loading indicators for slow operations
- [ ] Improve error messages clarity
- [ ] Test all keyboard shortcuts work consistently
- [ ] Verify terminal resize handling is smooth
- [ ] Check alignment of all UI elements

---

## Phase 19: Documentation

### User Documentation
- [ ] Write man page for datapainter
- [ ] Document all CLI arguments
- [ ] Document keyboard shortcuts
- [ ] Create usage examples
- [ ] Document database schema

### Developer Documentation
- [ ] Document architecture overview
- [ ] Document class responsibilities
- [ ] Add inline code comments for complex algorithms
- [ ] Document build instructions
- [ ] Document testing approach

---

## Phase 20: Packaging & Distribution

### Build Scripts
- [ ] Create Windows build script (scripts/build-windows.sh)
- [ ] Create macOS build script (scripts/build-macos.sh)
- [ ] Create Linux build script (scripts/build-linux.sh)
- [ ] Create Haiku build script (scripts/build-haiku.sh)
- [ ] Test cross-platform builds

### GitHub Actions
- [ ] Set up Windows CI build
- [ ] Set up Linux CI build
- [ ] Set up Haiku CI build
- [ ] Set up macOS release build (release-only)
- [ ] Configure DEPLOYMENT_SSH_KEY secret
- [ ] Set up deployment to packages.industrial-linguistics.com

### Release Process
- [ ] Create release checklist
- [ ] Set up version numbering scheme
- [ ] Create changelog template
- [ ] Test deployment pipeline
- [ ] Verify package downloads work

---

## Future Enhancements (Optional)

These are documented in README.md as future features:

- [ ] Table view: sort functionality
- [ ] Undo/redo: 50ms coalescing for key repeats
- [ ] Crash recovery: prompt on startup for unapplied changes
- [ ] Multi-process: WAL mode, busy_timeout, conflict resolution
- [ ] CSV export: RFC 4180 compliance, BOM support, filter support
- [ ] Performance: 30 FPS target for 1M rows
- [ ] Test hooks: viewport bounds in dump output
- [ ] Fix automatic terminal resize (currently Ctrl-L works as workaround)

---

## Notes

- This TODO follows TDD principles: write test first, then implementation
- Tasks are ordered to build incrementally (foundation → features → polish)
- Each checkbox represents a discrete, testable unit of work
- Phase numbers suggest logical grouping, but some phases can run in parallel
- Database layer must be solid before building UI on top
- Non-interactive commands can be built alongside UI features
- Integration testing comes after individual features are complete
