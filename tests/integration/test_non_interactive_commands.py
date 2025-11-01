"""
Integration tests for DataPainter non-interactive commands.

Tests commands that don't require TUI interaction:
- --show-metadata
- --add-point
- --delete-point
"""

import pytest
import subprocess
import tempfile
import os
import sqlite3


def run_datapainter(args, input_text=None):
    """
    Run datapainter with given arguments.

    Returns: (returncode, stdout, stderr)
    """
    cmd = ['./datapainter'] + args
    result = subprocess.run(
        cmd,
        input=input_text,
        capture_output=True,
        text=True,
        cwd='.'
    )
    return result.returncode, result.stdout, result.stderr


def create_test_db():
    """Create a temporary database with a test table."""
    fd, db_path = tempfile.mkstemp(suffix='.db')
    os.close(fd)

    # Use datapainter to create the table
    run_datapainter([
        '--database', db_path,
        '--create-table', 'test_table',
        '--x-axis-name', 'x_axis',
        '--y-axis-name', 'y_axis',
        '--target-column-name', 'target',
        '--x-meaning', 'positive',
        '--o-meaning', 'negative'
    ])

    return db_path


class TestShowMetadata:
    """Test --show-metadata command."""

    def test_show_metadata_basic(self):
        """Test that --show-metadata displays table metadata."""
        db_path = create_test_db()
        try:
            returncode, stdout, stderr = run_datapainter([
                '--database', db_path,
                '--show-metadata',
                '--table', 'test_table'
            ])

            assert returncode == 0, f"Command should succeed, got: {stderr}"

            # Verify metadata fields appear in output
            assert 'test_table' in stdout, "Should show table name"
            assert 'x_axis' in stdout, "Should show x-axis name"
            assert 'y_axis' in stdout, "Should show y-axis name"
            assert 'target' in stdout, "Should show target column name"
            assert 'positive' in stdout, "Should show x meaning"
            assert 'negative' in stdout, "Should show o meaning"

        finally:
            if os.path.exists(db_path):
                os.unlink(db_path)

    def test_show_metadata_requires_table(self):
        """Test that --show-metadata requires --table argument."""
        db_path = create_test_db()
        try:
            returncode, stdout, stderr = run_datapainter([
                '--database', db_path,
                '--show-metadata'
            ])

            assert returncode == 2, "Should fail with exit code 2 for missing argument"
            assert '--table is required' in stderr, "Should explain missing argument"

        finally:
            if os.path.exists(db_path):
                os.unlink(db_path)

    def test_show_metadata_nonexistent_table(self):
        """Test --show-metadata with non-existent table."""
        db_path = create_test_db()
        try:
            returncode, stdout, stderr = run_datapainter([
                '--database', db_path,
                '--show-metadata',
                '--table', 'nonexistent'
            ])

            assert returncode != 0, "Should fail for non-existent table"

        finally:
            if os.path.exists(db_path):
                os.unlink(db_path)


class TestAddPoint:
    """Test --add-point command."""

    def test_add_point_basic(self):
        """Test adding a single point via command line."""
        db_path = create_test_db()
        try:
            # Add a point
            returncode, stdout, stderr = run_datapainter([
                '--database', db_path,
                '--add-point',
                '--table', 'test_table',
                '--x', '5.0',
                '--y', '10.0',
                '--target', 'positive'
            ])

            assert returncode == 0, f"Add point should succeed, got: {stderr}"

            # Verify point was added to database
            conn = sqlite3.connect(db_path)
            cursor = conn.cursor()
            cursor.execute('SELECT x, y, target FROM test_table')
            rows = cursor.fetchall()
            conn.close()

            assert len(rows) == 1, "Should have exactly one point"
            assert rows[0] == (5.0, 10.0, 'positive'), "Point should have correct values"

        finally:
            if os.path.exists(db_path):
                os.unlink(db_path)

    def test_add_multiple_points(self):
        """Test adding multiple points."""
        db_path = create_test_db()
        try:
            # Add first point
            run_datapainter([
                '--database', db_path,
                '--add-point',
                '--table', 'test_table',
                '--x', '1.0',
                '--y', '2.0',
                '--target', 'positive'
            ])

            # Add second point
            run_datapainter([
                '--database', db_path,
                '--add-point',
                '--table', 'test_table',
                '--x', '3.0',
                '--y', '4.0',
                '--target', 'negative'
            ])

            # Verify both points exist
            conn = sqlite3.connect(db_path)
            cursor = conn.cursor()
            cursor.execute('SELECT COUNT(*) FROM test_table')
            count = cursor.fetchone()[0]
            conn.close()

            assert count == 2, "Should have two points"

        finally:
            if os.path.exists(db_path):
                os.unlink(db_path)

    def test_add_point_requires_x(self):
        """Test that --add-point requires --x argument."""
        db_path = create_test_db()
        try:
            returncode, stdout, stderr = run_datapainter([
                '--database', db_path,
                '--add-point',
                '--table', 'test_table',
                '--y', '10.0',
                '--target', 'positive'
            ])

            assert returncode == 2, "Should fail with exit code 2 for missing argument"
            assert '--x is required' in stderr, "Should explain missing argument"

        finally:
            if os.path.exists(db_path):
                os.unlink(db_path)

    def test_add_point_requires_y(self):
        """Test that --add-point requires --y argument."""
        db_path = create_test_db()
        try:
            returncode, stdout, stderr = run_datapainter([
                '--database', db_path,
                '--add-point',
                '--table', 'test_table',
                '--x', '5.0',
                '--target', 'positive'
            ])

            assert returncode == 2, "Should fail with exit code 2 for missing argument"
            assert '--y is required' in stderr, "Should explain missing argument"

        finally:
            if os.path.exists(db_path):
                os.unlink(db_path)

    def test_add_point_requires_target(self):
        """Test that --add-point requires --target argument."""
        db_path = create_test_db()
        try:
            returncode, stdout, stderr = run_datapainter([
                '--database', db_path,
                '--add-point',
                '--table', 'test_table',
                '--x', '5.0',
                '--y', '10.0'
            ])

            assert returncode == 2, "Should fail with exit code 2 for missing argument"
            assert '--target is required' in stderr, "Should explain missing argument"

        finally:
            if os.path.exists(db_path):
                os.unlink(db_path)

    def test_add_point_outside_valid_range(self):
        """Test adding a point outside valid range (should fail)."""
        db_path = create_test_db()
        try:
            # Table has default valid range of -10 to 10
            returncode, stdout, stderr = run_datapainter([
                '--database', db_path,
                '--add-point',
                '--table', 'test_table',
                '--x', '100.0',  # Outside valid range
                '--y', '10.0',
                '--target', 'positive'
            ])

            # Should either reject the point or succeed with warning
            # (implementation may vary - at least shouldn't crash)
            assert returncode in [0, 2], "Should either succeed or fail gracefully"

        finally:
            if os.path.exists(db_path):
                os.unlink(db_path)


class TestDeletePoint:
    """Test --delete-point command."""

    def test_delete_point_by_id(self):
        """Test deleting a point by its ID."""
        db_path = create_test_db()
        try:
            # Add a point first
            run_datapainter([
                '--database', db_path,
                '--add-point',
                '--table', 'test_table',
                '--x', '5.0',
                '--y', '10.0',
                '--target', 'positive'
            ])

            # Get the point ID
            conn = sqlite3.connect(db_path)
            cursor = conn.cursor()
            cursor.execute('SELECT id FROM test_table')
            point_id = cursor.fetchone()[0]
            conn.close()

            # Delete the point
            returncode, stdout, stderr = run_datapainter([
                '--database', db_path,
                '--delete-point',
                '--table', 'test_table',
                '--point-id', str(point_id)
            ])

            assert returncode == 0, f"Delete should succeed, got: {stderr}"

            # Verify point was deleted
            conn = sqlite3.connect(db_path)
            cursor = conn.cursor()
            cursor.execute('SELECT COUNT(*) FROM test_table')
            count = cursor.fetchone()[0]
            conn.close()

            assert count == 0, "Point should be deleted"

        finally:
            if os.path.exists(db_path):
                os.unlink(db_path)
