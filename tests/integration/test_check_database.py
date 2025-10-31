"""
Test to check what's in the database after deletion.
"""

import sqlite3
import time
import tempfile
import os
from tui_test_framework import DataPainterTest


def test_check_unsaved_changes_table():
    """Check what's actually in unsaved_changes table after deletion."""
    # Create a persistent temp database so we can examine it
    fd, temp_db = tempfile.mkstemp(suffix=".db")
    os.close(fd)

    try:
        with DataPainterTest(width=80, height=24, database_path=temp_db) as test:
            # Wait for UI
            test.wait_for_text('test_table', timeout=3.0)

            # Create a point
            test.send_keys('x')
            time.sleep(0.3)

            # Check database before deletion
            conn = sqlite3.connect(temp_db)
            cursor = conn.cursor()

            print("\n=== After creating point 'x' ===")
            cursor.execute("SELECT * FROM unsaved_changes")
            for row in cursor.fetchall():
                print(row)

            conn.close()

            # Delete the point
            test.send_keys('BACKSPACE')
            time.sleep(0.3)

            # Check database after deletion
            conn = sqlite3.connect(temp_db)
            cursor = conn.cursor()

            print("\n=== After pressing BACKSPACE ===")
            cursor.execute("SELECT id, action, data_id, x, y, new_target, is_active FROM unsaved_changes")
            for row in cursor.fetchall():
                print(f"id={row[0]}, action={row[1]}, data_id={row[2]}, x={row[3]}, y={row[4]}, target={row[5]}, is_active={row[6]}")

            conn.close()
    finally:
        os.unlink(temp_db)


if __name__ == '__main__':
    test_check_unsaved_changes_table()
