- Development is done in C++
- Features are added via red-green TDD

## Project Planning

- TODO.md contains the comprehensive implementation plan organized in 20 phases
- The TODO list follows TDD principles: write tests first, then implementation
- Tasks are ordered to build incrementally from foundation (database layer) through features to polish
- Each checkbox in TODO.md represents a discrete, testable unit of work
- Phases are logical groupings; some can be worked on in parallel
- The database layer (Phase 1) must be solid before building UI features on top
- Current project status: Greenfield (no code exists yet)

## C++ Coding Standards

### Language Version
- Use C++17 standard (set in CMakeLists.txt)
- Avoid compiler-specific extensions
- Prefer standard library over platform-specific alternatives when possible

### Naming Conventions
- **Classes/Structs**: PascalCase (`Database`, `Viewport`, `AxisRenderer`)
- **Functions/Methods**: snake_case (`open_database()`, `screen_to_data()`)
- **Variables**: snake_case (`x_min`, `cursor_pos`, `table_name`)
- **Constants**: UPPER_SNAKE_CASE (`MAX_SCREEN_WIDTH`, `DEFAULT_ZOOM`)
- **Private members**: trailing underscore (`data_`, `db_connection_`)
- **Namespaces**: lowercase (`datapainter`, `db`, `ui`)

### File Organization
- **Headers**: `.h` extension in `include/` directory
- **Implementation**: `.cpp` extension in `src/` directory
- **Tests**: `test_*.cpp` in `tests/` directory
- One class per file (typically), filename matches class name in snake_case
  - Example: `class Database` → `include/database.h` + `src/database.cpp`

### Header Structure
```cpp
#pragma once  // Use pragma once instead of include guards

#include <standard_library>  // Standard library first
#include <another_std_lib>

#include "project_headers.h"  // Project headers second

namespace datapainter {

class MyClass {
public:
    // Public interface first
    MyClass();
    ~MyClass();

    void public_method();

private:
    // Private implementation last
    void private_method();
    int member_variable_;
};

}  // namespace datapainter
```

### Error Handling
- **Database errors**: Return error codes (matching README.md exit codes: 65, 66)
- **File I/O errors**: Return error codes (67 for CSV write errors)
- **Invalid arguments**: Throw `std::invalid_argument` or return error code 2
- **Fatal errors**: Use `std::cerr` for error messages, then exit with appropriate code
- **Optional values**: Use `std::optional<T>` for values that may not exist
- **Error propagation**: Prefer explicit error handling over exceptions where performance matters

### Memory Management
- **Ownership**: Use RAII (Resource Acquisition Is Initialization) everywhere
- **Pointers**:
  - Prefer `std::unique_ptr<T>` for exclusive ownership
  - Use `std::shared_ptr<T>` only when shared ownership is truly needed
  - Avoid raw pointers except for non-owning references
- **Containers**: Use STL containers (`std::vector`, `std::string`, `std::map`)
- **Cleanup**: Destructors handle all cleanup; never require manual cleanup

### Code Style
- **Formatting**: Follow `.clang-format` (LLVM-based)
  - 4-space indentation (spaces, not tabs)
  - 100-character line limit
  - Opening braces on same line (`if (x) {`)
- **Spacing**: Space after control keywords (`if (`, `for (`, `while (`)
- **Pointers/References**: Attach to type (`int* ptr`, `const std::string& name`)

### Comments and Documentation
- **Public APIs**: Document purpose, parameters, return values, and any important notes
- **Complex algorithms**: Explain the approach and why it was chosen
- **Magic numbers**: Replace with named constants and explain meaning
- **TODOs**: Include context: `// TODO: Add validation when implementing issue #123`
- Prefer self-documenting code (good names) over comments explaining obvious things

### Modern C++ Practices
- Use `auto` when type is obvious from context: `auto it = map.find(key);`
- Range-based for loops: `for (const auto& item : collection)`
- Structured bindings: `auto [x, y] = get_coordinates();`
- `constexpr` for compile-time constants
- `nullptr` instead of `NULL` or `0`
- `enum class` instead of plain `enum`

### Testing Standards
- **Test naming**: `TEST(ClassName, DescriptiveTestName)`
- **Assertions**: Use Google Test macros (`EXPECT_EQ`, `ASSERT_TRUE`, etc.)
- **Setup/Teardown**: Use test fixtures for shared setup
- **Test data**: Keep test databases in-memory (`:memory:`) when possible
- **Coverage**: Test happy path, edge cases, and error conditions

### Python Integration Testing
- **Python tool**: Use `uv` for all Python operations
- **Installing dependencies**: `uv add <package-name>` (e.g., `uv add pyte`)
- **Running tests**: `uv run pytest <test-file>` or `uv run python <script>`
- **Integration tests**: Located in `tests/integration/` directory
- **Framework**: Uses `pyte` for terminal emulation to test actual TUI behavior
- **Important**: Unit tests that don't use the VTE/pyte framework are not meaningful for UI behavior - always write integration tests for UI features

### Performance Considerations
- **Viewport rendering**: Optimize for large datasets (1M+ rows)
- **Database queries**: Use proper indexes (already in schema)
- **String operations**: Avoid unnecessary copies, use `std::string_view` for read-only
- **Profile first**: Don't optimize prematurely, but design for efficiency

### Platform Compatibility
- **Terminal I/O**: Abstract platform differences (ncurses on Unix, Windows Console API)
- **File paths**: Use portable path handling (`std::filesystem` or simple strings)
- **Line endings**: Handle both LF and CRLF where needed
- **Endianness**: SQLite handles this, so no special consideration needed

### SQLite Best Practices
- **Prepared statements**: Always use for queries with user data (prevents SQL injection)
- **Transactions**: Wrap multiple writes in transactions for performance and atomicity
- **Error checking**: Always check return codes (`SQLITE_OK`, `SQLITE_DONE`, etc.)
- **Connection management**: One connection per database, close in destructor

### Code Review Checklist
Before committing:
- [ ] All tests pass (`ctest --output-on-failure`)
- [ ] Code follows naming conventions
- [ ] No compiler warnings (we use `-Werror`)
- [ ] Formatted with clang-format
- [ ] Public APIs documented
- [ ] Complex logic has explanatory comments
- [ ] No memory leaks (use smart pointers)
- [ ] Error cases handled appropriately

