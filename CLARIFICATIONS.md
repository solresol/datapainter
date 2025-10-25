# CLARIFICATIONS.md

This document clarifies implementation details and design decisions for items identified in chatgpt-questions.md that were not fully specified in README.md.

---

## 1. Concurrency & SQLite Configuration

**Decision:** Simple single-process mode only

- No SQLite WAL mode required
- No busy_timeout configuration needed
- Assume single process access to each database
- No multi-process conflict detection or handling
- Simpler implementation without concurrency complexity

---

## 2. Color & Accessibility

**Decision:** Monochrome mode only (no colors)

- No color support in initial version
- Use only ASCII characters for rendering
- No --no-colour flag needed
- No NO_COLOR environment variable support
- Keep implementation simple with monochrome output

---

## 3. Security & File Permissions

**Decision:** Warn but allow world-writable files

- Check file permissions on database open
- If database file is world-writable, display a warning message
- Allow operation to proceed despite warning
- No --i-know-what-i-am-doing flag required
- User is responsible for file security management

---

## 4. Performance Requirements

**Decision:** Best effort, no specific targets

- Optimize as needed during development
- No hard performance requirements (FPS, latency, save time)
- Profile and improve performance issues as they arise
- No baseline targets for 1M row tables

---

## 5. Help System

**Decision:** '?' key shows keyboard shortcuts and current state

- Pressing '?' brings up an in-app help cheatsheet overlay
- Display all keyboard shortcuts
- Show current zoom level
- Show pan step percentage
- Show current hit-test behavior
- Dismiss help overlay to return to normal operation

---

## 6. Exit Codes

**Decision:** Detailed exit codes for different error conditions

Standard exit codes:
- **0** - Success
- **2** - Invalid command-line arguments
- **64** - Screen too small (when --override-screen-height/width exceeds terminal size)
- **65** - Database I/O error
- **66** - Database lock timeout
- **67** - CSV write error

Non-interactive commands return machine-readable messages on stdout; errors on stderr.

---

## 7. CSV Export Format (--to-csv)

**Decision:** Basic CSV output, no special options

- Simple CSV format: headers + data rows
- Rows ordered by id (insertion order)
- UTF-8 encoding
- No optional BOM support
- No --filter support for selective export
- Format: three columns (x, y, target) with header row

---

## 8. Terminal Resize Handling (SIGWINCH)

**Decision:** Handle SIGWINCH with small-screen fallback

- Detect terminal resize events (SIGWINCH)
- Re-render UI at new terminal size
- If screen becomes too small to render header + 3 rows:
  - Pause canvas rendering
  - Show a status line prompting user to enlarge terminal
- Resume normal rendering when adequate size restored

---

## 9. Table Name Validation

**Decision:** Restrict to [A-Za-z0-9_]+ only

- Table names must match regex: `[A-Za-z0-9_]+`
- Alphanumeric characters and underscores only
- Reject table names with spaces, special characters, or other symbols
- Display clear error message for invalid table names
- Non-interactive mode: exit with code 2
- Interactive mode: show inline error and prompt again

---

## 10. Decimal Point Display (Locale)

**Decision:** Use default C++ formatting (printf/iostream)

- Let standard library decide decimal separator
- Use standard printf or iostream formatting for axis labels
- No explicit locale override
- No forced '.' period regardless of locale

---

## 11. Wide Character Handling

**Decision:** Block wide characters entirely

- Do not support emoji or multi-byte wide characters
- Strip or reject wide characters from input
- Ensure terminal alignment with fixed-width ASCII only
- Prevents rendering issues with variable-width characters

---

## 12. Validation Rules Summary

### Range Validation
- Valid ranges: `min <= max` required
- Invalid ranges:
  - Non-interactive: exit with code 2
  - Interactive: show inline error message

### CLI Conflict Resolution
- If CLI arguments conflict with existing metadata:
  - Program errors and refuses to start
  - Display clear error showing the conflict
  - Suggest resolution (remove flag or use different table name)

---

## 13. Implementation Notes

### Tick Algorithm (from chatgpt-questions.md)
- Let `range = x_high - x_low`
- Choose `step = 10^k * {1,2,5}` to prevent label collision
- Minor ticks appear if ≥ 3 characters between majors
- Tenth ticks if ≥ 6 characters between majors
- Scientific notation for `|k| ≥ 4`

### Focus Order (Tab key)
Tab cycles through header fields left-to-right:
1. Database name
2. Table name
3. Target name
4. X/O meanings
5. Valid ranges
6. Action buttons (Tabular view, Undo, Quit, Save)
7. Then into viewport

Enter key activates focused button or confirms edited field.

### Parameterized SQL
- Always use parameterized queries for dynamic values
- For table names: whitelist against metadata.table_name
- Prevents SQL injection vulnerabilities

---

## Notes on Test Coverage

Acceptance test scenarios are implementation details and not specified as requirements. Developers should design appropriate tests during development following TDD practices as specified in CLAUDE.md.
