#!/usr/bin/env python3
"""Test table creation flow."""

import os
import sqlite3
import subprocess
import tempfile


def test_create_table():
    """Test creating a table through the dialog."""
    # Create temporary database
    with tempfile.NamedTemporaryFile(suffix=".db", delete=False) as f:
        db_path = f.name

    try:
        # Prepare input:
        # 1. Press Enter to select "Create new table" from menu
        # 2. Fill in form fields
        # 3. Press Ctrl+O to submit
        # 4. Press 'q' to quit from main TUI
        input_data = (
            b"\n"  # Select "Create new table"
            b"test_table\t"  # Table name + Tab
            b"label\t"  # Target column + Tab
            b"x\t"  # X-axis + Tab
            b"y\t"  # Y-axis + Tab
            b"positive\t"  # X meaning + Tab
            b"negative\t"  # O meaning + Tab
            b"\t\t\t\t"  # Skip numeric fields (use defaults)
            b"\x0f"  # Ctrl+O to submit
            # Now in main TUI, quit
            b"q"  # Quit
            b"y"  # Confirm (if prompted)
        )

        # Run datapainter with input
        proc = subprocess.Popen(
            ["./build/datapainter", "--database", db_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

        try:
            stdout, stderr = proc.communicate(input=input_data, timeout=3)
        except subprocess.TimeoutExpired:
            proc.kill()
            stdout, stderr = proc.communicate()
            print("Process timed out")
            return False

        # Verify the table was created
        if not os.path.exists(db_path):
            print("Database file was not created")
            return False

        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()

        # Check metadata table
        cursor.execute(
            "SELECT table_name, target_col_name, x_axis_name, y_axis_name, "
            "x_meaning, o_meaning FROM metadata WHERE table_name = 'test_table'"
        )
        row = cursor.fetchone()

        if row is None:
            print("Table metadata not found in database")
            conn.close()
            return False

        # Verify all fields
        errors = []
        if row[0] != "test_table":
            errors.append(f"table_name: expected 'test_table', got '{row[0]}'")
        if row[1] != "label":
            errors.append(f"target_col_name: expected 'label', got '{row[1]}'")
        if row[2] != "x":
            errors.append(f"x_axis_name: expected 'x', got '{row[2]}'")
        if row[3] != "y":
            errors.append(f"y_axis_name: expected 'y', got '{row[3]}'")
        if row[4] != "positive":
            errors.append(f"x_meaning: expected 'positive', got '{row[4]}'")
        if row[5] != "negative":
            errors.append(f"o_meaning: expected 'negative', got '{row[5]}'")

        if errors:
            print("Validation errors:")
            for error in errors:
                print(f"  - {error}")
            conn.close()
            return False

        # Check data table exists
        cursor.execute(
            "SELECT name FROM sqlite_master WHERE type='table' AND name='test_table'"
        )
        table_row = cursor.fetchone()

        if table_row is None:
            print("Data table 'test_table' was not created")
            conn.close()
            return False

        conn.close()

        print("✓ Table creation flow test passed")
        print(f"  Table 'test_table' created successfully with all expected fields")
        return True

    finally:
        # Clean up
        if os.path.exists(db_path):
            os.unlink(db_path)


if __name__ == "__main__":
    success = test_create_table()
    exit(0 if success else 1)
