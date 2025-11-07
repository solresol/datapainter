#!/usr/bin/env python3
"""
Debug script to diagnose PTY test framework issues.
This will help us understand why the child process is failing.
"""

import os
import sys
import time
import pty
import select
import tempfile

# Add parent directory to path
sys.path.insert(0, os.path.dirname(__file__))

from pathlib import Path

def find_datapainter():
    """Find the datapainter executable."""
    repo_root = Path(__file__).resolve().parent.parent.parent
    candidates = [
        repo_root / "build" / "datapainter",
        repo_root / "datapainter",
    ]

    for candidate in candidates:
        if candidate.exists():
            return str(candidate.resolve())

    raise FileNotFoundError("Could not locate datapainter executable")


def test_pty_basic():
    """Test basic PTY functionality with datapainter."""
    print("=== PTY Diagnostic Test ===\n")

    # Find executable
    datapainter_path = find_datapainter()
    print(f"Using datapainter: {datapainter_path}")

    # Create temp database
    fd, temp_db = tempfile.mkstemp(suffix=".db")
    os.close(fd)
    print(f"Created temp database: {temp_db}")

    # Set up environment
    env = os.environ.copy()
    env['TERM'] = 'xterm-256color'

    print("\nForking PTY...")
    pid, master_fd = pty.fork()

    if pid == 0:
        # Child process
        args = [
            datapainter_path,
            "--database", temp_db,
            "--table", "test_points"
        ]
        print(f"Child executing: {' '.join(args)}", file=sys.stderr)
        os.execve(datapainter_path, args, env)
        # Should never reach here
        sys.exit(1)

    # Parent process
    print(f"Child process PID: {pid}")
    print("Waiting for initial output...")

    # Read initial output
    try:
        for i in range(10):
            ready, _, _ = select.select([master_fd], [], [], 0.5)
            if ready:
                try:
                    data = os.read(master_fd, 4096)
                    print(f"\n[Iteration {i}] Read {len(data)} bytes from child")
                    print(f"Data: {data[:100]}")  # Print first 100 bytes
                except OSError as e:
                    print(f"\n[Iteration {i}] OSError reading from PTY: {e}")
                    break
            else:
                print(f"[Iteration {i}] No data available yet")

            # Check if child is still alive
            try:
                child_pid, status = os.waitpid(pid, os.WNOHANG)
                if child_pid != 0:
                    print(f"\nChild process exited! PID: {child_pid}, Status: {status}")
                    if os.WIFEXITED(status):
                        print(f"Exit code: {os.WEXITSTATUS(status)}")
                    if os.WIFSIGNALED(status):
                        print(f"Killed by signal: {os.WTERMSIG(status)}")
                    break
                else:
                    print(f"[Iteration {i}] Child still alive")
            except ChildProcessError:
                print("\nChild process already exited (ChildProcessError)")
                break

        # Try to write to PTY
        print("\n\nAttempting to write to PTY...")
        try:
            os.write(master_fd, b'x')
            print("Successfully wrote 'x' to PTY")
        except OSError as e:
            print(f"Failed to write to PTY: {e}")

        # Wait a bit more
        time.sleep(0.5)

    finally:
        # Cleanup
        print("\n\nCleaning up...")
        try:
            os.kill(pid, 15)  # SIGTERM
            os.waitpid(pid, 0)
        except (OSError, ChildProcessError):
            pass

        try:
            os.close(master_fd)
        except OSError:
            pass

        try:
            os.unlink(temp_db)
        except OSError:
            pass

        print("Done!")


if __name__ == "__main__":
    test_pty_basic()
