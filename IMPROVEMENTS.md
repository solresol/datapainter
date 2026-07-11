# DataPainter — Improvements & Recommendations

*Analysis date: 2026-07-11*

DataPainter is a mature C++17 ncurses TUI for painting two-dimensional labelled datasets into SQLite, with solid packaging (signed APT repo, macOS .pkg, GitHub Actions CI) and a comprehensive TDD-driven test suite. The core feature set is essentially complete (Phases 0–onwards in TODO.md are nearly all ticked). The last ~15 commits are entirely Haiku cross-compilation debugging, and that effort is stalled on an upstream toolchain issue — the project's real remaining work is the small set of unfinished features in TODO.md and some repo hygiene.

## Bugs & Fixes

- **Automatic terminal resize is broken** (last item in TODO.md; Ctrl-L is the documented workaround). SIGWINCH handling in `src/terminal.cpp` / main loop in `src/main.cpp` should trigger a re-layout automatically. This is the most user-visible defect.
- **Database lock timeout (exit code 66) is documented but not implemented** — TODO.md flags "Feature not yet implemented". Either implement it (SQLite `busy_timeout` + WAL, see below) or remove it from docs/help so behaviour matches documentation.
- **Crash recovery prompt on startup** for unapplied `unsaved_changes` rows is unimplemented; the `unsaved_changes` table exists (`include/unsaved_changes.h`) but orphaned changes are silently ignored on restart.

## Unfinished Work (from TODO.md, 14 open items)

Highest-value first:
1. **Multi-process safety**: enable WAL mode + `busy_timeout` in `src/database.cpp`, define conflict resolution. Two concurrent datapainter instances on one file can currently corrupt or block each other.
2. **CSV export** (RFC 4180, BOM option) — the natural output path for a dataset-creation tool; its absence forces users through sqlite3 CLI.
3. **Table view sorting** and **inline cell editing** (backend reportedly complete, UI missing — `include/table_view.h` / `point_editor.h`).
4. **Undo coalescing (50ms) for key repeats** — held-key painting currently floods the undo log (`undo_log_manager.cpp`).
5. **Performance: 30 FPS at 1M rows** — needs a benchmark before it needs optimization; add a `tests/` perf harness using the random initializer.

## Haiku Port: Decide, Don't Drift

The last 15+ commits are debug/fix churn for Haiku cross-compilation, and `HAIKU_BUILD_STATUS.md` says the build is blocked on an upstream `haiku-toolchains-ubuntu` bug. Recommendations:
- Timebox it: mark `ci-haiku.yml` as `continue-on-error`/manual-dispatch-only so a permanently red Haiku job doesn't desensitize you to real CI failures.
- Squash-summarize the learning in HAIKU_BUILD_STATUS.md (already good) and file the upstream issue if you haven't; revisit quarterly rather than per-push.
- Consider building natively on a Haiku VM/nightly instead of cross-compiling — the cross toolchain has been the entire cost.

## Housekeeping

- **Committed test artifacts**: `test.db`, `test_borders.db`, `test_menu.db`, `test_validation.db` sit in the repo root, and a `Testing/` (CTest output) directory and `build/` directory exist in the working tree. Verify none are tracked (`git ls-files` shows `Testing/Temporary` implicated); add `*.db`, `Testing/`, `build/` to `.gitignore` and remove strays. Tests should create databases in a temp dir, not the repo root.
- **Debug commits polluting history**: the "debug:" commit series is fine for a solo project, but consider squash-merging Haiku work on a branch next time.
- `PACKAGING_TODO.md` and `HAIKU_BUILD_STATUS.md` overlap with TODO.md — consider consolidating status docs so there is one source of truth.

## Testing

- The unit test coverage is strong; the acknowledged gap is **blocking-UI tests** (help overlay, dialogs) which the PTY framework can't drive. Consider adding a scriptable input mode (the `input_source.h` abstraction is already there) that lets tests inject keys past a blocking read.
- Add a CI job that runs the binary headless against a golden dump (TODO.md mentions "test hooks: viewport bounds in dump output" — finish that hook; it makes end-to-end assertions cheap).

## Documentation

- README installation section is excellent. Add a short **usage/quickstart** section with a screenshot or asciinema cast of actually painting points — the README currently explains how to install a TUI without showing what it looks like.
- Document exit codes (including the planned 66) in the man page and keep them in sync with `src/main.cpp`.

## Security

- No committed secrets found. GPG-signed APT repo with fingerprint-verification instructions is good practice.
- World-writable database warning is implemented — good. When adding WAL mode, ensure `-wal`/`-shm` files inherit the main db's permissions.

## Quick Wins

1. Fix `.gitignore` and delete stray `*.db` / `Testing/` artifacts (5 minutes).
2. Make the Haiku CI job non-blocking (`continue-on-error: true`).
3. Implement `PRAGMA busy_timeout` + WAL — two lines in `database.cpp` that close two TODO items.
4. Wire SIGWINCH to the existing Ctrl-L redraw path to fix auto-resize.
