# Terminal Resize Testing

This document describes how to test terminal resize functionality in DataPainter.

## Test Approaches

### 1. Unit Test (Automated) ✅

**File**: `tests/test_table_selection_menu.cpp::ResizeUpdatesDisplay`

This test verifies the application correctly handles resize events by:
- Simulating a terminal size change via `set_dimensions()`
- Verifying the display re-renders correctly at the new size
- Checking that content remains intact after resize

**Run it**:
```bash
cd build
ctest -R ResizeUpdatesDisplay
```

**Pros**: Fast, reliable, automated
**Cons**: Doesn't test actual SIGWINCH signal handling

---

### 2. Manual Debug Tool ✅

**File**: `build/test_resize_debug`

Interactive tool that shows:
- Current terminal size
- Environment variables (LINES, COLUMNS)
- Real-time KEY_RESIZE event detection
- Manual refresh with Ctrl-L

**Run it**:
```bash
./build/test_resize_debug
```

Then resize your terminal window and observe the output.

**Pros**: Tests actual signal handling, easy to diagnose issues
**Cons**: Requires manual interaction

---

### 3. PTY-Based Integration Test (Future)

For a fully automated test that actually resizes a terminal, you would:

1. Create a pseudo-terminal (pty) using `openpty()`
2. Fork a child process running the application
3. Use `ioctl(TIOCSWINSZ)` to resize the pty
4. Verify the child process receives and handles the resize

**Example skeleton**:

```cpp
#include <pty.h>
#include <sys/ioctl.h>
#include <unistd.h>

TEST(ResizeIntegration, DetectsActualResize) {
    int master, slave;
    char name[256];

    // Create pty
    ASSERT_EQ(0, openpty(&master, &slave, name, nullptr, nullptr));

    pid_t pid = fork();
    if (pid == 0) {
        // Child: run application on slave pty
        close(master);
        dup2(slave, STDIN_FILENO);
        dup2(slave, STDOUT_FILENO);
        dup2(slave, STDERR_FILENO);
        // Run application...
        exit(0);
    }

    // Parent: resize the pty
    struct winsize ws = {30, 100, 0, 0};
    ASSERT_EQ(0, ioctl(master, TIOCSWINSZ, &ws));

    // Verify application responds...

    close(master);
    waitpid(pid, nullptr, 0);
}
```

**Pros**: Tests complete signal chain
**Cons**: Complex, platform-specific, harder to debug

---

## Debugging Resize Issues on macOS

If KEY_RESIZE events aren't being detected:

1. **Check environment variables**:
   ```bash
   echo "LINES=${LINES:-unset}"
   echo "COLUMNS=${COLUMNS:-unset}"
   ```
   If these are set, ncurses may ignore actual terminal size changes.

2. **Run the debug tool**:
   ```bash
   ./build/test_resize_debug
   ```
   It will show whether LINES/COLUMNS are being unset and if KEY_RESIZE events arrive.

3. **Verify ncurses support**:
   ```bash
   ncursesw5-config --version  # or ncurses6-config
   ```
   Check if ncurses was compiled with SIGWINCH support.

4. **Alternative: Direct SIGWINCH handler**:
   If ncurses KEY_RESIZE doesn't work, implement a custom SIGWINCH handler (see web research notes).

---

## Current Status

- ✅ Unit test for resize logic
- ✅ Manual debug tool
- ✅ Ctrl-L manual refresh as workaround
- ⏳ PTY-based integration test (not yet implemented)
- ⚠️ KEY_RESIZE detection on macOS Terminal (may require SIGWINCH handler)

