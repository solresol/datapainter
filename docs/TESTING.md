# Testing Guide for DataPainter

## Testing Philosophy

DataPainter follows Test-Driven Development (TDD) principles:

1. **Write the test first** - Define expected behavior before implementation
2. **See it fail** - Verify the test actually tests something
3. **Implement** - Write minimal code to make the test pass
4. **Refactor** - Clean up while keeping tests green

## Test Architecture

DataPainter uses a two-tier testing strategy:

### 1. Unit Tests (C++ / Google Test)

**Purpose:** Test individual classes and functions in isolation

**Framework:** Google Test (gtest)

**Location:** `tests/*.cpp`

**When to use:**
- Testing data structures and algorithms
- Testing business logic without UI
- Testing database operations
- Testing coordinate transforms

**Example:**
```cpp
TEST(ViewportTest, ScreenToDataSimple) {
    Viewport viewport;
    viewport.set_valid_ranges(-10, 10, -10, 10);
    viewport.set_screen_dimensions(20, 80);

    auto [x, y] = viewport.screen_to_data(10, 40);

    EXPECT_NEAR(x, 0.0, 0.1);  // Center of screen
    EXPECT_NEAR(y, 0.0, 0.1);
}
```

### 2. Integration Tests (Python / pytest + pyte)

**Purpose:** Test actual TUI behavior end-to-end

**Framework:** pytest with pyte (VTE emulator)

**Location:** `tests/integration/*.py`

**When to use:**
- Testing user interactions (keyboard input)
- Testing screen rendering
- Testing complete workflows
- Testing UI features

**Example:**
```python
def test_create_x_point():
    with DataPainterTest(width=80, height=24) as test:
        test.wait_for_text('test_table')

        # Press 'x' to create a point
        test.send_keys('x')
        time.sleep(0.3)

        # Verify point appears on screen
        lines = test.get_display_lines()
        content = '\n'.join(lines)
        assert 'x' in content
```

## Running Tests

### Quick Test Run

```bash
# Run all unit tests
cd build && ctest --output-on-failure

# Run all integration tests
uv run pytest tests/integration/ -v
```

### Unit Tests (C++)

```bash
cd build

# Run all unit tests
ctest --output-on-failure

# Run specific test suite
./datapainter_tests --gtest_filter=ViewportTest.*

# Run specific test
./datapainter_tests --gtest_filter=ViewportTest.ScreenToDataSimple

# List all tests
./datapainter_tests --gtest_list_tests

# Run with verbose output
ctest -V

# Run tests in parallel (4 jobs)
ctest -j4
```

### Integration Tests (Python)

```bash
# From project root

# Run all integration tests
uv run pytest tests/integration/ -v

# Run specific file
uv run pytest tests/integration/test_basic_operations.py -v

# Run specific test class
uv run pytest tests/integration/test_basic_operations.py::TestApplicationStartup -v

# Run specific test
uv run pytest tests/integration/test_basic_operations.py::TestApplicationStartup::test_startup_shows_table_name -v

# Run with extra verbosity (show print statements)
uv run pytest tests/integration/ -vv -s

# Run quickly (stop on first failure)
uv run pytest tests/integration/ -x

# Run quietly (summary only)
uv run pytest tests/integration/ -q
```

## Writing Unit Tests

### Test File Structure

```cpp
#include <gtest/gtest.h>
#include "my_class.h"

using namespace datapainter;

// Test fixture (optional, for shared setup)
class MyClassTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup code
        obj = std::make_unique<MyClass>();
    }

    void TearDown() override {
        // Cleanup code
    }

    std::unique_ptr<MyClass> obj;
};

// Test case
TEST_F(MyClassTest, MethodDoesSomething) {
    // Arrange
    int input = 42;

    // Act
    int result = obj->method(input);

    // Assert
    EXPECT_EQ(result, 84);
}

// Test without fixture
TEST(MyClassTest, StaticMethodWorks) {
    EXPECT_TRUE(MyClass::static_method());
}
```

### Common Assertions

```cpp
// Equality
EXPECT_EQ(actual, expected);
ASSERT_EQ(actual, expected);  // Fatal (stops test on failure)

// Floating point
EXPECT_NEAR(actual, expected, tolerance);
EXPECT_DOUBLE_EQ(actual, expected);

// Boolean
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);

// Strings
EXPECT_STREQ(str1, str2);
EXPECT_NE(str1, str2);

// Exceptions
EXPECT_THROW(statement, exception_type);
EXPECT_NO_THROW(statement);

// Nullptr
EXPECT_NE(ptr, nullptr);
EXPECT_EQ(ptr, nullptr);
```

### Test Naming Convention

```cpp
TEST(ClassName, DescriptiveTestName)
```

Examples:
- `TEST(Database, OpenValidFile)`
- `TEST(Viewport, ZoomInIncreasesScale)`
- `TEST(PointEditor, CreatePointWithinValidRange)`

## Writing Integration Tests

### Test File Structure

```python
import time
from tui_test_framework import DataPainterTest

class TestFeatureName:
    """Test suite for specific feature."""

    def test_specific_behavior(self):
        """Test that specific behavior works correctly."""
        with DataPainterTest(width=80, height=24) as test:
            # Wait for UI to be ready
            test.wait_for_text('test_table', timeout=3.0)

            # Simulate user input
            test.send_keys('x')
            time.sleep(0.3)  # Allow UI to update

            # Verify screen state
            lines = test.get_display_lines()
            content = '\n'.join(lines)

            assert 'expected text' in content
```

### DataPainterTest API

```python
# Constructor
DataPainterTest(
    width=80,              # Terminal width in columns
    height=24,             # Terminal height in rows
    database_path=None,    # Path to database (temp if None)
    table_name='test_table'  # Default table name
)

# Methods
test.send_keys(keys)           # Simulate keyboard input
test.get_display_lines()       # Get list of screen lines
test.wait_for_text(text, timeout=3.0)  # Wait for text to appear
test.get_cursor_position()     # Get (row, col) of cursor
```

### Keyboard Input

```python
# Single character
test.send_keys('x')

# Multiple characters
test.send_keys('xyz')

# Special keys
test.send_keys('ENTER')
test.send_keys('BACKSPACE')
test.send_keys('ESCAPE')
test.send_keys('TAB')
test.send_keys('UP')      # Arrow up
test.send_keys('DOWN')    # Arrow down
test.send_keys('LEFT')    # Arrow left
test.send_keys('RIGHT')   # Arrow right

# Shift combinations
test.send_keys('X')  # Shift+X (uppercase)
test.send_keys('O')  # Shift+O

# Control combinations
test.send_keys('\x03')  # Ctrl+C
```

### Screen Inspection

```python
# Get all screen lines
lines = test.get_display_lines()

# Check specific line
assert 'test_table' in lines[0]

# Check entire screen
content = '\n'.join(lines)
assert 'x' in content

# Check line contains specific text
assert any('Total points' in line for line in lines)

# Print screen for debugging
for i, line in enumerate(lines):
    print(f"{i:2d}: {line}")
```

### Timing Considerations

```python
# Wait for UI to stabilize after input
test.send_keys('x')
time.sleep(0.3)  # 300ms is usually enough

# Wait for specific text to appear
test.wait_for_text('expected text', timeout=3.0)

# Longer wait for slow operations
test.send_keys('s')  # Save
time.sleep(1.0)  # Database write might take longer
```

### Test Database Setup

```python
# Option 1: Use temporary database (default)
with DataPainterTest(width=80, height=24) as test:
    # Temp DB created automatically
    pass

# Option 2: Use specific database
import tempfile
import os

fd, temp_db = tempfile.mkstemp(suffix=".db")
os.close(fd)

try:
    # Initialize database first
    import subprocess
    subprocess.run([
        '../../build/datapainter',
        '--database', temp_db,
        '--create-table', '--table', 'test_table',
        '--target-column-name', 'label',
        '--x-axis-name', 'x',
        '--y-axis-name', 'y',
        '--x-meaning', 'positive',
        '--o-meaning', 'negative'
    ], check=True, capture_output=True)

    # Now use in test
    with DataPainterTest(database_path=temp_db) as test:
        # Test code here
        pass
finally:
    os.unlink(temp_db)
```

## Test Coverage

### Current Coverage

As of latest commit:
- **Unit tests:** 487 tests across all modules
- **Integration tests:** 61 tests (58 passed, 1 skipped)

### Coverage by Module

| Module | Unit Tests | Integration Tests |
|--------|------------|-------------------|
| Database | ✓ | ✓ |
| Viewport | ✓ | ✓ |
| PointEditor | ✓ | ✓ |
| UndoManager | ✓ | ✓ |
| Renderers | ✓ | ✓ |
| TableView | ✓ | ✓ |
| ArgumentParser | ✓ | ✓ |

### Adding Tests for New Features

When adding a new feature, follow this checklist:

- [ ] Write unit tests for new classes/methods
- [ ] Write integration tests for UI interactions
- [ ] Test happy path (expected use)
- [ ] Test edge cases (boundary values)
- [ ] Test error conditions (invalid input)
- [ ] Update test counts in this document

## Test Best Practices

### DO

✓ **Test one thing per test**
```cpp
TEST(Viewport, ZoomInIncreasesScale) {
    // Only test zoom in, not zoom out
}
```

✓ **Use descriptive names**
```python
def test_pressing_x_creates_x_point_at_cursor():
    # Clear what's being tested
```

✓ **Arrange-Act-Assert pattern**
```cpp
// Arrange: Set up test state
Viewport vp;
vp.set_valid_ranges(-10, 10, -10, 10);

// Act: Perform action
vp.zoom_in();

// Assert: Verify result
EXPECT_LT(vp.get_current_x_range(), 20.0);
```

✓ **Clean up resources**
```python
try:
    # Test code
    pass
finally:
    os.unlink(temp_file)
```

✓ **Use fixtures for shared setup**
```cpp
class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        db = std::make_unique<Database>(":memory:");
    }
    std::unique_ptr<Database> db;
};
```

### DON'T

✗ **Test multiple things in one test**
```cpp
TEST(Viewport, AllZoomOperations) {  // Too broad!
    viewport.zoom_in();
    viewport.zoom_out();
    viewport.full_viewport();
}
```

✗ **Use magic numbers**
```cpp
EXPECT_EQ(result, 42);  // What is 42?
// Better:
const int EXPECTED_POINT_COUNT = 42;
EXPECT_EQ(result, EXPECTED_POINT_COUNT);
```

✗ **Have brittle assertions**
```python
assert lines[5] == 'Total points=1'  # Will break if UI changes
# Better:
assert any('Total points=1' in line for line in lines)
```

✗ **Ignore timing issues**
```python
test.send_keys('x')
# Missing sleep here!
lines = test.get_display_lines()  # Might be too early
```

✗ **Leave debug code**
```python
# print(lines)  # Remove before committing
```

## Debugging Failing Tests

### Unit Test Failures

```bash
# Run failing test in verbose mode
./build/datapainter_tests --gtest_filter=MyTest.MyCase -V

# Run in debugger
gdb ./build/datapainter_tests
(gdb) run --gtest_filter=MyTest.MyCase
```

### Integration Test Failures

```bash
# Run with verbose output
uv run pytest tests/integration/test_file.py::test_name -vv -s

# Add debug prints in test
def test_something():
    test.send_keys('x')
    time.sleep(0.3)

    lines = test.get_display_lines()
    for i, line in enumerate(lines):
        print(f"{i:2d}: {line}")  # See what's on screen

    assert 'expected' in '\n'.join(lines)
```

### Common Issues

**Issue:** Test passes individually but fails in suite
- **Cause:** Shared state between tests
- **Fix:** Ensure clean state in SetUp/TearDown

**Issue:** Integration test times out
- **Cause:** UI not rendering or application hung
- **Fix:** Check for infinite loops, increase timeout

**Issue:** Flaky tests (pass sometimes)
- **Cause:** Race conditions, timing issues
- **Fix:** Add appropriate `time.sleep()`, use `wait_for_text()`

**Issue:** Different results on CI vs local
- **Cause:** Environment differences
- **Fix:** Check terminal size, database path, file permissions

## Performance Testing

DataPainter doesn't currently have dedicated performance tests, but you can benchmark:

```cpp
#include <chrono>

TEST(Viewport, RenderPerformance) {
    Viewport vp;
    vp.set_valid_ranges(-1000, 1000, -1000, 1000);

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 1000; ++i) {
        vp.screen_to_data(10, 10);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    EXPECT_LT(duration.count(), 100);  // Should take < 100ms
}
```

## Test Maintenance

### When to Update Tests

- **Breaking changes:** Update tests to match new behavior
- **Bug fixes:** Add regression test for the bug
- **Refactoring:** Tests should still pass (if not, you changed behavior)
- **New features:** Add new tests

### Removing Tests

Only remove tests if:
- Feature was completely removed
- Test was redundant (duplicate coverage)
- Test was testing implementation details, not behavior

## Continuous Integration

Tests should be run automatically on:
- Every commit
- Every pull request
- Before merging to main

Example GitHub Actions workflow:
```yaml
name: Tests
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install deps
        run: sudo apt-get install -y cmake libsqlite3-dev
      - name: Build
        run: mkdir build && cd build && cmake .. && make -j4
      - name: Unit tests
        run: cd build && ctest --output-on-failure
      - name: Integration tests
        run: uv run pytest tests/integration/ -v
```

## Further Reading

- [Google Test Documentation](https://google.github.io/googletest/)
- [pytest Documentation](https://docs.pytest.org/)
- [pyte Documentation](https://pyte.readthedocs.io/)
- [Test-Driven Development by Example](https://www.amazon.com/Test-Driven-Development-Kent-Beck/dp/0321146530)
