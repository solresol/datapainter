"""
Test framework for DataPainter TUI using pyte terminal emulation.

This module provides utilities for testing terminal user interfaces by:
1. Spawning datapainter in a pseudo-terminal (PTY)
2. Using pyte to emulate terminal and capture screen state
3. Sending keystrokes and validating output
4. Managing test lifecycle (setup, cleanup, timeouts)
"""

import os
import pty
import select
import signal
import tempfile
import time
from pathlib import Path
from typing import List, Optional, Tuple

import pyte


def resolve_datapainter_path(provided_path: Optional[str] = None) -> str:
    """Return an absolute path to the datapainter executable."""

    env_path = os.environ.get("DATAPAINTER_PATH")
    if env_path and not provided_path:
        candidate = Path(env_path).expanduser()
        if candidate.exists():
            return str(candidate.resolve())

    repo_root = Path(__file__).resolve().parents[2]

    def _resolve_candidate(path: Path) -> Optional[str]:
        if path.exists():
            return str(path.resolve())
        return None

    if provided_path:
        candidate = Path(provided_path)
        if not candidate.is_absolute():
            candidate = Path(__file__).resolve().parent / candidate
        resolved = _resolve_candidate(candidate)
        if resolved:
            return resolved
        raise FileNotFoundError(
            f"datapainter executable not found at provided path: {candidate}"
        )

    candidates = [
        repo_root / "build" / "datapainter",
        repo_root / "datapainter",
        repo_root / "bin" / "datapainter",
    ]

    for candidate in candidates:
        resolved = _resolve_candidate(candidate)
        if resolved:
            return resolved

    raise FileNotFoundError(
        "Could not locate the datapainter executable. Build the project or set "
        "DATAPAINTER_PATH to the executable."
    )


class DataPainterTest:
    """
    Test harness for DataPainter TUI application.

    Usage:
        with DataPainterTest(width=80, height=24) as test:
            test.send_keys("x")  # Send 'x' key
            screen = test.get_screen()  # Get current screen state
            assert "x" in screen.display[10]
    """

    def __init__(self, width: int = 80, height: int = 24,
                 datapainter_path: Optional[str] = None,
                 database_path: Optional[str] = None,
                 table_name: Optional[str] = None):
        """
        Initialize test harness.

        Args:
            width: Terminal width in characters
            height: Terminal height in rows
            datapainter_path: Path to datapainter executable
            database_path: Path to test database (creates temp if None)
            table_name: Name of table to open (creates test_table if None)
        """
        self.width = width
        self.height = height
        self.datapainter_path = resolve_datapainter_path(datapainter_path)
        self.database_path = database_path
        self.table_name = table_name or "test_table"

        # Process management
        self.pid: Optional[int] = None
        self.fd: Optional[int] = None

        # Terminal emulation
        self.screen = pyte.Screen(width, height)
        self.stream = pyte.ByteStream(self.screen)

        # Temporary file management
        self.temp_db: Optional[str] = None
        self._cleanup_handlers = []

    def __enter__(self):
        """Start datapainter in a PTY."""
        self.start()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Clean up resources."""
        self.stop()
        return False

    def start(self):
        """
        Start datapainter process in a pseudo-terminal.
        Creates a temporary database if none specified.
        """
        # Track whether we created the database
        created_temp_db = False

        # Create temporary database if needed
        if self.database_path is None:
            fd, self.temp_db = tempfile.mkstemp(suffix=".db")
            os.close(fd)
            self.database_path = self.temp_db
            self._cleanup_handlers.append(lambda: os.unlink(self.temp_db))
            created_temp_db = True

        # Initialize database with test table if it doesn't exist or is empty
        if created_temp_db or not os.path.exists(self.database_path) or os.path.getsize(self.database_path) == 0:
            self._init_test_database()

        # Set up environment for clean terminal
        env = os.environ.copy()
        env['TERM'] = 'xterm-256color'

        # Fork and exec datapainter
        self.pid, self.fd = pty.fork()

        if self.pid == 0:
            # Child process - exec datapainter
            os.execve(
                self.datapainter_path,
                [
                    'datapainter',
                    '--database', self.database_path,
                    '--table', self.table_name,
                    '--override-screen-height', str(self.height),
                    '--override-screen-width', str(self.width)
                ],
                env
            )

        # Parent process - set up terminal
        # Set terminal window size
        import fcntl
        import struct
        import termios

        # Pack window size: rows, cols, xpixel, ypixel
        winsize = struct.pack('HHHH', self.height, self.width, 0, 0)
        fcntl.ioctl(self.fd, termios.TIOCSWINSZ, winsize)

        # Set non-blocking mode for reading
        flags = fcntl.fcntl(self.fd, fcntl.F_GETFL)
        fcntl.fcntl(self.fd, fcntl.F_SETFL, flags | os.O_NONBLOCK)

        # Wait for initial screen to render
        # Give extra time for startup message to clear
        time.sleep(1.0)
        self._read_output()

    def _init_test_database(self):
        """Create a test database with a test table."""
        import subprocess
        subprocess.run([
            self.datapainter_path,
            '--database', self.database_path,
            '--create-table',
            '--table', self.table_name,
            '--target-column-name', 'label',
            '--x-axis-name', 'x',
            '--y-axis-name', 'y',
            '--x-meaning', 'positive',
            '--o-meaning', 'negative',
            '--min-x', '-10',
            '--max-x', '10',
            '--min-y', '-10',
            '--max-y', '10'
        ], check=True, capture_output=True)

    def stop(self):
        """Stop datapainter process and clean up resources."""
        if self.pid:
            try:
                # Try SIGTERM first
                os.kill(self.pid, signal.SIGTERM)

                # Wait up to 1 second for graceful shutdown
                for _ in range(10):
                    try:
                        pid, status = os.waitpid(self.pid, os.WNOHANG)
                        if pid != 0:  # Process exited
                            break
                    except (OSError, ChildProcessError):
                        break
                    time.sleep(0.1)
                else:
                    # Process still alive, force kill
                    try:
                        os.kill(self.pid, signal.SIGKILL)
                        os.waitpid(self.pid, 0)
                    except (OSError, ChildProcessError):
                        pass
            except (OSError, ChildProcessError):
                pass
            self.pid = None

        if self.fd:
            try:
                os.close(self.fd)
            except OSError:
                pass
            self.fd = None

        # Run cleanup handlers
        for handler in self._cleanup_handlers:
            try:
                handler()
            except Exception:
                pass
        self._cleanup_handlers.clear()

    def send_keys(self, keys: str, delay: float = 0.05):
        """
        Send keystrokes to datapainter.

        Args:
            keys: String of keys to send. Can include special key names like
                  'TAB', 'ESC', 'UP', 'DOWN', 'LEFT', 'RIGHT', 'ENTER', 'BACKSPACE', 'DELETE'.
                  Regular characters are sent as-is.
            delay: Delay between keys in seconds
        """
        # Special keys that need to be recognized as multi-character sequences
        special_keys = ['UP', 'DOWN', 'LEFT', 'RIGHT', 'BACKSPACE', 'DELETE', 'ENTER', 'ESC', 'TAB']

        i = 0
        while i < len(keys):
            # Check if we're at the start of a special key sequence
            found_special = False
            for special in special_keys:
                if keys[i:i+len(special)] == special:
                    self._send_key(special)
                    i += len(special)
                    found_special = True
                    break

            if not found_special:
                # Regular character
                self._send_key(keys[i])
                i += 1

            time.sleep(delay)
            self._read_output()

    def _send_key(self, key: str):
        """Send a single key to the PTY."""
        if self.fd is None:
            raise RuntimeError("Process not started")

        # Handle special keys
        key_codes = {
            'UP': b'\x1b[A',
            'DOWN': b'\x1b[B',
            'LEFT': b'\x1b[C',
            'RIGHT': b'\x1b[D',
            'KEY_UP': b'\x1b[A',      # Alternative format
            'KEY_DOWN': b'\x1b[B',    # Alternative format
            'KEY_LEFT': b'\x1b[C',    # Alternative format
            'KEY_RIGHT': b'\x1b[D',   # Alternative format
            'BACKSPACE': b'\x7f',
            'DELETE': b'\x1b[3~',
            'ENTER': b'\r',
            'ESC': b'\x1b',
            'TAB': b'\t',
        }

        if key in key_codes:
            os.write(self.fd, key_codes[key])
        else:
            os.write(self.fd, key.encode('utf-8'))

    def _read_output(self, timeout: float = 0.1):
        """Read available output from PTY and feed to terminal emulator."""
        if self.fd is None:
            return

        end_time = time.time() + timeout
        while time.time() < end_time:
            ready, _, _ = select.select([self.fd], [], [], 0.01)
            if ready:
                try:
                    data = os.read(self.fd, 4096)
                    if data:
                        self.stream.feed(data)
                except OSError:
                    break
            else:
                # No data available
                break

    def get_screen(self) -> pyte.Screen:
        """
        Get current screen state.

        Returns:
            pyte.Screen object with current display
        """
        self._read_output()
        return self.screen

    def get_display_lines(self) -> List[str]:
        """
        Get screen content as list of strings (one per line).

        Returns:
            List of strings, one for each row
        """
        self._read_output()
        return [self.screen.display[row] for row in range(self.height)]

    def get_cell(self, row: int, col: int) -> str:
        """
        Get character at specific screen position.

        Args:
            row: Row number (0-based)
            col: Column number (0-based)

        Returns:
            Character at position
        """
        self._read_output()
        return self.screen.display[row][col]

    def wait_for_text(self, text: str, timeout: float = 2.0) -> bool:
        """
        Wait for specific text to appear on screen.

        Args:
            text: Text to wait for
            timeout: Maximum time to wait in seconds

        Returns:
            True if text appeared, False if timeout
        """
        end_time = time.time() + timeout
        while time.time() < end_time:
            self._read_output()
            display = '\n'.join(self.get_display_lines())
            if text in display:
                return True
            time.sleep(0.05)
        return False

    def assert_char_at(self, row: int, col: int, expected: str, msg: str = ""):
        """
        Assert that a specific character appears at a position.

        Args:
            row: Row number (0-based)
            col: Column number (0-based)
            expected: Expected character
            msg: Optional error message
        """
        actual = self.get_cell(row, col)
        if actual != expected:
            display = '\n'.join(self.get_display_lines())
            error_msg = (
                f"{msg}\n" if msg else "" +
                f"Expected '{expected}' at ({row}, {col}) but got '{actual}'\n"
                f"\nFull screen:\n{display}"
            )
            raise AssertionError(error_msg)

    def assert_text_in_screen(self, text: str, msg: str = ""):
        """
        Assert that text appears somewhere on screen.

        Args:
            text: Text to search for
            msg: Optional error message
        """
        display = '\n'.join(self.get_display_lines())
        if text not in display:
            error_msg = (
                f"{msg}\n" if msg else "" +
                f"Expected text '{text}' not found in screen\n"
                f"\nFull screen:\n{display}"
            )
            raise AssertionError(error_msg)

    def save_snapshot(self, name: str, directory: str = "snapshots"):
        """
        Save current screen state as a snapshot file.

        Args:
            name: Snapshot name (will add .txt extension)
            directory: Directory to save snapshots
        """
        os.makedirs(directory, exist_ok=True)
        snapshot_path = os.path.join(directory, f"{name}.txt")

        with open(snapshot_path, 'w') as f:
            # Add markers around each line to show trailing spaces
            for i, line in enumerate(self.get_display_lines()):
                f.write(f"{i:2d} |{line}|\n")
