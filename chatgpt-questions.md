Short version: Most critical gaps have been addressed in CLARIFICATIONS.md. Remaining items below are suggestions for future implementation details.

---

## 1) Rendering, ticks, zoom & pan (implementation details)

The zoom model is described in README.md lines 136-165 and CLARIFICATIONS.md section 10. Additional implementation details:

* **Tick algorithm:**
  * Let `range = x_high - x_low`.
  * Choose `step = 10^k * {1,2,5}` such that labels don't collide: `ceil(label_width_chars * approx_char_px / pixels_per_unit)`.
  * Minor ticks appear if there's ≥ 3 char between majors; tenths if ≥ 6.
  * For very small/large ranges, switch to scientific notation once `|k| ≥ 4`.

Publish constants (label spacing) so results are identical across machines.

---

## 2) Keyboard model & focus (implementation details)

Keyboard mappings are specified in README.md lines 39-55 and CLARIFICATIONS.md section 3. Additional implementation details:

* `Tab` cycles focus across header fields (db name, table name, target name, x/o meanings, valid ranges, action buttons) left‑to‑right, then into viewport.
* `Enter` activates a focused button or confirms an edited field.
* A help key (`?`) could bring up a cheatsheet showing all shortcuts.

Document the focus order and every accelerator shown on screen.

---

## 3) Table view semantics (implementation details)

Table view is described in README.md lines 57-62 and CLARIFICATIONS.md sections 5 and 10. Additional implementation detail:

* **Sort:** default by `id` (insertion order); toggle sort by any column.

---

## 4) Undo/redo semantics & crash recovery (future features)

Undo is described in README.md lines 121-135 and CLARIFICATIONS.md section 3. Additional suggestions for future implementation:

* **Undo/redo granularity:** one key action = one event (or batch when holding a key repeats; batch by 50ms coalescing).
* **Crash recovery:** on startup, detect unapplied events for this table; prompt: "Commit, discard, or review."

---

## 5) Concurrency & SQLite settings (future features)

* **WAL mode ON** per database; **busy_timeout** ≥ 5s.
* **Multiple processes:** support read‑share, single writer. If two TUIs open the same table, only the process that "Saves" first succeeds; the other must rebase (replay) its local `unsaved_changes` against the new table state.

---

## 6) CLI: validation & exit codes (implementation details)

CLI is described in README.md lines 65-92 and CLARIFICATIONS.md section 7. Additional implementation details:

* **Validation:**
  * Table names: `[A-Za-z0-9_]+` only; reject others.
  * Ranges: `min <= max`. If invalid, refuse to start (non‑interactive) or show inline error (interactive).
  * `--override-screen-height/width`: if larger than terminal *and* not dumping output, exit **64** with a clear message (from README line 118).
* **Exit codes:** 0 success; 2 bad args; 64 screen too small; 65 DB I/O error; 66 lock timeout; 67 CSV write error.
* **Non‑interactive commands** return machine‑readable messages on `stdout`; errors on `stderr`.

---

## 7) Non‑interactive behaviours & test hooks (implementation details)

Non-interactive mode is described in README.md lines 94-111. Additional implementation details:

* **`--to-csv` format:** RFC 4180, UTF‑8 with BOM optional (default off), headers on, ordered by `id`. Optional `--filter "x>=… AND …"`.
* **`--key-stroke-at-point x,y KEY`:** deterministic hit‑test (per CLARIFICATIONS.md section 1), then emit the same events as the UI.
* **`--dump-screen` / `--dump-edit-area-contents`:** monochrome ASCII, fixed width, includes a first line comment with viewport bounds and cursor pos for regression testing.

---

## 8) Accessibility, internationalisation & colour (implementation details)

* **Colour:** optional but recommended; ensure a high‑contrast, colour‑blind safe palette. Provide `--no-colour` and automatic detection when `NO_COLOR` env var is set.
* **Locale:** axis labels use `.` decimal point consistently (avoid locale‑dependent commas) to keep tests deterministic.

---

## 9) Platform & terminal specifics (implementation details)

Platform support is mentioned in README.md lines 202-204. Additional implementation details:

* Handle SIGWINCH (resize). If the screen becomes too small to render header + 3 rows, pause canvas rendering and show a status line prompting the user to enlarge.
* Treat wide characters as width 1 (no emoji in labels), or block them entirely—be explicit.

---

## 10) Performance guarantees (baseline targets)

* 1M rows table: pan/zoom keeps UI > 30 FPS for viewport rendering (because rendering is density‑based, not point‑based).
* Inserts/deletes in the viewport: amortised < 20 ms per action; Save (100k unapplied events): < 2 s with WAL.

(These are reasonable baselines; you can tune after profiling.)

---

## 11) Security & safety (implementation details)

* Always use parameterised SQL for dynamic table names by whitelisting against `metadata.table_name`.
* Refuse to open world‑writable DB files unless `--i-know-what-i-am-doing` is set (prevents accidental edits on temp/shared files).

---

## 12) Documentation: on‑screen hints & help (implementation details)

* A single‑keystroke `?` brings up a cheatsheet showing all shortcuts, plus current hit-test behavior, pan step %, and zoom level.
* `--help` prints exhaustive CLI help, including precedence rules and exit codes.

---

## 13) Suggested acceptance tests

These behaviors are specified in CLARIFICATIONS.md and README.md. Suggested tests:

1. **Erase semantics:** Given 3 rows at the same coordinate, pressing Space once removes all three (CLARIFICATIONS.md section 1).
2. **Viewport↔Table sync:** After filtering in table view (x∈[0,1], y∈[-2,3]), switching to canvas must set the viewport bounds tightly to those ranges (CLARIFICATIONS.md section 5).
3. **Tick stability:** For width 120 chars and x‑range 0..1000, tick labels never overlap; switching to width 80 recomputes steps using the published rule.
4. **CLI conflict detection:** If `--table` exists and CLI provides conflicting metadata values, the program must error and refuse to start (CLARIFICATIONS.md section 7).
5. **CSV determinism:** `--to-csv` then re‑importing the file reproduces the same row count and coordinates bit‑for‑bit.

---

## Summary

Most critical specification ambiguities identified in the original ChatGPT analysis have been addressed in CLARIFICATIONS.md:
- ✅ Duplicate point semantics and erasure behavior (section 1)
- ✅ Target column storage and display logic (section 2)
- ✅ Keyboard mappings including flip and undo/redo (section 3)
- ✅ Viewport behavior for empty vs populated tables (section 4)
- ✅ Table view editing capabilities and sync (section 5)
- ✅ Save behavior (section 6)
- ✅ CLI argument conflict resolution (section 7)
- ✅ Database schema (section 8)
- ✅ Zero-axis bars behavior (section 9)

The remaining items in this document are implementation details and suggestions for future features that can be addressed during development.

