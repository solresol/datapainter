#!/usr/bin/env python3
"""Integration test for table creation dialog."""

import os
import sqlite3
import subprocess
import tempfile
import time
import pyte


def test_table_creation_dialog():
    """Test creating a table through the TUI dialog."""
    # Create temporary database
    with tempfile.NamedTemporaryFile(suffix=".db", delete=False) as f:
        db_path = f.name

    try:
        # Set up terminal emulator
        screen = pyte.Screen(80, 40)
        stream = pyte.ByteStream(screen)

        # Start datapainter
        proc = subprocess.Popen(
            ["./build/datapainter", "--database", db_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

        # Give it a moment to start
        time.sleep(0.5)

        # Navigate to "Create new table" in menu (press 'c' or down arrow and enter)
        # The menu should have options, and 'c' might be a shortcut
        # Let's try sending 'c' for create
        proc.stdin.write(b"c")
        proc.stdin.flush()
        time.sleep(0.3)

        # Now we should be in the table creation dialog
        # Fill in the fields:
        # 1. Table name
        proc.stdin.write(b"test_table")
        proc.stdin.flush()
        time.sleep(0.1)

        # Move to next field (Tab or Down arrow)
        proc.stdin.write(b"\t")
        proc.stdin.flush()
        time.sleep(0.1)

        # 2. Target column name
        proc.stdin.write(b"label")
        proc.stdin.flush()
        time.sleep(0.1)

        # Next field
        proc.stdin.write(b"\t")
        proc.stdin.flush()
        time.sleep(0.1)

        # 3. X-axis name
        proc.stdin.write(b"x")
        proc.stdin.flush()
        time.sleep(0.1)

        # Next field
        proc.stdin.write(b"\t")
        proc.stdin.flush()
        time.sleep(0.1)

        # 4. Y-axis name
        proc.stdin.write(b"y")
        proc.stdin.flush()
        time.sleep(0.1)

        # Next field
        proc.stdin.write(b"\t")
        proc.stdin.flush()
        time.sleep(0.1)

        # 5. X meaning
        proc.stdin.write(b"positive")
        proc.stdin.flush()
        time.sleep(0.1)

        # Next field
        proc.stdin.write(b"\t")
        proc.stdin.flush()
        time.sleep(0.1)

        # 6. O meaning
        proc.stdin.write(b"negative")
        proc.stdin.flush()
        time.sleep(0.1)

        # Skip numeric fields (they have defaults) by pressing Tab
        proc.stdin.write(b"\t")
        proc.stdin.flush()
        time.sleep(0.1)
        proc.stdin.write(b"\t")
        proc.stdin.flush()
        time.sleep(0.1)
        proc.stdin.write(b"\t")
        proc.stdin.flush()
        time.sleep(0.1)
        proc.stdin.write(b"\t")
        proc.stdin.flush()
        time.sleep(0.1)

        # Press Ctrl+O to submit (ASCII 15)
        proc.stdin.write(b"\x0f")
        proc.stdin.flush()
        time.sleep(0.5)

        # Now we should be in the interactive TUI with the new table
        # Exit by pressing 'q'
        proc.stdin.write(b"q")
        proc.stdin.flush()
        time.sleep(0.3)

        # Confirm quit (if there are no changes, it should exit immediately)
        proc.stdin.write(b"y")
        proc.stdin.flush()

        # Wait for process to exit
        proc.wait(timeout=2)

        # Verify the table was created in the database
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()

        # Check metadata table
        cursor.execute(
            "SELECT table_name, target_col_name, x_axis_name, y_axis_name, "
            "x_meaning, o_meaning FROM metadata WHERE table_name = 'test_table'"
        )
        row = cursor.fetchone()

        assert row is not None, "Table metadata not found"
        assert row[0] == "test_table", f"Expected table_name='test_table', got '{row[0]}'"
        assert row[1] == "label", f"Expected target_col_name='label', got '{row[1]}'"
        assert row[2] == "x", f"Expected x_axis_name='x', got '{row[2]}'"
        assert row[3] == "y", f"Expected y_axis_name='y', got '{row[3]}'"
        assert row[4] == "positive", f"Expected x_meaning='positive', got '{row[4]}'"
        assert row[5] == "negative", f"Expected o_meaning='negative', got '{row[5]}'"

        # Check data table exists
        cursor.execute(
            "SELECT name FROM sqlite_master WHERE type='table' AND name='test_table'"
        )
        table_row = cursor.fetchone()
        assert table_row is not None, "Data table not created"

        conn.close()

        print("✓ Table creation dialog test passed")

    finally:
        # Clean up
        if os.path.exists(db_path):
            os.unlink(db_path)


if __name__ == "__main__":
    test_table_creation_dialog()
