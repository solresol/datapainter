# Building DataPainter

Quick reference for building and testing DataPainter.

## Prerequisites

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install cmake g++ libsqlite3-dev libncurses-dev
```

### macOS
```bash
brew install cmake sqlite3
```

### Windows
```powershell
vcpkg install sqlite3:x64-windows
```

## Build Steps

```bash
# 1. Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 2. Build
cmake --build build

# 3. Run tests
cd build
ctest --output-on-failure

# 4. Run the program
./datapainter --help
```

## Development Build (Debug)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cd build && ctest --output-on-failure
```

## Code Formatting

Before committing, format your code:

```bash
find src include tests -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

## Running Specific Tests

```bash
cd build
./datapainter_tests --gtest_filter=DatabaseTest.*
```

## Clean Build

```bash
rm -rf build
cmake -B build
cmake --build build
```

## IDE Integration

The build generates `compile_commands.json` for LSP/IDE integration (VS Code, CLion, etc.).

## Troubleshooting

**SQLite3 not found on Linux:**
```bash
sudo apt-get install libsqlite3-dev
```

**CMake version too old:**
Download newer CMake from https://cmake.org/download/

**Tests failing:**
Make sure you're in the build directory when running ctest.

See CONTRIBUTING.md for detailed development workflow.
