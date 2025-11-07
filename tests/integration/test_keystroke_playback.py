#!/usr/bin/env python3
"""Integration tests for keystroke playback functionality.

Tests the --keystroke-file CLI argument that allows automated testing
by replaying keystrokes from a file instead of reading from terminal.
"""

import os
import subprocess
import tempfile
import sqlite3
from pathlib import Path


def setup_test_database():
    """Create a temporary database with a test table."""
    # Create temporary database file
    fd, db_path = tempfile.mkstemp(suffix='.db')
    os.close(fd)

    # Get path to datapainter executable
    repo_root = Path(__file__).parent.parent.parent
    datapainter_exe = repo_root / "build" / "datapainter"

    # Create test table using datapainter
    result = subprocess.run([
        str(datapainter_exe),
        "--database", db_path,
        "--create-table",
        "--table", "test",
        "--target-column-name", "label",
        "--x-axis-name", "x",
        "--y-axis-name", "y",
        "--x-meaning", "positive",
        "--o-meaning", "negative",
        "--min-x", "-10",
        "--max-x", "10",
        "--min-y", "-10",
        "--max-y", "10"
    ], capture_output=True, text=True)

    if result.returncode != 0:
        os.unlink(db_path)
        raise RuntimeError(f"Failed to create test table: {result.stderr}")

    return db_path


def cleanup_database(db_path):
    """Remove temporary database."""
    if os.path.exists(db_path):
        os.unlink(db_path)


def test_keystroke_playback_creates_points():
    """Test that keystroke playback can create points."""
    db_path = setup_test_database()

    try:
        # Create keystroke file
        fd, keystroke_file = tempfile.mkstemp(suffix='.txt')
        with os.fdopen(fd, 'w') as f:
            f.write("# Move cursor and create points\n")
            f.write("x\n")  # Create an x point at center
            f.write("<right>\n")  # Move right
            f.write("o\n")  # Create an o point
            f.write("s\n")  # Save changes
            f.write("q\n")  # Quit

        # Get path to datapainter executable
        repo_root = Path(__file__).parent.parent.parent
        datapainter_exe = repo_root / "build" / "datapainter"

        # Run datapainter with keystroke file
        result = subprocess.run([
            str(datapainter_exe),
            "--database", db_path,
            "--table", "test",
            "--keystroke-file", keystroke_file,
            "--override-screen-height", "20",
            "--override-screen-width", "60"
        ], capture_output=True, text=True, timeout=5)

        # Clean up keystroke file
        os.unlink(keystroke_file)

        # Check that program exited successfully
        assert result.returncode == 0, f"datapainter failed: {result.stderr}"

        # Verify points were created in database
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        cursor.execute("SELECT target FROM test ORDER BY id")
        points = cursor.fetchall()
        conn.close()

        # Should have created 2 points (x and o)
        assert len(points) == 2, f"Expected 2 points, got {len(points)}"
        assert points[0][0] == "positive", "First point should be 'positive'"
        assert points[1][0] == "negative", "Second point should be 'negative'"

    finally:
        cleanup_database(db_path)


def test_keystroke_playback_invalid_file():
    """Test that invalid keystroke file produces error."""
    db_path = setup_test_database()

    try:
        # Get path to datapainter executable
        repo_root = Path(__file__).parent.parent.parent
        datapainter_exe = repo_root / "build" / "datapainter"

        # Run datapainter with non-existent keystroke file
        result = subprocess.run([
            str(datapainter_exe),
            "--database", db_path,
            "--table", "test",
            "--keystroke-file", "/nonexistent/file.txt"
        ], capture_output=True, text=True, timeout=5)

        # Should fail with error exit code
        assert result.returncode == 2, f"Expected exit code 2, got {result.returncode}"

        # Should report error about file
        assert "Could not open file" in result.stderr, "Should report file error"

    finally:
        cleanup_database(db_path)


def test_keystroke_playback_empty_file():
    """Test that empty keystroke file produces error."""
    db_path = setup_test_database()

    try:
        # Create empty keystroke file
        fd, keystroke_file = tempfile.mkstemp(suffix='.txt')
        os.close(fd)  # Close immediately, leaving file empty

        # Get path to datapainter executable
        repo_root = Path(__file__).parent.parent.parent
        datapainter_exe = repo_root / "build" / "datapainter"

        # Run datapainter with empty keystroke file
        result = subprocess.run([
            str(datapainter_exe),
            "--database", db_path,
            "--table", "test",
            "--keystroke-file", keystroke_file
        ], capture_output=True, text=True, timeout=5)

        # Clean up keystroke file
        os.unlink(keystroke_file)

        # Should fail with error exit code
        assert result.returncode == 2, f"Expected exit code 2, got {result.returncode}"

        # Should report error about no keystrokes
        assert "no valid keystrokes" in result.stderr, "Should report no keystrokes error"

    finally:
        cleanup_database(db_path)


def test_keystroke_playback_with_special_keys():
    """Test that special keys (arrow keys, space, etc.) work in keystroke playback."""
    db_path = setup_test_database()

    try:
        # Create keystroke file with special keys
        fd, keystroke_file = tempfile.mkstemp(suffix='.txt')
        with os.fdopen(fd, 'w') as f:
            f.write("# Test special keys\n")
            f.write("x\n")  # Create point
            f.write("<up>\n")  # Move up
            f.write("<up>\n")
            f.write("o\n")  # Create another point
            f.write("<down>\n")  # Move down
            f.write("<left>\n")  # Move left
            f.write("x\n")  # Create another point
            f.write("s\n")  # Save changes
            f.write("q\n")  # Quit

        # Get path to datapainter executable
        repo_root = Path(__file__).parent.parent.parent
        datapainter_exe = repo_root / "build" / "datapainter"

        # Run datapainter with keystroke file
        result = subprocess.run([
            str(datapainter_exe),
            "--database", db_path,
            "--table", "test",
            "--keystroke-file", keystroke_file,
            "--override-screen-height", "20",
            "--override-screen-width", "60"
        ], capture_output=True, text=True, timeout=5)

        # Clean up keystroke file
        os.unlink(keystroke_file)

        # Check that program exited successfully
        assert result.returncode == 0, f"datapainter failed: {result.stderr}"

        # Verify points were created
        conn = sqlite3.connect(db_path)
        cursor = conn.cursor()
        cursor.execute("SELECT COUNT(*) FROM test")
        count = cursor.fetchone()[0]
        conn.close()

        # Should have created 3 points
        assert count == 3, f"Expected 3 points, got {count}"

    finally:
        cleanup_database(db_path)


if __name__ == "__main__":
    import pytest
    pytest.main([__file__, "-v"])
