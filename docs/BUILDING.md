# Building DataPainter

## Prerequisites

### Required

- **C++ Compiler** with C++17 support:
  - GCC 7+ or Clang 5+ on Linux/macOS
  - MSVC 2017+ on Windows (not yet fully supported)
- **CMake** 3.10 or higher
- **SQLite3** development libraries
- **Make** or **Ninja** build system

### Optional (for testing)

- **Python 3.7+** (for integration tests)
- **uv** (Python package manager) - recommended, or pip
- **pytest** (Python testing framework)
- **pyte** (VTE emulator for TUI testing)

## Quick Start

### Linux / macOS

```bash
# Clone the repository
git clone https://github.com/solresol/datapainter.git
cd datapainter

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build (using 4 parallel jobs)
make -j4

# Run unit tests
ctest --output-on-failure

# Run integration tests (from project root)
cd ..
uv run pytest tests/integration/ -v
```

### Installing Dependencies

#### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libsqlite3-dev \
    python3 \
    python3-pip

# Install uv for Python testing
pip3 install uv
```

#### macOS (with Homebrew)

```bash
brew install cmake sqlite3

# Install uv for Python testing
brew install uv
```

#### Fedora/RHEL

```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    sqlite-devel \
    python3 \
    python3-pip

pip3 install uv
```

## Build Process Details

### CMake Configuration

The CMakeLists.txt defines:

- **C++17 standard** (required for `std::optional`, `std::string_view`)
- **Compiler warnings** as errors (`-Werror`)
- **SQLite3 linkage**
- **Google Test** framework (fetched automatically via FetchContent)
- Two build targets:
  - `datapainter`: Main executable
  - `datapainter_tests`: Unit test executable

### Build Options

```bash
# Debug build (with symbols)
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j4

# Release build (optimized)
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4

# Specify custom SQLite3 path
cmake -DSQLITE3_ROOT=/usr/local ..
make -j4
```

### Build Artifacts

After building, you'll have:

```
build/
├── datapainter           # Main executable
├── datapainter_tests     # Unit tests executable
└── _deps/                # Google Test (auto-downloaded)
```

## Running Tests

### Unit Tests (C++ / Google Test)

```bash
# From build directory
cd build

# Run all tests
ctest --output-on-failure

# Run with verbose output
ctest -V

# Run specific test
./datapainter_tests --gtest_filter=ViewportTest.ScreenToData

# List all tests
./datapainter_tests --gtest_list_tests
```

### Integration Tests (Python / pytest)

```bash
# From project root
cd /path/to/datapainter

# Install test dependencies (with uv)
uv add pytest pyte

# Run all integration tests
uv run pytest tests/integration/ -v

# Run specific test file
uv run pytest tests/integration/test_basic_operations.py -v

# Run specific test
uv run pytest tests/integration/test_basic_operations.py::TestApplicationStartup::test_startup_shows_table_name -v

# Run with verbose output
uv run pytest tests/integration/ -vv
```

## Installation

### System-Wide Installation (Linux/macOS)

```bash
# After building
cd build
sudo make install

# This installs:
# - /usr/local/bin/datapainter (executable)
# - /usr/local/share/man/man1/datapainter.1 (man page)
```

### Local Installation

```bash
# Install to custom prefix
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
make -j4
make install

# Add to PATH
export PATH="$HOME/.local/bin:$PATH"

# View man page
export MANPATH="$HOME/.local/share/man:$MANPATH"
man datapainter
```

## Development Workflow

### 1. Make Code Changes

Edit files in `src/` or `include/`:

```bash
# Example: Edit viewport.cpp
vim src/viewport.cpp
```

### 2. Rebuild

```bash
cd build
make -j4  # Incremental build (only changed files)
```

### 3. Run Tests

```bash
# Run C++ unit tests
ctest --output-on-failure

# Run Python integration tests
cd ..
uv run pytest tests/integration/ -v
```

### 4. Test Manually

```bash
# Run datapainter
./build/datapainter --help

# Create a test database
./build/datapainter --database test.db --create-table --table test \
  --target-column-name label --x-axis-name x --y-axis-name y \
  --x-meaning positive --o-meaning negative

# Open in interactive mode
./build/datapainter --database test.db --table test
```

## Troubleshooting

### CMake Can't Find SQLite3

```bash
# On Ubuntu/Debian
sudo apt-get install libsqlite3-dev

# On macOS
brew install sqlite3

# Specify custom location
cmake -DSQLITE3_ROOT=/path/to/sqlite3 ..
```

### Google Test Download Fails

Check internet connection. Google Test is auto-downloaded via CMake FetchContent.

If behind a proxy:
```bash
export https_proxy=http://proxy.example.com:8080
cmake ..
```

### Compiler Warnings Treated as Errors

If you see `-Werror` issues:

```bash
# Temporary workaround (not recommended)
cmake -DCMAKE_CXX_FLAGS="-Wno-error" ..

# Better: Fix the warnings!
```

### Integration Tests Fail

Ensure Python dependencies are installed:

```bash
# With uv
uv add pytest pyte

# Or with pip
pip install pytest pyte
```

Ensure datapainter binary exists:
```bash
# Build first
cd build && make -j4 && cd ..

# Verify binary
ls -la build/datapainter
```

### Tests Timeout

Integration tests have a 10-second timeout. If tests are slow:

```bash
# Increase timeout in tests/integration/pyproject.toml
# timeout = 30  # seconds
```

## Platform-Specific Notes

### macOS

- Uses system SQLite3 (usually in `/usr/lib`)
- Terminal rendering works via ANSI escape codes
- Tested on macOS Monterey (12.x) and later

### Linux

- Tested on Ubuntu 20.04, 22.04
- Tested on Fedora 35+
- Should work on any Linux with C++17 compiler

### Windows

- **Not fully supported yet**
- Terminal abstraction needs Windows Console API implementation
- Contributions welcome!

## Continuous Integration

### GitHub Actions (if configured)

```yaml
# Example .github/workflows/build.yml
name: Build and Test
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake libsqlite3-dev
      - name: Build
        run: |
          mkdir build && cd build
          cmake ..
          make -j4
      - name: Test
        run: |
          cd build
          ctest --output-on-failure
```

## Performance Profiling

### With gprof

```bash
# Build with profiling
cmake -DCMAKE_CXX_FLAGS="-pg" ..
make -j4

# Run application
./build/datapainter --database test.db --table test

# Generate profile
gprof build/datapainter gmon.out > analysis.txt
```

### With Valgrind (memory leaks)

```bash
# Build with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j4

# Run with valgrind
valgrind --leak-check=full --show-leak-kinds=all \
  ./build/datapainter --database test.db --list-tables
```

## Code Formatting

DataPainter follows LLVM coding style (see CLAUDE.md).

### With clang-format

```bash
# Format all source files
find src include -name '*.cpp' -o -name '*.h' | xargs clang-format -i

# Check formatting (without modifying)
find src include -name '*.cpp' -o -name '*.h' | xargs clang-format --dry-run -Werror
```

## Documentation Generation

### Man Page

The man page is already written in groff format (`docs/datapainter.1`).

To view:
```bash
man ./docs/datapainter.1
```

To install:
```bash
sudo cp docs/datapainter.1 /usr/local/share/man/man1/
sudo mandb  # Update man page database
```

## Further Reading

- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture and design
- [../README.md](../README.md) - Project overview and usage
- [../TODO.md](../TODO.md) - Development roadmap
- [../CLAUDE.md](../CLAUDE.md) - Coding standards
