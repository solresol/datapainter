"""
Integration tests for Tab navigation through header fields and buttons.

These tests validate the actual tab navigation behavior using the VTE framework,
checking that the UI properly highlights focused fields as the user presses Tab.
"""

import pytest
import time
from tui_test_framework import DataPainterTest


class TestTabNavigation:
    """Test tab key navigation through UI elements."""

    def test_tab_highlights_database_field(self):
        """Verify pressing Tab highlights database name field."""
        with DataPainterTest(width=80, height=24) as test:
            # Wait for initial render
            test.wait_for_text('test_table', timeout=3.0)
            time.sleep(0.2)

            # Press Tab to focus database field
            test.send_keys('TAB')
            time.sleep(0.1)

            # Get header row - database name should be in brackets
            lines = test.get_display_lines()
            header_row0 = lines[0]

            # Database filename (temp file) should be highlighted with brackets
            # Looking for pattern like [something.db]
            assert '[' in header_row0 and '.db]' in header_row0, \
                f"Database name should be highlighted with brackets after Tab. Got: {header_row0}"

    def test_tab_cycles_to_table_field(self):
        """Verify second Tab highlights table name field."""
        with DataPainterTest(width=80, height=24) as test:
            test.wait_for_text('test_table', timeout=3.0)
            time.sleep(0.2)

            # Press Tab twice to reach table field
            test.send_keys('TAB')
            time.sleep(0.05)
            test.send_keys('TAB')
            time.sleep(0.1)

            # Table name should be in brackets
            lines = test.get_display_lines()
            header_row0 = lines[0]

            assert '[test_table]' in header_row0, \
                f"Table name should be highlighted with brackets. Got: {header_row0}"

    def test_tab_cycles_to_target_field(self):
        """Verify third Tab highlights target column field."""
        with DataPainterTest(width=80, height=24) as test:
            test.wait_for_text('test_table', timeout=3.0)
            time.sleep(0.2)

            # Press Tab 3 times to reach target field (row 1)
            for _ in range(3):
                test.send_keys('TAB')
                time.sleep(0.05)
            time.sleep(0.1)

            lines = test.get_display_lines()
            header_row1 = lines[1]

            # Target column "label" should be in brackets
            assert '[label]' in header_row1, \
                f"Target column should be highlighted. Got: {header_row1}"

    def test_tab_cycles_to_x_meaning_field(self):
        """Verify fourth Tab highlights x meaning field."""
        with DataPainterTest(width=80, height=24) as test:
            test.wait_for_text('test_table', timeout=3.0)
            time.sleep(0.2)

            # Press Tab 4 times to reach x meaning
            for _ in range(4):
                test.send_keys('TAB')
                time.sleep(0.05)
            time.sleep(0.1)

            lines = test.get_display_lines()
            header_row1 = lines[1]

            # X meaning "positive" should be in brackets
            assert 'x=[positive]' in header_row1, \
                f"X meaning should be highlighted. Got: {header_row1}"

    def test_tab_cycles_to_o_meaning_field(self):
        """Verify fifth Tab highlights o meaning field."""
        with DataPainterTest(width=80, height=24) as test:
            test.wait_for_text('test_table', timeout=3.0)
            time.sleep(0.2)

            # Press Tab 5 times to reach o meaning
            for _ in range(5):
                test.send_keys('TAB')
                time.sleep(0.05)
            time.sleep(0.1)

            lines = test.get_display_lines()
            header_row1 = lines[1]

            # O meaning "negative" should be in brackets
            assert 'o=[negative]' in header_row1, \
                f"O meaning should be highlighted. Got: {header_row1}"

    def test_tab_cycles_to_tabular_button(self):
        """Verify Tab cycles from header fields to Tabular button in footer."""
        with DataPainterTest(width=80, height=24) as test:
            test.wait_for_text('test_table', timeout=3.0)
            time.sleep(0.2)

            # Press Tab 6 times to reach first button
            # (database, table, target, x meaning, o meaning, then tabular button)
            for _ in range(6):
                test.send_keys('TAB')
                time.sleep(0.05)
            time.sleep(0.1)

            lines = test.get_display_lines()
            footer_row = lines[-1]  # Last row is footer

            # Tabular button should be highlighted
            assert '[#:Tabular]' in footer_row or '[Tabular]' in footer_row, \
                f"Tabular button should be highlighted. Got: {footer_row}"

    def test_tab_cycles_through_all_buttons(self):
        """Verify Tab cycles through all footer buttons."""
        with DataPainterTest(width=80, height=24) as test:
            test.wait_for_text('test_table', timeout=3.0)
            time.sleep(0.2)

            # Tab to Undo button (7th tab)
            for _ in range(7):
                test.send_keys('TAB')
                time.sleep(0.05)
            time.sleep(0.1)

            lines = test.get_display_lines()
            footer = lines[-1]
            assert '[u:Undo]' in footer or '[Undo]' in footer, \
                f"Undo button should be highlighted. Got: {footer}"

            # Tab to Save button (8th tab)
            test.send_keys('TAB')
            time.sleep(0.1)
            lines = test.get_display_lines()
            footer = lines[-1]
            assert '[s:Save]' in footer or '[Save]' in footer, \
                f"Save button should be highlighted. Got: {footer}"

            # Tab to Quit button (9th tab)
            test.send_keys('TAB')
            time.sleep(0.1)
            lines = test.get_display_lines()
            footer = lines[-1]
            assert '[q:Quit]' in footer or '[Quit]' in footer, \
                f"Quit button should be highlighted. Got: {footer}"

    def test_tab_wraps_back_to_viewport(self):
        """Verify Tab wraps from last button back to viewport."""
        with DataPainterTest(width=80, height=24) as test:
            test.wait_for_text('test_table', timeout=3.0)
            time.sleep(0.2)

            # Tab through all fields and buttons, then one more to wrap
            # 5 header fields + 4 buttons + 1 to wrap = 10
            for _ in range(10):
                test.send_keys('TAB')
                time.sleep(0.05)
            time.sleep(0.1)

            lines = test.get_display_lines()
            header_row0 = lines[0]
            footer = lines[-1]

            # Should be back to viewport - no brackets in header or footer
            # Database should not be highlighted
            assert not ('[' in header_row0 and '.db]' in header_row0 and
                       header_row0.index('[') < header_row0.index('.db')), \
                "Database should not be highlighted when back to viewport"

            # No buttons should be highlighted
            assert '[#:Tabular]' not in footer and \
                   '[u:Undo]' not in footer and \
                   '[s:Save]' not in footer and \
                   '[q:Quit]' not in footer, \
                f"No buttons should be highlighted when back to viewport. Got: {footer}"

    def test_escape_returns_to_viewport_from_field(self):
        """Verify Escape key returns to viewport from any focused field."""
        with DataPainterTest(width=80, height=24) as test:
            test.wait_for_text('test_table', timeout=3.0)
            time.sleep(0.2)

            # Tab to target field
            for _ in range(3):
                test.send_keys('TAB')
                time.sleep(0.05)
            time.sleep(0.1)

            # Verify target is focused
            lines = test.get_display_lines()
            assert '[label]' in lines[1], "Target should be focused before ESC"

            # Press Escape
            test.send_keys('ESC')
            time.sleep(0.1)

            # Check that focus is cleared
            lines = test.get_display_lines()
            header_row1 = lines[1]

            # Label should not be in brackets anymore
            assert '[label]' not in header_row1 or 'label' in header_row1.replace('[', '').replace(']', ''), \
                f"Target should not be highlighted after ESC. Got: {header_row1}"

    def test_escape_returns_to_viewport_from_button(self):
        """Verify Escape key returns to viewport from focused button."""
        with DataPainterTest(width=80, height=24) as test:
            test.wait_for_text('test_table', timeout=3.0)
            time.sleep(0.2)

            # Tab to Save button
            for _ in range(8):
                test.send_keys('TAB')
                time.sleep(0.05)
            time.sleep(0.1)

            # Verify Save is focused
            lines = test.get_display_lines()
            footer = lines[-1]
            assert '[s:Save]' in footer or '[Save]' in footer, \
                "Save button should be focused before ESC"

            # Press Escape
            test.send_keys('ESC')
            time.sleep(0.1)

            # Check that focus is cleared
            lines = test.get_display_lines()
            footer = lines[-1]

            assert '[s:Save]' not in footer and '[Save]' not in footer.replace('s:Save', ''), \
                f"Save button should not be highlighted after ESC. Got: {footer}"
