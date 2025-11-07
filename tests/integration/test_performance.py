#!/usr/bin/env python3
"""Performance tests for DataPainter with large datasets.

Tests verify that DataPainter can handle large datasets (1M+ rows)
with acceptable performance for viewport rendering and interactions.
"""

import os
import subprocess
import tempfile
import time
import sqlite3
from pathlib import Path
import pytest


def create_large_dataset(db_path, table_name, num_rows=1000000):
    """Create a database with a large number of points.

    Args:
        db_path: Path to database file
        table_name: Name of table to create
        num_rows: Number of rows to insert

    Returns:
        Time taken to create dataset in seconds
    """
    # Get path to datapainter executable
    repo_root = Path(__file__).parent.parent.parent
    datapainter_exe = repo_root / "build" / "datapainter"

    # Create table
    result = subprocess.run([
        str(datapainter_exe),
        "--database", db_path,
        "--create-table",
        "--table", table_name,
        "--target-column-name", "label",
        "--x-axis-name", "x",
        "--y-axis-name", "y",
        "--x-meaning", "positive",
        "--o-meaning", "negative",
        "--min-x", "-100",
        "--max-x", "100",
        "--min-y", "-100",
        "--max-y", "100"
    ], capture_output=True, text=True)

    if result.returncode != 0:
        raise RuntimeError(f"Failed to create table: {result.stderr}")

    # Measure time to insert rows directly using SQLite
    # This is faster and more realistic for testing large datasets
    start_time = time.time()

    import random
    random.seed(42)  # Reproducible results

    conn = sqlite3.connect(db_path)
    cursor = conn.cursor()

    # Insert in batches for better performance
    batch_size = 10000
    half = num_rows // 2

    # Insert positive points
    for batch_start in range(0, half, batch_size):
        batch = []
        for i in range(batch_size):
            if batch_start + i >= half:
                break
            # Normal distribution centered at 0 with std=30
            x = random.gauss(0, 30)
            y = random.gauss(0, 30)
            # Clamp to valid range
            x = max(-100, min(100, x))
            y = max(-100, min(100, y))
            batch.append((x, y, "positive"))

        cursor.executemany(f"INSERT INTO {table_name} (x, y, target) VALUES (?, ?, ?)", batch)

    # Insert negative points
    for batch_start in range(0, half, batch_size):
        batch = []
        for i in range(batch_size):
            if batch_start + i >= half:
                break
            # Normal distribution centered at 0 with std=30
            x = random.gauss(0, 30)
            y = random.gauss(0, 30)
            # Clamp to valid range
            x = max(-100, min(100, x))
            y = max(-100, min(100, y))
            batch.append((x, y, "negative"))

        cursor.executemany(f"INSERT INTO {table_name} (x, y, target) VALUES (?, ?, ?)", batch)

    conn.commit()
    conn.close()

    elapsed = time.time() - start_time

    return elapsed


@pytest.mark.timeout(120)  # 2 minutes timeout
def test_1m_row_creation():
    """Test creating and populating a table with 1M rows."""
    # Create temporary database
    fd, db_path = tempfile.mkstemp(suffix='.db')
    os.close(fd)

    try:
        print("\nCreating dataset with 1M rows...")
        elapsed = create_large_dataset(db_path, "perf_test", 1000000)

        print(f"  Created 1M rows in {elapsed:.2f} seconds")

        # Verify row count
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        cursor.execute("SELECT COUNT(*) FROM perf_test")
        count = cursor.fetchone()[0]
        conn.close()

        assert count == 1000000, f"Expected 1M rows, got {count}"
        print(f"  ✓ Verified {count:,} rows in database")

    finally:
        if os.path.exists(db_path):
            os.unlink(db_path)


@pytest.mark.timeout(120)  # 2 minutes timeout
def test_1m_row_viewport_rendering():
    """Test viewport rendering performance with 1M rows.

    This test creates a 1M row dataset and then performs several
    viewport operations to verify acceptable performance:
    - Initial render
    - Cursor movement
    - Zoom operations
    - Pan operations
    """
    # Create temporary database
    fd, db_path = tempfile.mkstemp(suffix='.db')
    os.close(fd)

    try:
        print("\nSetting up 1M row dataset for viewport testing...")
        create_large_dataset(db_path, "perf_test", 1000000)

        # Create keystroke file to test viewport operations
        fd, keystroke_file = tempfile.mkstemp(suffix='.txt')
        with os.fdopen(fd, 'w') as f:
            f.write("# Performance test keystrokes\n")
            # Move cursor around
            f.write("<right>\n")
            f.write("<right>\n")
            f.write("<up>\n")
            f.write("<up>\n")
            # Zoom in
            f.write("+\n")
            # Move some more
            f.write("<left>\n")
            f.write("<down>\n")
            # Zoom out
            f.write("-\n")
            # Full zoom
            f.write("=\n")
            # Dump screen to verify render
            f.write("k\n")
            # Quit
            f.write("q\n")

        # Get path to datapainter executable
        repo_root = Path(__file__).parent.parent.parent
        datapainter_exe = repo_root / "build" / "datapainter"

        # Run viewport operations and measure time
        print("  Running viewport operations...")
        start_time = time.time()

        result = subprocess.run([
            str(datapainter_exe),
            "--database", db_path,
            "--table", "perf_test",
            "--keystroke-file", keystroke_file,
            "--override-screen-height", "24",
            "--override-screen-width", "80"
        ], capture_output=True, text=True, timeout=30)

        elapsed = time.time() - start_time

        # Clean up keystroke file
        os.unlink(keystroke_file)

        # Check that operations completed successfully
        assert result.returncode == 0, f"Viewport operations failed: {result.stderr}"

        print(f"  ✓ Completed viewport operations in {elapsed:.2f} seconds")

        # Performance target: Should complete in under 10 seconds
        # (This is a reasonable target for 1M rows with zoom/pan operations)
        if elapsed > 10.0:
            print(f"  ⚠ Warning: Operations took {elapsed:.2f}s (target: <10s)")
        else:
            print(f"  ✓ Performance within target (<10s)")

        # Verify screen dump was generated
        assert "perf_test" in result.stdout, "Screen dump should show table name"

    finally:
        if os.path.exists(db_path):
            os.unlink(db_path)


@pytest.mark.timeout(120)  # 2 minutes timeout
def test_1m_row_csv_export():
    """Test CSV export performance with 1M rows."""
    # Create temporary database
    fd, db_path = tempfile.mkstemp(suffix='.db')
    os.close(fd)

    try:
        print("\nSetting up 1M row dataset for CSV export...")
        create_large_dataset(db_path, "perf_test", 1000000)

        # Get path to datapainter executable
        repo_root = Path(__file__).parent.parent.parent
        datapainter_exe = repo_root / "build" / "datapainter"

        # Export to CSV and measure time
        print("  Exporting to CSV...")
        start_time = time.time()

        result = subprocess.run([
            str(datapainter_exe),
            "--database", db_path,
            "--table", "perf_test",
            "--to-csv"
        ], capture_output=True, text=True, timeout=60)

        elapsed = time.time() - start_time

        # Check that export completed successfully
        assert result.returncode == 0, f"CSV export failed: {result.stderr}"

        # Count lines in output (should be 1M + 1 header)
        lines = result.stdout.strip().split('\n')
        assert len(lines) == 1000001, f"Expected 1000001 lines, got {len(lines)}"

        print(f"  ✓ Exported 1M rows in {elapsed:.2f} seconds")

        # Performance target: Should complete in under 30 seconds
        if elapsed > 30.0:
            print(f"  ⚠ Warning: Export took {elapsed:.2f}s (target: <30s)")
        else:
            print(f"  ✓ Performance within target (<30s)")

    finally:
        if os.path.exists(db_path):
            os.unlink(db_path)


if __name__ == "__main__":
    import pytest
    pytest.main([__file__, "-v", "-s"])
