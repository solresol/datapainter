# DataPainter TODO List

This is a comprehensive implementation plan for the DataPainter TUI project. Tasks are organized by feature area and ordered to support Test-Driven Development (TDD) with incremental builds.

---

## Phase 0: Project Setup

### Build System & Infrastructure
- [x] Create CMakeLists.txt with C++17/20 standard
- [x] Set up project directory structure (src/, include/, tests/)
- [ ] Configure SQLite3 dependency
- [ ] Set up testing framework (Google Test or Catch2)
- [ ] Create main.cpp entry point
- [ ] Set up continuous integration (GitHub Actions)
- [ ] Create .gitignore for build artifacts

---

## Phase 1: Database Layer (Foundation)

### Core Schema & Connection
- [ ] Create Database class with SQLite connection management
- [ ] Test: Open/close database file
- [ ] Test: Create metadata table with correct schema
- [ ] Test: Create unsaved_changes table with correct schema
- [ ] Test: Detect if database file is world-writable and warn
- [ ] Implement table name validation (regex: `[A-Za-z0-9_]+`)
- [ ] Test: Reject invalid table names

### Data Table Operations
- [ ] Create DataTable class for managing data tables
- [ ] Test: Create data table with indexes (id, x, y, target)
- [ ] Test: Insert point with (x, y, target) values
- [ ] Test: Delete point by id
- [ ] Test: Update point target value
- [ ] Test: Query points within viewport bounds
- [ ] Test: Get distinct target values from table
- [ ] Test: Count points by target value

### Metadata Operations
- [ ] Create Metadata class/struct
- [ ] Test: Insert metadata row for new table
- [ ] Test: Read metadata for existing table
- [ ] Test: Update metadata fields
- [ ] Test: List all tables from metadata
- [ ] Test: Delete metadata entry
- [ ] Test: Rename table in metadata and data table
- [ ] Test: Copy table (data + metadata)

### Unsaved Changes Tracking
- [ ] Create UnsavedChanges class
- [ ] Test: Record insert action
- [ ] Test: Record delete action
- [ ] Test: Record update action
- [ ] Test: Record metadata change action
- [ ] Test: Retrieve unsaved changes for a table
- [ ] Test: Apply unsaved changes to data table (commit)
- [ ] Test: Clear unsaved changes for a table
- [ ] Test: Support undo (mark change as undone)
- [ ] Test: Support redo (re-apply undone change)

---

## Phase 2: CLI Argument Parsing

### Argument Parser
- [ ] Create ArgumentParser class
- [ ] Test: Parse --database argument
- [ ] Test: Parse --table argument
- [ ] Test: Parse axis names (--x-axis-name, --y-axis-name)
- [ ] Test: Parse --target-column-name
- [ ] Test: Parse meanings (--x-meaning, --o-meaning)
- [ ] Test: Parse valid ranges (--min-x, --max-x, --min-y, --max-y)
- [ ] Test: Validate min <= max for ranges (exit code 2 if invalid)
- [ ] Test: Parse --show-zero-bars flag
- [ ] Test: Parse screen overrides (--override-screen-height/width)
- [ ] Test: Parse --start-tabular flag

### Conflict Detection
- [ ] Test: Detect CLI vs metadata conflicts
- [ ] Test: Generate clear error message showing conflicts
- [ ] Test: Suggest resolution (remove flag or use different table)
- [ ] Test: Exit with code 2 on conflict

### Non-Interactive Mode Arguments
- [ ] Test: Parse --create-table
- [ ] Test: Parse --rename-table
- [ ] Test: Parse --copy-table
- [ ] Test: Parse --delete-table
- [ ] Test: Parse --list-tables
- [ ] Test: Parse --show-metadata
- [ ] Test: Parse --add-point
- [ ] Test: Parse --delete-point
- [ ] Test: Parse --to-csv
- [ ] Test: Parse --key-stroke-at-point with x,y,key

### Testing & Debug Arguments
- [ ] Test: Parse --dump-screen
- [ ] Test: Parse --dump-edit-area-contents
- [ ] Test: Parse --zoom-in / --zoom-out
- [ ] Test: Parse --list-x-axis-marks / --list-y-axis-marks

### Study Mode Arguments
- [ ] Test: Parse --study flag
- [ ] Validate requires both --database and --table

### Random Init Arguments
- [ ] Test: Parse --random-count
- [ ] Test: Parse --random-target
- [ ] Test: Parse --mean-x, --mean-y
- [ ] Test: Parse --normal-x, --normal-y, --std-x, --std-y
- [ ] Test: Parse --uniform-x, --uniform-y, --range-x, --range-y

### Undo Log Arguments
- [ ] Test: Parse --clear-undo-log
- [ ] Test: Parse --clear-all-undo-log
- [ ] Test: Parse --commit-unsaved-changes
- [ ] Test: Parse --list-unsaved-changes

---

## Phase 3: Viewport Math & Coordinate System

### Coordinate System
- [ ] Create Viewport class
- [ ] Test: Initialize viewport with (x_min, x_max, y_min, y_max)
- [ ] Test: Calculate default viewport for empty table (use valid ranges or [-1,1,-1,1])
- [ ] Test: Calculate auto-fit viewport with 10% padding
- [ ] Test: Convert screen coordinates to data coordinates
- [ ] Test: Convert data coordinates to screen coordinates
- [ ] Test: Determine if data point is visible in viewport
- [ ] Test: Round data coordinates to screen cell

### Zoom Operations
- [ ] Test: Zoom in (halve data-per-screen-space, centered on cursor)
- [ ] Test: Zoom out (double data-per-screen-space, centered on cursor)
- [ ] Test: Full viewport zoom (= key, fit all data)
- [ ] Test: Respect valid range constraints during zoom
- [ ] Test: Calculate decimal place for major tick marks (log10 of range)

### Pan Operations
- [ ] Test: Pan right (shift viewport if cursor at edge and range available)
- [ ] Test: Pan left
- [ ] Test: Pan up
- [ ] Test: Pan down
- [ ] Test: Prevent pan beyond valid ranges
- [ ] Test: Cursor movement within viewport (no pan)

---

## Phase 4: Terminal/Screen Management

### Terminal Detection
- [ ] Create Terminal class
- [ ] Test: Detect terminal dimensions (rows, cols)
- [ ] Test: Override dimensions with CLI args
- [ ] Test: Exit code 64 if override exceeds actual terminal size
- [ ] Test: Handle SIGWINCH (terminal resize)
- [ ] Test: Pause rendering if screen too small (< header + 3 rows)
- [ ] Test: Show "enlarge terminal" message when too small
- [ ] Test: Resume rendering when size adequate

### Screen Buffer
- [ ] Create ScreenBuffer class
- [ ] Test: Allocate 2D character buffer (rows x cols)
- [ ] Test: Write character at (row, col)
- [ ] Test: Clear screen buffer
- [ ] Test: Render buffer to stdout
- [ ] Test: Block wide characters (emoji, multi-byte)

---

## Phase 5: Axis Rendering & Labels

### Tick Mark Algorithm
- [ ] Create AxisRenderer class
- [ ] Test: Calculate tick step (10^k × {1,2,5}) to prevent label collision
- [ ] Test: Generate major tick positions
- [ ] Test: Generate minor tick positions (if >= 3 chars between majors)
- [ ] Test: Generate tenth tick positions (if >= 6 chars between majors)
- [ ] Test: Format tick labels (standard printf/iostream)
- [ ] Test: Use scientific notation for |k| >= 4

### Axis Drawing
- [ ] Test: Draw x-axis with tick marks and labels
- [ ] Test: Draw y-axis with tick marks and labels
- [ ] Test: Draw x-axis label (column name)
- [ ] Test: Draw y-axis label (column name)
- [ ] Test: Handle negative numbers in labels
- [ ] Test: Align labels correctly with tick marks

### Zero Bars (Cartesian Axes)
- [ ] Test: Do NOT show zero bars by default
- [ ] Test: Show vertical line at x=0 when --show-zero-bars AND x=0 in viewport
- [ ] Test: Show horizontal line at y=0 when --show-zero-bars AND y=0 in viewport
- [ ] Test: Use distinct characters for zero bars (| and ─)
- [ ] Test: Differentiate zero bars from grid lines

---

## Phase 6: Viewport Rendering

### Edit Area Rendering
- [ ] Create EditAreaRenderer class
- [ ] Test: Draw border around edit area
- [ ] Test: Render empty edit area
- [ ] Test: Query data points within current viewport
- [ ] Test: Apply unsaved changes to viewport data
- [ ] Test: Render single 'x' point at screen position
- [ ] Test: Render single 'o' point at screen position
- [ ] Test: Render multiple x's at same cell as 'X'
- [ ] Test: Render multiple o's at same cell as 'O'
- [ ] Test: Render mixed x+o at same cell as '#'
- [ ] Test: Render wall characters '!' for out-of-valid-range areas
- [ ] Test: Draw cursor at current position
- [ ] Test: Show cursor coordinates in header

### Header Rendering
- [ ] Create HeaderRenderer class
- [ ] Test: Display database filename
- [ ] Test: Display table name
- [ ] Test: Display target column name
- [ ] Test: Display x and o meanings
- [ ] Test: Display counts (total, x count, o count)
- [ ] Test: Display valid x/y ranges
- [ ] Test: Display current zoom info
- [ ] Test: Highlight focused field (for Tab navigation)

### Footer Rendering
- [ ] Test: Display zoom controls (+ - =)
- [ ] Test: Display cursor position
- [ ] Test: Display valid ranges
- [ ] Test: Display action buttons (Tabular, Undo, Quit, Save)

---

## Phase 7: Input Handling & Navigation

### Keyboard Input
- [ ] Create InputHandler class
- [ ] Test: Read single keypress (blocking)
- [ ] Test: Parse arrow keys
- [ ] Test: Parse letter keys (x, o, g, X, O, etc.)
- [ ] Test: Parse special keys (Tab, Enter, Space, Esc)
- [ ] Test: Parse symbols (+, -, =, #, ?, u, q, s)

### Cursor Movement
- [ ] Test: Move cursor right within viewport
- [ ] Test: Move cursor left within viewport
- [ ] Test: Move cursor up within viewport
- [ ] Test: Move cursor down within viewport
- [ ] Test: Pan viewport when cursor reaches edge
- [ ] Test: Prevent cursor movement beyond valid ranges

### Tab Navigation
- [ ] Test: Tab cycles through header fields (left to right)
- [ ] Test: Tab order: database → table → target → meanings → ranges → buttons → viewport
- [ ] Test: Enter activates focused field/button
- [ ] Test: Edit focused field inline

---

## Phase 8: Point Editing Operations

### Point Creation
- [ ] Create PointEditor class
- [ ] Test: Press 'x' creates x point at cursor
- [ ] Test: Press 'o' creates o point at cursor
- [ ] Test: Reject point creation outside valid ranges
- [ ] Test: Record point creation in unsaved_changes
- [ ] Test: Update on-screen display immediately

### Point Deletion
- [ ] Test: Press Space deletes all points under cursor
- [ ] Test: "Under cursor" means rounds to same screen cell
- [ ] Test: Record deletion in unsaved_changes
- [ ] Test: Update display after deletion

### Point Conversion
- [ ] Test: Press Shift-X converts o points under cursor to x
- [ ] Test: Press Shift-O converts x points under cursor to o
- [ ] Test: Press 'g' flips each point individually (x↔o)
- [ ] Test: Record conversions in unsaved_changes

### Hit Testing
- [ ] Test: Find all points that round to cursor's screen cell
- [ ] Test: Handle multiple points at exact same (x,y)
- [ ] Test: Handle points close enough to round to same cell

---

## Phase 9: Undo/Redo System

### Undo Stack Management
- [ ] Create UndoManager class
- [ ] Test: Track current position in unsaved_changes
- [ ] Test: Press 'u' undoes last action
- [ ] Test: Multiple undo steps backward
- [ ] Test: Enable redo after undo
- [ ] Test: Clear redo stack on new edit
- [ ] Test: Display undo/redo availability in UI

### Undo/Redo Operations
- [ ] Test: Undo point insert (mark as undone)
- [ ] Test: Undo point delete
- [ ] Test: Undo point update
- [ ] Test: Undo metadata change
- [ ] Test: Redo undone operation
- [ ] Test: Update display after undo/redo

---

## Phase 10: Save/Quit Operations

### Save Mechanism
- [ ] Create SaveManager class
- [ ] Test: Commit all unsaved_changes to data table in transaction
- [ ] Test: Apply inserts, deletes, updates in correct order
- [ ] Test: Apply metadata changes
- [ ] Test: Clear/mark unsaved_changes as applied after save
- [ ] Test: Continue running after save (don't exit)
- [ ] Test: Handle save errors (exit code 65)

### Quit Operations
- [ ] Test: Press 'q' quits without saving (if no unsaved changes)
- [ ] Test: Warn if unsaved changes exist
- [ ] Test: "Quit without saving" button discards changes
- [ ] Test: Exit with code 0 on normal quit

---

## Phase 11: Tabular View

### Table View UI
- [ ] Create TableView class
- [ ] Test: Display three columns (x, y, target)
- [ ] Test: Show all rows by default
- [ ] Test: Display filter at top
- [ ] Test: Default filter = current viewport bounds when entering from viewport
- [ ] Test: Navigate rows with arrow keys
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
- [ ] Test: Edit filter (SQL WHERE clause)
- [ ] Test: Apply filter to visible rows
- [ ] Test: Switch from viewport to table (press #)
- [ ] Test: Switch from table to viewport (press #)
- [ ] Test: Set viewport to fit filtered rows when returning to viewport
- [ ] Test: Preserve unsaved changes across view switches

---

## Phase 12: Help System

### Help Overlay
- [ ] Create HelpOverlay class
- [ ] Test: Press '?' shows help overlay
- [ ] Test: Display all keyboard shortcuts
- [ ] Test: Display current zoom level
- [ ] Test: Display current pan step percentage
- [ ] Test: Display hit-test behavior explanation
- [ ] Test: Dismiss help to return to normal operation

---

## Phase 13: Non-Interactive Commands

### Table Management Commands
- [ ] Test: --create-table creates table with metadata
- [ ] Test: --rename-table renames table and updates metadata
- [ ] Test: --copy-table duplicates table and metadata
- [ ] Test: --delete-table removes table and metadata
- [ ] Test: --list-tables outputs all tables to stdout
- [ ] Test: --show-metadata outputs metadata for table

### Point Management Commands
- [ ] Test: --add-point inserts point directly
- [ ] Test: --delete-point removes point by id or coordinates

### Undo Log Commands
- [ ] Test: --clear-undo-log removes unsaved_changes for table
- [ ] Test: --clear-all-undo-log removes all unsaved_changes
- [ ] Test: --commit-unsaved-changes applies and clears for table
- [ ] Test: --list-unsaved-changes outputs pending changes

### Testing Commands
- [ ] Test: --key-stroke-at-point simulates key press at coordinates
- [ ] Test: --dump-screen outputs full screen rendering
- [ ] Test: --dump-edit-area-contents outputs just edit area
- [ ] Test: Works with --override-screen-height/width

---

## Phase 14: CSV Export

### CSV Writer
- [ ] Create CSVExporter class
- [ ] Test: --to-csv exports table to stdout
- [ ] Test: Output format: header row + data rows
- [ ] Test: Three columns: x, y, target
- [ ] Test: Rows ordered by id
- [ ] Test: UTF-8 encoding
- [ ] Test: Proper quote escaping for text values
- [ ] Test: Exit code 67 on write error

---

## Phase 15: Study Mode

### Import & Configure
- [ ] Create StudyMode class
- [ ] Test: Check if metadata already exists (error if yes)
- [ ] Test: Validate exactly 3 columns exist
- [ ] Test: Validate 2 columns are REAL type
- [ ] Test: Validate third column has exactly 2 distinct values
- [ ] Test: Validate no nulls in any column
- [ ] Test: Prompt for which value is 'x' and which is 'o'
- [ ] Test: Prompt for which column is x-axis and y-axis
- [ ] Test: Suggest min/max based on data
- [ ] Test: Allow user override of min/max
- [ ] Test: Create metadata entry
- [ ] Test: Exit code 2 on validation failures

---

## Phase 16: Random Initialization

### Random Point Generation
- [ ] Create RandomInitializer class
- [ ] Test: --random-count generates N points
- [ ] Test: --random-target sets target value
- [ ] Test: --mean-x, --mean-y centers distribution
- [ ] Test: --normal-x/y with --std-x/y generates normal distribution
- [ ] Test: --uniform-x/y with --range-x/y generates uniform distribution
- [ ] Test: Insert generated points into table
- [ ] Test: Respect valid ranges during generation

---

## Phase 17: Table Selection Dialog

### Interactive Table Selection
- [ ] Create TableSelectionDialog class
- [ ] Test: Show dialog when only --database provided
- [ ] Test: List all tables from metadata
- [ ] Test: Allow selection of existing table
- [ ] Test: Option to create new blank table
- [ ] Test: Option to copy existing table to new name
- [ ] Test: Option to delete table
- [ ] Test: Option to rename table
- [ ] Test: Navigate dialog with arrow keys and Enter

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
- [ ] Add 'k' keystroke handler (dump full screen)
- [ ] Add 'K' keystroke handler (dump edit area only)
- [ ] Test: 'k' outputs full screen buffer to stdout
- [ ] Test: 'K' outputs only edit area to stdout
- [ ] Test: Screen dumps include current cursor position
- [ ] Test: 'k' works in normal interactive mode
- [ ] Test: 'K' works in normal interactive mode
- [ ] Test: 'k' works in automated mode (--keystroke-file)
- [ ] Test: 'K' works in automated mode (--keystroke-file)

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

---

## Notes

- This TODO follows TDD principles: write test first, then implementation
- Tasks are ordered to build incrementally (foundation → features → polish)
- Each checkbox represents a discrete, testable unit of work
- Phase numbers suggest logical grouping, but some phases can run in parallel
- Database layer must be solid before building UI on top
- Non-interactive commands can be built alongside UI features
- Integration testing comes after individual features are complete
