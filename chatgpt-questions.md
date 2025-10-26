Short version: Most critical gaps have been addressed and merged into README.md. Remaining items below are suggestions for future implementation details not currently specified.

---

## 1) Table view sort functionality (implementation detail)

Table view is described in README.md. Additional implementation detail:

* **Sort:** default by `id` (insertion order); toggle sort by any column.

---

## 2) Undo/redo granularity & crash recovery (future features)

Undo/redo is described in README.md. Additional suggestions for future implementation:

* **Undo/redo granularity:** one key action = one event (or batch when holding a key repeats; batch by 50ms coalescing).
* **Crash recovery:** on startup, detect unapplied events for this table; prompt: "Commit, discard, or review."

---

## 3) Multi-process concurrency (future feature)

README.md specifies simple single-process mode only. Future multi-process support could include:

* **WAL mode ON** per database; **busy_timeout** ≥ 5s.
* **Multiple processes:** support read‑share, single writer. If two TUIs open the same table, only the process that "Saves" first succeeds; the other must rebase (replay) its local `unsaved_changes` against the new table state.

---

## 4) CSV export enhancements (future features)

README.md specifies basic CSV export. Future enhancements could include:

* **`--to-csv` format:** RFC 4180 compliance, optional BOM support, optional `--filter "x>=… AND …"` for selective export.

---

## 5) Test hook details (implementation detail)

Non-interactive mode test hooks are described in README.md. Additional implementation detail:

* **`--dump-screen` / `--dump-edit-area-contents`:** could include a first line comment with viewport bounds and cursor pos for regression testing.

---

## 6) Performance baseline targets (future feature)

README.md specifies "best effort, no specific targets." Future performance targets could include:

* 1M rows table: pan/zoom keeps UI > 30 FPS for viewport rendering (because rendering is density‑based, not point‑based).
* Inserts/deletes in the viewport: amortised < 20 ms per action; Save (100k unapplied events): < 2 s with WAL.

(These are reasonable baselines; tune after profiling.)

---

## Summary

All critical specification gaps have been addressed and merged into README.md in the "Implementation Details & Clarifications" section:
- ✅ Duplicate point semantics and erasure behavior
- ✅ Target column storage and display logic
- ✅ Keyboard mappings including flip and undo/redo
- ✅ Viewport behavior for empty vs populated tables
- ✅ Table view editing capabilities and sync
- ✅ Save behavior
- ✅ CLI argument conflict resolution
- ✅ Database schema
- ✅ Zero-axis bars behavior
- ✅ Concurrency model (single-process)
- ✅ Color/accessibility (monochrome only)
- ✅ Security & file permissions
- ✅ Performance approach (best effort)
- ✅ Help system (? key)
- ✅ Exit codes
- ✅ CSV export format
- ✅ Terminal resize handling
- ✅ Table name validation
- ✅ Decimal point display
- ✅ Wide character handling
- ✅ Validation rules
- ✅ Tick algorithm
- ✅ Focus order
- ✅ Parameterized SQL

The remaining items in this document are optional implementation details and suggestions for future features that can be addressed during development as needed.

