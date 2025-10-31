# DataPainter Integration Tests

End-to-end integration tests for the DataPainter TUI application using terminal emulation.

## Overview

These tests use [pyte](https://github.com/selectel/pyte) (a Python VTxxx terminal emulator) to test the actual screen output and user interactions of the DataPainter TUI. Unlike unit tests that test individual components, these integration tests:

- Spawn the actual `datapainter` binary in a pseudo-terminal (PTY)
- Send real keystrokes and validate screen output
- Test complete user workflows end-to-end
- Catch rendering bugs, coordinate conversion issues, and interaction problems

## Requirements

- Python 3.8+
- [uv](https://github.com/astral-sh/uv) package manager
- Built DataPainter executable at `../../build/datapainter`

## Installation

The test dependencies are managed through `pyproject.toml`. To install:

```bash
cd tests/integration
uv add pyte pytest pytest-timeout
```

Or simply run tests with `uv run pytest` (uv will automatically set up the environment).

## Running Tests

### Run all tests
```bash
uv run pytest
```

### Run specific test class
```bash
uv run pytest test_basic_operations.py::TestPointCreation -v
```

### Run single test
```bash
uv run pytest test_basic_operations.py::TestApplicationStartup::test_startup_shows_header -v
```

### Run with verbose output
```bash
uv run pytest -v -s
```

### Run with extra verbosity (show full diffs)
```bash
uv run pytest -vv
```

## Test Structure

### `tui_test_framework.py`
Core testing framework providing the `DataPainterTest` class:

- **PTY Management**: Spawns datapainter in pseudo-terminal with controlled dimensions
- **Terminal Emulation**: Uses pyte to capture and parse terminal output
- **Keystroke Injection**: Sends keys including special keys (arrows, backspace, delete)
- **Screen Validation**: Methods to check screen content and specific positions
- **Snapshot Testing**: Save screen states for regression testing

### `test_basic_operations.py`
Integration tests covering:

- **Application Startup**: Header display, axis rendering, initial state
- **Point Creation**: Creating 'x' and 'o' points with keyboard
- **Point Deletion**: Deleting points with backspace (known issues)
- **Point Conversion**: Converting between point types with X/O keys
- **Cursor Movement**: Moving cursor and verifying positions
- **Screen Resizing**: Different terminal sizes
- **Edge Cases**: Rapid input, viewport edges, quitting

## Writing New Tests

Example test using the framework:

```python
from tui_test_framework import DataPainterTest

def test_my_feature():
    with DataPainterTest(width=80, height=24) as test:
        # Wait for UI to load
        test.wait_for_text('test_table', timeout=3.0)

        # Send keystrokes
        test.send_keys('x')  # Create a point
        test.send_keys('RIGHT')  # Move cursor
        test.send_keys('o')  # Create another point

        # Validate screen content
        lines = test.get_display_lines()
        assert 'x' in '\n'.join(lines)

        # Check specific position
        test.assert_char_at(row=10, col=40, expected='x')

        # Save snapshot for regression testing
        test.save_snapshot('my_feature_state')
```

### Special Keys

The framework supports these special key names:
- `'ENTER'` - Enter/return key
- `'BACKSPACE'` - Backspace key
- `'DELETE'` - Forward delete key
- `'UP'`, `'DOWN'`, `'LEFT'`, `'RIGHT'` - Arrow keys
- `'ESC'` - Escape key

Regular characters are sent as-is: `test.send_keys('xyz123')`

### Best Practices

1. **Wait for UI**: Always call `test.wait_for_text('test_table', timeout=3.0)` after starting to ensure startup completes
2. **Add delays**: Use `time.sleep(0.1-0.2)` after keystrokes to allow screen updates
3. **Check middle area**: When checking for points, focus on the middle content area (rows 8-18, cols 10-70) to avoid axis labels
4. **Use counts**: Compare counts before/after actions rather than exact matches to handle axis tick marks
5. **Set timeouts**: The global pytest timeout is 10s - adjust per-test if needed

## Known Issues

### Deletion Tests Failing
Tests in `TestPointDeletion` currently fail because point deletion is not working reliably in the application. This is a known bug that the tests are correctly catching:
- Single point deletion doesn't work
- Multiple point deletion is unreliable
- Backspace key registers but doesn't remove points from display

This validates the user's earlier report: "The backspace key works (sometimes) when there are multiple objects in a cell, but not reliably."

**Fix required in**: `src/point_editor.cpp` or `src/edit_area_renderer.cpp` - likely an issue with how unsaved changes are being applied to the viewport rendering.

### Test Timeouts
If tests hang, check:
- Is datapainter executable built? (`make` in project root)
- Does the application exit on 'q' key? (process cleanup issue)
- Are there deadlocks in ncurses refresh? (terminal I/O blocking)

## Debugging Failed Tests

### View full screen output
```python
def test_debug():
    with DataPainterTest(width=80, height=24) as test:
        test.wait_for_text('test_table', timeout=3.0)
        lines = test.get_display_lines()
        for i, line in enumerate(lines):
            print(f"{i:2d} |{line}|")
```

### Save snapshots
```python
test.save_snapshot('before_action', directory='debug_snapshots')
test.send_keys('x')
test.save_snapshot('after_action', directory='debug_snapshots')
```

Then examine `debug_snapshots/*.txt` files.

### Check exact positions
```python
char = test.get_cell(row=10, col=40)
print(f"Character at (10, 40): '{char}'")
```

## Architecture

```
┌─────────────────────────────────────────────┐
│  pytest test_basic_operations.py            │
│  ┌───────────────────────────────────────┐  │
│  │  DataPainterTest Context Manager      │  │
│  │  ┌─────────────────────────────────┐  │  │
│  │  │  pty.fork()                     │  │  │
│  │  │  ├─ Parent: Python test code   │  │  │
│  │  │  └─ Child: datapainter binary  │  │  │
│  │  └─────────────────────────────────┘  │  │
│  │  ┌─────────────────────────────────┐  │  │
│  │  │  pyte Screen/ByteStream         │  │  │
│  │  │  - Emulates VTxxx terminal      │  │  │
│  │  │  - Parses escape sequences      │  │  │
│  │  │  - Maintains screen buffer      │  │  │
│  │  └─────────────────────────────────┘  │  │
│  └───────────────────────────────────────┘  │
└─────────────────────────────────────────────┘
```

## Contributing

When adding new tests:
1. Use descriptive test names: `test_create_point_at_cursor_position`
2. Add docstrings explaining what's being tested
3. Group related tests in classes
4. Update this README with new test categories
5. Mark known failures with `@pytest.mark.xfail(reason="...")`

## References

- [pyte documentation](https://pyte.readthedocs.io/)
- [pytest documentation](https://docs.pytest.org/)
- [PTY programming guide](https://www.rkoucha.fr/tech_corner/pty_pdip.html)
