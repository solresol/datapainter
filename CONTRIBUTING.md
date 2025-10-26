# Contributing to DataPainter

Thank you for your interest in contributing to DataPainter!

## Development Workflow

DataPainter follows **Test-Driven Development (TDD)** with a red-green-refactor cycle:

1. **Red**: Write a failing test first
2. **Green**: Write minimal code to make the test pass
3. **Refactor**: Improve the code while keeping tests green

## Getting Started

### Prerequisites

- C++17 or later compiler (GCC, Clang, or MSVC)
- CMake 3.14+
- SQLite3 development libraries
- ncurses (Linux/macOS) or equivalent terminal library

### Setup

```bash
# Clone the repository
git clone https://github.com/yourusername/datapainter.git
cd datapainter

# Install dependencies (Linux)
sudo apt-get install cmake g++ libsqlite3-dev libncurses-dev

# Install dependencies (macOS)
brew install cmake sqlite3

# Install dependencies (Windows)
vcpkg install sqlite3:x64-windows

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Run tests
ctest --output-on-failure
```

## Project Structure

```
datapainter/
├── src/              # Implementation files (.cpp)
├── include/          # Header files (.h)
├── tests/            # Test files (test_*.cpp)
├── scripts/          # Build and packaging scripts
├── docs/             # Documentation
│   └── man/          # Man pages
├── .github/          # GitHub Actions CI
├── CMakeLists.txt    # Build configuration
├── README.md         # Project overview and specification
├── TODO.md           # Implementation roadmap
└── CLAUDE.md         # Coding standards and AI context
```

## Coding Standards

See `CLAUDE.md` for detailed C++ coding standards. Key points:

- **Style**: Follow `.clang-format` (LLVM-based, 4-space indent, 100 char line limit)
- **Naming**: See CLAUDE.md for conventions
- **Error handling**: See CLAUDE.md for approach
- **Memory**: Prefer RAII and smart pointers

### Formatting Code

Format code before committing:

```bash
# Format all C++ files
find src include tests -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

## Development Process

### 1. Pick a Task from TODO.md

The `TODO.md` file contains our implementation roadmap organized in 20 phases. Pick a task that:
- Is not yet started
- Has its prerequisites completed
- Matches your skill level

### 2. Write Tests First

Before implementing any feature:

1. Create or update a test file in `tests/`
2. Write a test that describes the desired behavior
3. Run the test and verify it fails (red phase)

Example:
```cpp
// tests/test_database.cpp
#include <gtest/gtest.h>
#include "database.h"

TEST(DatabaseTest, OpenDatabaseFile) {
    Database db("test.db");
    EXPECT_TRUE(db.is_open());
}
```

### 3. Implement the Feature

Write minimal code to make the test pass:

```cpp
// include/database.h
class Database {
public:
    explicit Database(const std::string& filename);
    bool is_open() const;
private:
    // implementation details
};
```

### 4. Run Tests

```bash
cd build
ctest --output-on-failure
```

### 5. Refactor

Once tests pass, improve the code:
- Extract duplicated code
- Improve naming
- Add documentation
- Keep tests green throughout

### 6. Commit

Follow the commit message format:

```
Brief one-line summary (imperative mood)

Optional longer description explaining:
- Why this change was made
- What problem it solves
- Any important implementation details

🤖 Generated with [Claude Code](https://claude.com/claude-code)

Co-Authored-By: Claude <noreply@anthropic.com>
```

## Testing Guidelines

### Test Organization

- One test file per implementation file (`test_database.cpp` for `database.cpp`)
- Group related tests with `TEST(ClassName, TestName)`
- Use descriptive test names that explain what is being tested

### Test Coverage

Aim for tests that cover:
- **Happy path**: Normal, expected usage
- **Edge cases**: Boundary conditions
- **Error cases**: Invalid input, failure conditions
- **Integration**: How components work together

### Example Test Structure

```cpp
TEST(ViewportTest, ConvertScreenToDataCoordinates) {
    Viewport viewport(0.0, 10.0, 0.0, 10.0, 80, 24);

    // Happy path
    auto [x, y] = viewport.screen_to_data(40, 12);
    EXPECT_NEAR(x, 5.0, 0.1);
    EXPECT_NEAR(y, 5.0, 0.1);

    // Edge cases
    auto [x_min, y_min] = viewport.screen_to_data(0, 0);
    EXPECT_NEAR(x_min, 0.0, 0.1);

    auto [x_max, y_max] = viewport.screen_to_data(79, 23);
    EXPECT_NEAR(x_max, 10.0, 0.1);
}
```

## Continuous Integration

All pull requests must:
- Pass all existing tests
- Include tests for new functionality
- Build successfully on Linux and Windows (macOS on releases only)
- Follow code formatting standards

CI runs automatically via GitHub Actions on every push and PR.

## Documentation

### Code Documentation

- Add comments for complex algorithms
- Document public APIs with clear descriptions
- Explain *why*, not just *what*

### Man Pages

Update `docs/man/datapainter.1` when adding:
- New command-line arguments
- New features
- Changed behavior

## Getting Help

- Check `README.md` for specification details
- Review `TODO.md` for implementation context
- Look at existing tests for examples
- Open an issue for questions

## License

By contributing, you agree that your contributions will be licensed under the same license as the project (see LICENSE file).

## Questions?

Feel free to open an issue for:
- Feature requests
- Bug reports
- Questions about the codebase
- Suggestions for improvement

Happy coding!
