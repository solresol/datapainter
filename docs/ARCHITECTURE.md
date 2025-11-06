# DataPainter Architecture

## Overview

DataPainter is a terminal user interface (TUI) application for creating and editing two-dimensional labeled datasets. The architecture follows a layered design with clear separation of concerns:

```
┌─────────────────────────────────────────────────────┐
│                   Main Event Loop                    │
│              (src/main.cpp)                          │
└─────────────────────────────────────────────────────┘
                        │
        ┌───────────────┼───────────────┐
        │               │               │
        ▼               ▼               ▼
┌──────────────┐ ┌─────────────┐ ┌──────────────┐
│   UI Layer   │ │ Logic Layer │ │  Data Layer  │
│  (Rendering) │ │ (Business)  │ │  (SQLite)    │
└──────────────┘ └─────────────┘ └──────────────┘
```

## Architectural Layers

### 1. Data Layer (Database & Models)

The data layer manages all interactions with the SQLite database and provides data structures for the application.

**Core Components:**

- **Database** (`database.h/cpp`): SQLite connection wrapper, handles opening/closing database connections
- **Metadata** (`metadata.h/cpp`): Table configuration (axis names, valid ranges, display settings)
- **DataTable** (`data_table.h/cpp`): Manages data table operations (CRUD for points)
- **TableManager** (`table_manager.h/cpp`): High-level table operations (create, delete, rename tables)
- **UnsavedChanges** (`unsaved_changes.h/cpp`): Tracks uncommitted edits for undo/redo
- **UndoLogManager** (`undo_log_manager.h/cpp`): Manages undo/redo history

**Database Schema:**

- `metadata`: Configuration for each table
- `<table_name>`: User data (id, x, y, target)
- `<table_name>_unsaved_changes`: Undo log (tracks creates, deletes, updates)

### 2. Logic Layer (Business Logic)

The logic layer handles application state, user input processing, and coordinates between UI and data layers.

**Core Components:**

- **ArgumentParser** (`argument_parser.h/cpp`): Parses and validates command-line arguments
- **Viewport** (`viewport.h/cpp`): Manages viewport state (zoom, pan, coordinate transforms)
- **PointEditor** (`point_editor.h/cpp`): Handles point manipulation operations
- **UndoManager** (`undo_manager.h/cpp`): Coordinates undo/redo operations
- **SaveManager** (`save_manager.h/cpp`): Manages saving changes to database
- **InputHandler** (`input_handler.h/cpp`): Processes keyboard input
- **StudyMode** (`study_mode.h/cpp`): Analyzes existing tables to suggest configuration
- **RandomInitializer** (`random_initializer.h/cpp`): Generates random point distributions

### 3. UI Layer (Rendering & Display)

The UI layer is responsible for rendering the interface and handling terminal interactions.

**Core Components:**

- **Terminal** (`terminal.h/cpp`): Terminal abstraction (buffer management, cursor control)
- **HeaderRenderer** (`header_renderer.h/cpp`): Renders top UI elements (database info, stats)
- **EditAreaRenderer** (`edit_area_renderer.h/cpp`): Renders the main viewport with points
- **AxisRenderer** (`axis_renderer.h/cpp`): Renders X and Y axis tick marks
- **FooterRenderer** (`footer_renderer.h/cpp`): Renders bottom UI (buttons, valid ranges)
- **TableView** (`table_view.h/cpp`): Tabular data editing mode
- **TableSelectionMenu** (`table_selection_menu.h/cpp`): Table picker dialog
- **HelpOverlay** (`help_overlay.h/cpp`): In-app help display
- **RandomDialog** (`random_dialog.h/cpp`): Dialog for random point generation
- **CursorUtils** (`cursor_utils.h/cpp`): Cursor positioning utilities

## Data Flow

### Interactive Mode

```
User Input (Keyboard)
  │
  ▼
InputHandler
  │
  ├──> PointEditor ──> UnsavedChanges ──> Database
  │                         │
  ├──> Viewport (zoom/pan)  │
  │                         │
  └──> UndoManager ◄────────┘
         │
         ▼
   Renderers ──> Terminal ──> Screen
```

### Non-Interactive Mode

```
CLI Arguments
  │
  ▼
ArgumentParser
  │
  ▼
TableManager / DataTable
  │
  ▼
Database
  │
  ▼
Output (stdout/stderr)
```

## Key Design Patterns

### 1. Separation of Concerns

Each layer has a clear responsibility:
- **Data Layer**: Persistence and data integrity
- **Logic Layer**: Application state and business rules
- **UI Layer**: Presentation only (no business logic)

### 2. Undo/Redo with Unsaved Changes

All edits are logged to `<table>_unsaved_changes` table:
- Creates: Store new points with action='create'
- Deletes: Mark points with action='delete', is_active=0
- Updates: Store new values with action='update'

This allows:
- **Undo**: Flip `is_active` flag on most recent change
- **Redo**: Flip `is_active` flag back
- **Save**: Apply all active changes to main table, clear undo log
- **Discard**: Clear all unsaved changes

### 3. Viewport Coordinate Transform

The **Viewport** class manages two coordinate systems:

- **Data coordinates**: Actual (x, y) values in database
- **Screen coordinates**: Row/col positions in terminal

Transformations:
```
screen_to_data(row, col) → (x, y)
data_to_screen(x, y) → (row, col)
```

Handles:
- Zoom levels (percentage of full valid range)
- Pan offsets (viewport center position)
- Aspect ratio (terminal cell dimensions)

### 4. Terminal Abstraction

The **Terminal** class provides a platform-independent buffer:
- Write to buffer: `write_char(row, col, ch)`
- Read from buffer: `read_char(row, col)`
- Render to screen: `render()` (outputs ANSI codes)
- Clear screen: `clear()`

This enables:
- Testing without actual terminal (buffer inspection)
- Consistent behavior across platforms
- Screen dumps for debugging

### 5. Metadata-Driven Configuration

Each table has metadata that drives UI behavior:
- Axis names (displayed in UI)
- Valid ranges (min_x, max_x, min_y, max_y)
- Target meanings (what 'x' and 'o' represent)
- Display settings (show_zero_bars)

Benefits:
- Multiple tables with different configurations
- Configuration persisted in database
- CLI args vs metadata conflict detection

## Module Interactions

### Creating a Point

1. User presses 'x' key
2. **InputHandler** detects keypress
3. **Viewport** converts cursor position to data coordinates
4. **PointEditor** validates coordinates against valid ranges
5. **UnsavedChanges** logs create action
6. **Database** inserts into unsaved_changes table
7. **EditAreaRenderer** re-renders viewport with new point

### Undo Last Action

1. User presses 'u' key
2. **UndoManager** queries most recent active change from unsaved_changes
3. **UndoLogManager** flips `is_active` flag to 0
4. **Database** updates unsaved_changes table
5. **EditAreaRenderer** re-renders without undone point

### Saving Changes

1. User presses 's' key
2. **SaveManager** retrieves all active unsaved changes
3. For each change:
   - Create: Insert into main data table
   - Delete: Remove from main data table
   - Update: Update row in main data table
4. **UndoLogManager** clears unsaved_changes table
5. UI updates unsaved count to 0

## Testing Architecture

DataPainter uses a multi-layered testing approach:

### Unit Tests (C++ / Google Test)

- Test individual classes in isolation
- Mock dependencies where needed
- Located in `tests/`
- Run via `ctest`

Example: `test_viewport.cpp` tests coordinate transforms without database

### Integration Tests (Python / pytest + pyte)

- Test actual TUI behavior with terminal emulation
- Use `pyte` library for VTE (Virtual Terminal Emulator)
- Simulate keyboard input, inspect screen output
- Located in `tests/integration/`
- Run via `pytest`

Example: `test_basic_operations.py` tests point creation by simulating 'x' keypress

### Test Framework

**tui_test_framework.py** provides:
- `DataPainterTest`: Context manager for running datapainter with VTE
- `send_keys()`: Simulate keyboard input
- `get_display_lines()`: Capture rendered screen
- `wait_for_text()`: Wait for UI element to appear

## Build System

**CMake** (`CMakeLists.txt`):
- C++17 standard
- SQLite3 linkage
- Google Test framework for unit tests
- Separate executables: `datapainter` (main app), `datapainter_tests` (unit tests)

**Build Process:**
```
mkdir build && cd build
cmake ..
make -j4
ctest --output-on-failure  # Run unit tests
```

## Code Organization

```
datapainter/
├── include/           # Header files (.h)
│   ├── database.h
│   ├── viewport.h
│   └── ...
├── src/              # Implementation files (.cpp)
│   ├── main.cpp
│   ├── database.cpp
│   └── ...
├── tests/            # C++ unit tests
│   ├── test_viewport.cpp
│   ├── test_database.cpp
│   └── ...
├── tests/integration/ # Python integration tests
│   ├── test_basic_operations.py
│   ├── tui_test_framework.py
│   └── ...
├── docs/             # Documentation
│   ├── ARCHITECTURE.md
│   ├── datapainter.1
│   └── ...
├── CMakeLists.txt    # Build configuration
└── README.md         # Project overview
```

## Performance Considerations

### Large Datasets

- **Viewport queries**: Only fetch points within visible range
- **Indexes**: Database indexes on x and y columns
- **Rendering**: Only re-render changed screen regions (not implemented yet)

### Undo Log Growth

- Undo log can grow large with many edits
- Mitigations:
  - Commit periodically to clear log
  - `--clear-undo-log` command for cleanup
  - Future: Auto-commit threshold

## Error Handling

**Exit Codes:**
- 0: Success
- 2: Invalid arguments
- 65: Failed to open database
- 66: Database operation failed
- 67: CSV write error

**Strategy:**
- Database errors: Return error codes, display to stderr
- Invalid input: Validate early (ArgumentParser, PointEditor)
- Graceful degradation: Continue operation when possible

## Future Architecture Enhancements

### Planned Improvements

1. **Event System**: Replace direct function calls with event bus
2. **Renderer Optimization**: Dirty region tracking, incremental updates
3. **Plugin System**: External point generators, exporters
4. **Multi-table View**: Edit multiple tables simultaneously
5. **Network Mode**: Client-server for remote editing

### Technical Debt

- Terminal abstraction needs better platform support (Windows)
- Renderer classes have some duplicated logic
- Test coverage for error paths could be improved
- No formal logging system (uses stderr)

## Contributing

When adding new features:

1. **Follow TDD**: Write tests first, then implementation
2. **Respect layer boundaries**: Don't mix UI and data logic
3. **Update documentation**: Keep this file and TODO.md current
4. **Follow coding standards**: See CLAUDE.md for style guide
5. **Add integration tests**: UI features need TUI tests, not just unit tests
