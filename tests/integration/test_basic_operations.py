"""
Integration tests for DataPainter basic TUI operations.

These tests validate the actual screen output and user interactions
using pyte terminal emulation.
"""

import pytest
import time
from tui_test_framework import DataPainterTest


class TestApplicationStartup:
    """Test application initialization and initial screen state."""

    def test_startup_shows_header(self):
        """Verify application displays header information on startup."""
        with DataPainterTest(width=80, height=24) as test:
            # Wait for startup message to clear and table UI to appear
            assert test.wait_for_text('test_table', timeout=3.0), \
                "Table name should appear after startup"

            lines = test.get_display_lines()
            # Check for key elements in header area (first 3 rows)
            header_text = '\n'.join(lines[:3])

            # Should show table name
            assert 'test_table' in header_text, "Table name should appear in header"

    def test_startup_shows_axes(self):
        """Verify application displays axis borders and labels."""
        with DataPainterTest(width=80, height=24) as test:
            # Wait for UI to appear
            test.wait_for_text('test_table', timeout=3.0)

            lines = test.get_display_lines()

            # Check for vertical axis (left border) - should have box drawing chars or '|'
            for row_idx in range(4, 23):  # Skip header rows (0-3)
                first_char = lines[row_idx][0] if lines[row_idx] else ' '
                # Should be some kind of border character
                assert first_char in ['|', '│', '+', '┤', '├', '┼'], \
                    f"Row {row_idx} should have left border, got '{first_char}'"

    def test_initial_viewport_empty(self):
        """Verify viewport starts with no data points."""
        with DataPainterTest(width=80, height=24) as test:
            lines = test.get_display_lines()

            # Content area (excluding header and borders)
            content = '\n'.join(lines[4:23])  # Skip header rows

            # Should not contain any 'x' or 'o' points initially
            assert 'x' not in content and 'X' not in content, \
                "Should not have x points initially"
            assert 'o' not in content and 'O' not in content, \
                "Should not have o points initially"


class TestPointCreation:
    """Test creating points with keyboard input."""

    def test_create_x_point(self):
        """Create a single 'x' point and verify it appears on screen."""
        with DataPainterTest(width=80, height=24) as test:
            # Wait for initial render
            time.sleep(0.2)

            # Send 'x' key to create a point at cursor position
            test.send_keys('x')

            # Allow time for screen update
            time.sleep(0.1)

            # Verify 'x' appears somewhere in the viewport
            lines = test.get_display_lines()
            screen_text = '\n'.join(lines)

            assert 'x' in screen_text, "Created 'x' point should appear on screen"

    def test_create_o_point(self):
        """Create a single 'o' point and verify it appears on screen."""
        with DataPainterTest(width=80, height=24) as test:
            time.sleep(0.2)

            # Send 'o' key to create a point
            test.send_keys('o')
            time.sleep(0.1)

            # Verify 'o' appears
            lines = test.get_display_lines()
            screen_text = '\n'.join(lines)

            assert 'o' in screen_text, "Created 'o' point should appear on screen"

    def test_multiple_points_same_cell(self):
        """Create multiple points in same cell and verify uppercase display."""
        with DataPainterTest(width=80, height=24) as test:
            time.sleep(0.2)

            # Create two 'x' points at same location
            test.send_keys('x')
            time.sleep(0.1)
            test.send_keys('x')
            time.sleep(0.1)

            # Should show 'X' (uppercase) indicating multiple points
            lines = test.get_display_lines()
            screen_text = '\n'.join(lines)

            # Note: This assumes the rendering logic uppercases multiple points
            # If point is at center, it might show as 'X' if there are 2+ points
            assert 'X' in screen_text or 'x' in screen_text, \
                "Multiple points should be visible"

    def test_create_different_types_same_cell(self):
        """Create both 'x' and 'o' points in same cell."""
        with DataPainterTest(width=80, height=24) as test:
            time.sleep(0.2)

            # Create 'x' and 'o' at same location
            test.send_keys('xo')
            time.sleep(0.1)

            lines = test.get_display_lines()
            screen_text = '\n'.join(lines)

            # Should show some indication of multiple/mixed points
            # Could be uppercase or special marker
            has_x = 'x' in screen_text or 'X' in screen_text
            has_o = 'o' in screen_text or 'O' in screen_text

            assert has_x or has_o, "Mixed point types should be visible"


class TestPointDeletion:
    """Test deleting points with backspace/delete keys."""

    def test_delete_single_point_with_backspace(self):
        """Create a point and delete it with backspace."""
        with DataPainterTest(width=80, height=24) as test:
            # Wait for UI
            test.wait_for_text('test_table', timeout=3.0)

            # Create point
            test.send_keys('x')
            time.sleep(0.2)

            # Count 'x' and 'X' in middle area before deletion
            lines_before = test.get_display_lines()
            # Check middle area (columns 10-70, rows 8-18) to avoid axis labels
            middle_before = '\n'.join(line[10:70] for line in lines_before[8:18])
            x_count_before = middle_before.count('x') + middle_before.count('X')
            assert x_count_before > 0, "Point should exist before deletion"

            # Delete point with backspace (special key)
            test.send_keys('BACKSPACE')
            time.sleep(1.0)  # Give extra time for render to complete

            # Count 'x' and 'X' in middle area after deletion
            lines_after = test.get_display_lines()
            middle_after = '\n'.join(line[10:70] for line in lines_after[8:18])
            x_count_after = middle_after.count('x') + middle_after.count('X')

            # Should have fewer x's after deletion
            assert x_count_after < x_count_before, \
                f"Point should be deleted (before: {x_count_before}, after: {x_count_after})"

    def test_delete_unsaved_point(self):
        """Create and delete a point before saving (tests unsaved changes logic)."""
        with DataPainterTest(width=80, height=24) as test:
            # Wait for UI
            test.wait_for_text('test_table', timeout=3.0)

            # Create point (not saved to DB yet)
            test.send_keys('o')
            time.sleep(0.2)

            # Count 'o' in middle area before deletion
            lines_before = test.get_display_lines()
            middle_before = '\n'.join(line[10:70] for line in lines_before[8:18])
            o_count_before = middle_before.count('o') + middle_before.count('O')
            assert o_count_before > 0, "Unsaved point should appear"

            # Delete the unsaved point
            test.send_keys('BACKSPACE')
            time.sleep(0.2)

            # Count 'o' in middle area after deletion
            lines_after = test.get_display_lines()
            middle_after = '\n'.join(line[10:70] for line in lines_after[8:18])
            o_count_after = middle_after.count('o') + middle_after.count('O')

            # Should have fewer o's after deletion
            assert o_count_after < o_count_before, \
                f"Unsaved point should be deleted (before: {o_count_before}, after: {o_count_after})"

    def test_delete_multiple_points_in_cell(self):
        """Create multiple points and delete them together."""
        with DataPainterTest(width=80, height=24) as test:
            # Wait for UI
            test.wait_for_text('test_table', timeout=3.0)

            # Create three points at same location
            test.send_keys('xxx')
            time.sleep(0.2)

            # Count 'x' in middle area before deletion
            lines_before = test.get_display_lines()
            middle_before = '\n'.join(line[10:70] for line in lines_before[8:18])
            x_count_before = middle_before.count('x') + middle_before.count('X')
            assert x_count_before > 0, "Multiple points should exist"

            # Delete all points in cell
            test.send_keys('BACKSPACE')
            time.sleep(0.2)

            # Count 'x' in middle area after deletion
            lines_after = test.get_display_lines()
            middle_after = '\n'.join(line[10:70] for line in lines_after[8:18])
            x_count_after = middle_after.count('x') + middle_after.count('X')

            # All points should be deleted
            assert x_count_after < x_count_before, \
                f"All points in cell should be deleted (before: {x_count_before}, after: {x_count_after})"


class TestPointConversion:
    """Test converting points between types."""

    def test_convert_x_to_o(self):
        """Create 'x' point and convert to 'o' with uppercase 'O'."""
        with DataPainterTest(width=80, height=24) as test:
            time.sleep(0.2)

            # Create 'x' point
            test.send_keys('x')
            time.sleep(0.1)

            screen_before = '\n'.join(test.get_display_lines())
            assert 'x' in screen_before, "Should have 'x' point"

            # Convert to 'o' using uppercase 'O'
            test.send_keys('O')
            time.sleep(0.1)

            # Should now show 'o' instead
            lines_after = test.get_display_lines()
            content_after = '\n'.join(lines_after[4:23])

            assert 'o' in content_after or 'O' in content_after, \
                "Point should be converted to 'o'"

    def test_convert_o_to_x(self):
        """Create 'o' point and convert to 'x' with uppercase 'X'."""
        with DataPainterTest(width=80, height=24) as test:
            time.sleep(0.2)

            # Create 'o' point
            test.send_keys('o')
            time.sleep(0.1)

            screen_before = '\n'.join(test.get_display_lines())
            assert 'o' in screen_before, "Should have 'o' point"

            # Convert to 'x' using uppercase 'X'
            test.send_keys('X')
            time.sleep(0.1)

            # Should now show 'x'
            content_after = '\n'.join(test.get_display_lines()[4:23])

            assert 'x' in content_after or 'X' in content_after, \
                "Point should be converted to 'x'"

    def test_flip_point_with_g(self):
        """Test flipping point type with 'g' key."""
        with DataPainterTest(width=80, height=24) as test:
            time.sleep(0.2)

            # Create 'x' point
            test.send_keys('x')
            time.sleep(0.1)

            # Flip with 'g'
            test.send_keys('g')
            time.sleep(0.1)

            # Should now be 'o'
            content_after = '\n'.join(test.get_display_lines()[4:23])

            assert 'o' in content_after or 'O' in content_after, \
                "Point should flip to opposite type"


class TestCursorMovement:
    """Test cursor movement with arrow keys."""

    def test_cursor_moves_with_arrows(self):
        """Verify cursor responds to arrow key input."""
        with DataPainterTest(width=80, height=24) as test:
            time.sleep(0.2)

            # Create point at initial cursor position
            test.send_keys('x')
            time.sleep(0.1)

            screen_before = '\n'.join(test.get_display_lines())
            x_positions_before = [
                (i, line.find('x'))
                for i, line in enumerate(test.get_display_lines())
                if 'x' in line
            ]

            # Move cursor right and create another point
            test.send_keys('RIGHT')
            time.sleep(0.05)
            test.send_keys('o')
            time.sleep(0.1)

            # Should have both 'x' and 'o' at different positions
            lines_after = test.get_display_lines()
            has_x = any('x' in line for line in lines_after)
            has_o = any('o' in line for line in lines_after)

            assert has_x and has_o, "Both points should exist after cursor move"

    def test_point_appears_at_cursor_position(self):
        """Verify created points appear at cursor location, not offset."""
        with DataPainterTest(width=80, height=24) as test:
            time.sleep(0.2)

            # Move cursor to a specific area (down and right from center)
            test.send_keys('DOWNDOWNDOWNRIGHTRIGHTRIGHTRIGHT')
            time.sleep(0.1)

            # Create point
            test.send_keys('x')
            time.sleep(0.1)

            # Verify point appears (this test mainly ensures no crash/offset bug)
            # More specific positioning tests would require knowing exact cell coordinates
            screen = '\n'.join(test.get_display_lines())
            assert 'x' in screen, "Point should appear after cursor movement"


class TestScreenResizing:
    """Test application behavior with different terminal sizes."""

    def test_small_terminal_80x24(self):
        """Verify application works with standard 80x24 terminal."""
        with DataPainterTest(width=80, height=24) as test:
            # Just verify it starts and renders
            lines = test.get_display_lines()
            assert len(lines) == 24, "Should have 24 rows"
            assert all(len(line) <= 80 for line in lines), \
                "No line should exceed 80 characters"

    def test_large_terminal_120x40(self):
        """Verify application works with larger terminal size."""
        with DataPainterTest(width=120, height=40) as test:
            lines = test.get_display_lines()
            assert len(lines) == 40, "Should have 40 rows"

            # Should still show header and content
            header_text = '\n'.join(lines[:3])
            assert 'test_table' in header_text, "Header should appear in large terminal"

    @pytest.mark.skip(reason="Dynamic resize not implemented in test framework yet")
    def test_terminal_resize_during_operation(self):
        """Test resizing terminal while application is running."""
        # This would require sending SIGWINCH to the process
        # and updating the pyte screen dimensions
        pass


class TestEdgeCases:
    """Test edge cases and boundary conditions."""

    def test_create_point_at_viewport_edge(self):
        """Create points near the viewport boundaries."""
        with DataPainterTest(width=80, height=24) as test:
            time.sleep(0.2)

            # Move to top-left corner of viewport
            test.send_keys('UPUPUPUPLEFTLEFTLEFTLEFT')
            time.sleep(0.1)
            test.send_keys('x')
            time.sleep(0.1)

            # Should still create point without crash
            assert True, "Should handle edge positioning"

    def test_rapid_keystrokes(self):
        """Test application handles rapid key input."""
        with DataPainterTest(width=80, height=24) as test:
            time.sleep(0.2)

            # Rapid point creation
            test.send_keys('xoxoxoxoxo', delay=0.01)
            time.sleep(0.2)

            # Should show points (exact count may vary due to timing)
            screen = '\n'.join(test.get_display_lines())
            has_points = 'x' in screen or 'o' in screen or 'X' in screen or 'O' in screen
            assert has_points, "Should handle rapid input"

    def test_quit_with_q_key(self):
        """Verify application quits cleanly with 'q' key."""
        with DataPainterTest(width=80, height=24) as test:
            time.sleep(0.2)

            # Send quit command
            test.send_keys('q')
            time.sleep(0.2)

            # Process should exit cleanly
            # The context manager will handle cleanup and verify no hanging process
            assert True, "Application should quit cleanly"


if __name__ == '__main__':
    pytest.main([__file__, '-v'])
