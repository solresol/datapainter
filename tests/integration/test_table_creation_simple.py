#!/usr/bin/env python3
"""Simple test to debug table creation dialog."""

import os
import subprocess
import tempfile


def test_simple():
    """Simple test to see if the dialog appears."""
    # Create temporary database
    with tempfile.NamedTemporaryFile(suffix=".db", delete=False) as f:
        db_path = f.name

    try:
        # Start datapainter and immediately send ESC to exit
        proc = subprocess.Popen(
            ["./build/datapainter", "--database", db_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

        # Send 'c' to create table, then ESC to cancel
        try:
            proc.stdin.write(b"c")
            proc.stdin.flush()
            proc.stdin.write(b"\x1b")  # ESC
            proc.stdin.flush()
        except BrokenPipeError:
            pass

        # Wait and get output
        try:
            stdout, stderr = proc.communicate(timeout=2)
        except subprocess.TimeoutExpired:
            proc.kill()
            stdout, stderr = proc.communicate()

        print("=== STDOUT ===")
        print(stdout.decode('utf-8', errors='replace'))
        print("\n=== STDERR ===")
        print(stderr.decode('utf-8', errors='replace'))
        print("\n=== EXIT CODE ===")
        print(proc.returncode)

    finally:
        # Clean up
        if os.path.exists(db_path):
            os.unlink(db_path)


if __name__ == "__main__":
    test_simple()
