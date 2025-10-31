"""
Debug test to understand deletion behavior.
"""

import time
from tui_test_framework import DataPainterTest


def test_debug_deletion_verbose():
    """Debug test to see exactly what's happening with deletion."""
    with DataPainterTest(width=80, height=24) as test:
        # Wait for UI
        test.wait_for_text('test_table', timeout=3.0)

        print("\n=== Initial state ===")
        lines = test.get_display_lines()
        for i, line in enumerate(lines):
            print(f"{i:2d} |{line}|")

        # Create a point
        print("\n=== Creating point with 'x' ===")
        test.send_keys('x')
        time.sleep(0.3)

        lines = test.get_display_lines()
        for i, line in enumerate(lines):
            print(f"{i:2d} |{line}|")

        # Try to delete
        print("\n=== Pressing BACKSPACE ===")
        test.send_keys('BACKSPACE')
        time.sleep(0.3)

        lines = test.get_display_lines()
        for i, line in enumerate(lines):
            print(f"{i:2d} |{line}|")


if __name__ == '__main__':
    test_debug_deletion_verbose()
