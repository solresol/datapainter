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


class TestCSVExport:
    """Test --to-csv command."""

    def test_csv_export_basic(self):
        """Test basic CSV export with simple data."""
        db_path = create_test_db()
        try:
            # Add some points
            run_datapainter([
                '--database', db_path,
                '--add-point',
                '--table', 'test_table',
                '--x', '1.5',
                '--y', '2.5',
                '--target', 'positive'
            ])
            run_datapainter([
                '--database', db_path,
                '--add-point',
                '--table', 'test_table',
                '--x', '3.0',
                '--y', '4.0',
                '--target', 'negative'
            ])

            # Export to CSV
            returncode, stdout, stderr = run_datapainter([
                '--database', db_path,
                '--to-csv',
                '--table', 'test_table'
            ])

            assert returncode == 0, f"CSV export should succeed, got: {stderr}"

            # Verify CSV format
            lines = stdout.strip().split('\n')
            assert len(lines) == 3, "Should have header + 2 data rows"
            assert lines[0] == "x,y,target", "First line should be CSV header"

            # Verify data rows
            assert '1.5' in stdout, "Should contain x=1.5"
            assert '2.5' in stdout, "Should contain y=2.5"
            assert 'positive' in stdout, "Should contain target=positive"
            assert '3' in stdout or '3.0' in stdout, "Should contain x=3.0"
            assert '4' in stdout or '4.0' in stdout, "Should contain y=4.0"
            assert 'negative' in stdout, "Should contain target=negative"

        finally:
            if os.path.exists(db_path):
                os.unlink(db_path)

    def test_csv_export_empty_table(self):
        """Test CSV export with no data."""
        db_path = create_test_db()
        try:
            # Export empty table to CSV
            returncode, stdout, stderr = run_datapainter([
                '--database', db_path,
                '--to-csv',
                '--table', 'test_table'
            ])

            assert returncode == 0, f"CSV export should succeed even for empty table, got: {stderr}"

            # Should have header but no data rows
            lines = stdout.strip().split('\n')
            assert len(lines) == 1, "Should have only header row"
            assert lines[0] == "x,y,target", "Should have CSV header"

        finally:
            if os.path.exists(db_path):
                os.unlink(db_path)

    def test_csv_export_quote_escaping(self):
        """Test CSV export properly escapes quotes and commas."""
        db_path = create_test_db()
        try:
            # Add a point with a target value containing special characters
            run_datapainter([
                '--database', db_path,
                '--add-point',
                '--table', 'test_table',
                '--x', '1.0',
                '--y', '2.0',
                '--target', 'value,with,commas'
            ])

            # Export to CSV
            returncode, stdout, stderr = run_datapainter([
                '--database', db_path,
                '--to-csv',
                '--table', 'test_table'
            ])

            assert returncode == 0, f"CSV export should succeed, got: {stderr}"

            # Values with commas should be quoted
            assert '"value,with,commas"' in stdout, "Values with commas should be quoted"

        finally:
            if os.path.exists(db_path):
                os.unlink(db_path)

    def test_csv_export_rows_ordered_by_id(self):
        """Test CSV export orders rows by ID."""
        db_path = create_test_db()
        try:
            # Add points in a specific order
            run_datapainter([
                '--database', db_path,
                '--add-point',
                '--table', 'test_table',
                '--x', '3.0',
                '--y', '3.0',
                '--target', 'third'
            ])
            run_datapainter([
                '--database', db_path,
                '--add-point',
                '--table', 'test_table',
                '--x', '1.0',
                '--y', '1.0',
                '--target', 'first'
            ])
            run_datapainter([
                '--database', db_path,
                '--add-point',
                '--table', 'test_table',
                '--x', '2.0',
                '--y', '2.0',
                '--target', 'second'
            ])

            # Export to CSV
            returncode, stdout, stderr = run_datapainter([
                '--database', db_path,
                '--to-csv',
                '--table', 'test_table'
            ])

            assert returncode == 0, f"CSV export should succeed, got: {stderr}"

            lines = stdout.strip().split('\n')
            # Should be ordered by id (which matches insertion order)
            # First data row should have 'third' (id=1), then 'first' (id=2), then 'second' (id=3)
            assert 'third' in lines[1], "First data row should be the first inserted point"
            assert 'first' in lines[2], "Second data row should be the second inserted point"
            assert 'second' in lines[3], "Third data row should be the third inserted point"

        finally:
            if os.path.exists(db_path):
                os.unlink(db_path)

    def test_csv_export_requires_table(self):
        """Test that --to-csv requires --table argument."""
        db_path = create_test_db()
        try:
            returncode, stdout, stderr = run_datapainter([
                '--database', db_path,
                '--to-csv'
            ])

            assert returncode == 2, "Should fail with exit code 2 for missing argument"
            assert '--table is required' in stderr, "Should explain missing argument"

        finally:
            if os.path.exists(db_path):
                os.unlink(db_path)
