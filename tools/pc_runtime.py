"""Paths and subprocess settings shared by source and portable Windows setup."""
import os
from pathlib import Path
import subprocess
import sys


def is_frozen():
    return bool(getattr(sys, 'frozen', False))


def package_root():
    if is_frozen():
        return Path(sys.executable).resolve().parent
    return Path(__file__).resolve().parents[1]


def resource_root():
    if is_frozen():
        return Path(sys._MEIPASS)
    return package_root()


def hidden_process_options():
    # Workers and FFmpeg have no interactive UI; keep one setup console.
    return {'creationflags': subprocess.CREATE_NO_WINDOW} if os.name == 'nt' else {}


def preparation_command(source, destination, root):
    if is_frozen():
        command = [sys.executable, '--prepare-worker']
    else:
        command = [sys.executable, '-u', str(root / 'tools/prepare_assets.py')]
    return [*command, '--iso', str(source), '--output', str(destination)]
