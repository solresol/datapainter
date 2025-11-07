"""
Integration tests for table view mode in DataPainter.

Tests switching between viewport and table view modes,
table navigation, filtering, and editing operations.
"""

import pytest
import tempfile
import os
import time
from pathlib import Path
from tui_test_framework import DataPainterTest


class TestTableViewSwitching:
    """Test switching between viewport and table view modes."""

    def test_switch_to_table_view_with_hash_key(self):
        """Test that pressing # switches from viewport to table view."""
        with tempfile.TemporaryDirectory() as tmpdir:
            db_path = os.path.join(tmpdir, "test.db")

            with DataPainterTest(
                width=80, height=24,
                database_path=db_path,
                table_name="points"
            ) as test:
                # Wait for initial render
                time.sleep(0.1)

                # Add a couple of points so we have data to view
                test.send_keys("x")  # Create point at cursor
                time.sleep(0.1)

                test.send_keys("o")  # Create another point
                time.sleep(0.1)

                # Switch to table view with #
                test.send_keys("#")
                time.sleep(0.1)

                # Verify we're in table view mode
                screen = test.get_screen()
                screen_text = '\n'.join(screen.display)

                # Should show table headers
                assert "x" in screen_text.lower() and "y" in screen_text.lower()
                # Should show filter line (indicates we're in table view)
                assert "filter" in screen_text.lower()

    def test_switch_back_to_viewport_from_table(self):
        """Test that pressing # again switches back to viewport view."""
        with tempfile.TemporaryDirectory() as tmpdir:
            db_path = os.path.join(tmpdir, "test.db")

            with DataPainterTest(
                width=80, height=24,
                database_path=db_path,
                table_name="points"
            ) as test:
                # Wait for initial render
                time.sleep(0.1)

                # Add some points
                test.send_keys("x")
                time.sleep(0.1)

                # Switch to table view
                test.send_keys("#")
                time.sleep(0.1)

                # Switch back to viewport
                test.send_keys("#")
                time.sleep(0.1)

                # Verify we're back in viewport mode
                screen = test.get_screen()
                screen_text = '\n'.join(screen.display)

                # Should show viewport elements (axes, cursor indicator)
                # The exact content depends on viewport implementation
                # but should NOT show "Table View" anymore
                assert "table view" not in screen_text.lower()


class TestTableViewDisplay:
    """Test table view display and formatting."""

    def test_display_filter_at_top(self):
        """Test that the filter is displayed at the top of table view."""
        with tempfile.TemporaryDirectory() as tmpdir:
            db_path = os.path.join(tmpdir, "test.db")

            with DataPainterTest(
                width=80, height=24,
                database_path=db_path,
                table_name="points"
            ) as test:
                time.sleep(0.1)

                # Add some points
                test.send_keys("x")
                time.sleep(0.1)

                # Switch to table view
                test.send_keys("#")
                time.sleep(0.1)

                screen = test.get_screen()
                screen_text = '\n'.join(screen.display)

                # Should show filter information
                # Default filter is viewport bounds
                assert "filter" in screen_text.lower() or "where" in screen_text.lower()

    def test_table_shows_column_headers(self):
        """Test that table view shows x, y, target column headers."""
        with tempfile.TemporaryDirectory() as tmpdir:
            db_path = os.path.join(tmpdir, "test.db")

            with DataPainterTest(
                width=80, height=24,
                database_path=db_path,
                table_name="points"
            ) as test:
                time.sleep(0.1)

                # Add a point
                test.send_keys("x")
                time.sleep(0.1)

                # Switch to table view
                test.send_keys("#")
                time.sleep(0.1)

                screen = test.get_screen()
                # Get the first few lines where headers should be
                header_area = '\n'.join(screen.display[:5])

                # Should contain column headers
                assert "x" in header_area.lower()
                assert "y" in header_area.lower()
                assert "target" in header_area.lower()


class TestTableViewNavigation:
    """Test navigation within table view."""

    def test_navigate_rows_with_arrow_keys(self):
        """Test that arrow keys navigate between rows in table view."""
        with tempfile.TemporaryDirectory() as tmpdir:
            db_path = os.path.join(tmpdir, "test.db")

            with DataPainterTest(
                width=80, height=24,
                database_path=db_path,
                table_name="points"
            ) as test:
                time.sleep(0.1)

                # Add multiple points at different positions
                test.send_keys("x")  # First point at default position
                time.sleep(0.1)
                test.send_keys("KEY_DOWN")  # Move cursor down
                time.sleep(0.1)
                test.send_keys("KEY_RIGHT")  # Move cursor right
                time.sleep(0.1)
                test.send_keys("o")  # Second point at different position
                time.sleep(0.1)
                test.send_keys("KEY_DOWN")  # Move cursor down again
                time.sleep(0.1)
                test.send_keys("KEY_LEFT")  # Move cursor left
                time.sleep(0.1)
                test.send_keys("x")  # Third point at yet another position
                time.sleep(0.1)

                # Switch to table view
                test.send_keys("#")
                time.sleep(0.1)

                # Navigate with arrow keys - should not crash
                initial_screen = test.get_screen()
                test.send_keys("KEY_DOWN")
                time.sleep(0.1)

                after_down_screen = test.get_screen()
                # Should still have the same screen structure
                assert len(after_down_screen.display) == len(initial_screen.display)
                # Should still show filter (still in table view)
                assert "filter" in '\n'.join(after_down_screen.display).lower()

                # Navigate up - should also not crash
                test.send_keys("KEY_UP")
                time.sleep(0.1)

                after_up_screen = test.get_screen()
                # Should still be in table view with same structure
                assert len(after_up_screen.display) == len(initial_screen.display)
                assert "filter" in '\n'.join(after_up_screen.display).lower()


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
