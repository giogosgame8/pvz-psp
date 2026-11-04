"""Windows setup: select an owned PS3 disc and produce a ready-to-copy PSP folder."""
from __future__ import annotations

import argparse
import contextlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import time

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'importer'))
from pvz_import import inspect_or_import


def acquire_setup_lock(path):
    """OS lock is released even if setup is interrupted or the PC restarts."""
    lock = path.open('a+b')
    try:
        lock.seek(0, os.SEEK_END)
        if lock.tell() == 0:
            lock.write(b'0')
            lock.flush()
        lock.seek(0)
        if os.name == 'nt':
            import msvcrt
            msvcrt.locking(lock.fileno(), msvcrt.LK_NBLCK, 1)
        else:
            import fcntl
            fcntl.flock(lock.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
        return lock
    except OSError:
        lock.close()
        raise ValueError('Setup is already running in this folder. Keep its window open and wait for READY.') from None


def file_dialog(title, patterns):
    import tkinter as tk
    from tkinter import filedialog
    root = tk.Tk()
    root.withdraw()
    root.attributes('-topmost', True)
    try:
        selected = filedialog.askopenfilename(parent=root, title=title, filetypes=patterns)
        return Path(selected) if selected else None
    finally:
        root.destroy()


def ask(title, message):
    import tkinter as tk
    from tkinter import messagebox
    root = tk.Tk()
    root.withdraw()
    root.attributes('-topmost', True)
    try:
        return messagebox.askyesno(title, message, parent=root)
    finally:
        root.destroy()


def supports_music(path):
    try:
        result = subprocess.run([str(path), '-hide_banner', '-demuxers'],
                                capture_output=True, text=True, encoding='utf-8',
                                errors='replace', timeout=20)
        return result.returncode == 0 and bool(re.search(r'\blibopenmpt\b', result.stdout))
    except (OSError, subprocess.TimeoutExpired):
        return False


def find_ffmpeg():
    candidates = []
    on_path = shutil.which('ffmpeg')
    if on_path:
        candidates.append(Path(on_path))
    local = os.environ.get('LOCALAPPDATA')
    if local and os.name == 'nt':
        winget = Path(local) / 'Microsoft/WinGet'
        candidates.append(winget / 'Links/ffmpeg.exe')
        packages = winget / 'Packages'
        if packages.is_dir():
            for package in packages.glob('Gyan.FFmpeg_*'):
                candidates.extend(package.glob('*/bin/ffmpeg.exe'))
    seen = set()
    for candidate in candidates:
        key = str(candidate).casefold()
        if key not in seen and candidate.is_file():
            seen.add(key)
            if supports_music(candidate):
                return candidate.resolve()
    return None


def ensure_dependencies(allow_install):
    if importlib.util.find_spec('PIL') is None:
        if not allow_install or not ask('PC setup', 'Pillow is required to prepare the pictures.\n\nInstall Pillow for this Python now?'):
            raise ValueError('Pillow is required. Run Setup.cmd again and allow its installation.')
        print('Installing Pillow...', flush=True)
        command = [sys.executable, '-m', 'pip', 'install']
        if sys.prefix == sys.base_prefix:
            command.append('--user')
        subprocess.run([*command, 'Pillow'], check=True)
    ffmpeg = find_ffmpeg()
    if ffmpeg is None and allow_install:
        winget = shutil.which('winget') if os.name == 'nt' else None
        if winget and ask('PC setup', 'FFmpeg is required for the music and sounds.\n\nInstall Gyan.FFmpeg using Windows Package Manager?\nAny agreement prompts will appear in the setup window.'):
            print('Installing FFmpeg. Complete any agreement prompts in this window.', flush=True)
            # Leave agreement acceptance to the person running setup.
            subprocess.run([winget, 'install', '--id', 'Gyan.FFmpeg', '--exact', '--source', 'winget'], check=True)
            ffmpeg = find_ffmpeg()
        if ffmpeg is None:
            print('Select ffmpeg.exe from an installed full build of FFmpeg.', flush=True)
            chosen = file_dialog('Select ffmpeg.exe (full build with libopenmpt)', [('FFmpeg', 'ffmpeg.exe'), ('Programs', '*.exe')])
            if chosen and supports_music(chosen):
                ffmpeg = chosen.resolve()
    if ffmpeg is None:
        raise ValueError('FFmpeg with music support (libopenmpt) was not found. Install the full FFmpeg build, then run Setup.cmd again.')
    return ffmpeg


def validate_prepared(folder):
    manifest = json.loads((folder / 'manifest.json').read_text(encoding='utf-8'))
    if manifest.get('version') != '0.18.1' or manifest.get('disc_title_id') != 'BLUS30852':
        raise ValueError('Prepared files do not match this game package.')
    assets = manifest.get('assets', [])
    if not assets:
        raise ValueError('No prepared graphics were produced.')
    for asset in assets:
        relative = Path(asset['file'])
        if relative.is_absolute() or '..' in relative.parts:
            raise ValueError('Invalid prepared asset path.')
        path = folder / relative
        if not path.is_file() or path.stat().st_size == 0:
            raise ValueError(f'Prepared file is missing: {relative.name}')
    for name in ('foley.pvf', 'music-index.json', 'ending.pvj', 'ending.json'):
        if not (folder / name).is_file() or (folder / name).stat().st_size == 0:
            raise ValueError(f'Preparation did not finish: {name} is missing.')
    for song in json.loads((folder / 'music-index.json').read_text(encoding='utf-8')):
        relative = Path(song['file'])
        if relative.is_absolute() or '..' in relative.parts or not (folder / relative).is_file():
            raise ValueError('A prepared music track is missing.')
    return manifest


def install_prepared(staging, destination):
    """Keep an existing installation until a complete replacement is ready."""
    validate_prepared(staging)
    backup = None
    if destination.exists():
        backup = destination.with_name(destination.name + '-backup-' + time.strftime('%Y%m%d-%H%M%S'))
        number = 1
        while backup.exists():
            backup = destination.with_name(destination.name + '-backup-' + time.strftime('%Y%m%d-%H%M%S') + f'-{number}')
            number += 1
        destination.rename(backup)
    try:
        staging.rename(destination)
    except OSError:
        if backup is not None:
            backup.rename(destination)
        raise
    return backup


def prepare_game(source, destination, ffmpeg, log, root=ROOT):
    destination.parent.mkdir(parents=True, exist_ok=True)
    # Work on the same drive as the final data, without touching old data/saves.
    staging = Path(tempfile.mkdtemp(prefix='pvz-preparing-', dir=destination.parent)) / 'data'
    env = os.environ.copy()
    env['PATH'] = str(ffmpeg.parent) + os.pathsep + env.get('PATH', '')
    env['PYTHONUTF8'] = '1'
    command = [sys.executable, '-u', str(root / 'tools/prepare_assets.py'), '--iso', str(source), '--output', str(staging)]
    print('Preparing game files. This can take several minutes; keep this window open.', flush=True)
    count = 0
    music_count = 0
    stage = 'animations'
    process = subprocess.Popen(command, cwd=root, env=env, stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT, text=True, encoding='utf-8', errors='replace')
    try:
        for line in process.stdout:
            log.write(line)
            log.flush()
            if line.startswith('Stage: '):
                stage = line.removeprefix('Stage: ').strip()
                count = 0
                print(f'Preparing {stage}...', flush=True)
            elif line.startswith('Prepared original music: '):
                music_count += 1
                name = line.removeprefix('Prepared original music: ').split(',')[0]
                print(f'Preparing music... {music_count}/13 ({name})', flush=True)
            elif match := re.match(r'Prepared (\d+)/(\d+) ', line):
                done, total = int(match[1]), int(match[2])
                if done % 20 == 0 or done == total:
                    print(f'Preparing sound effects... {done}/{total}', flush=True)
            elif re.match(r'Prepared [^:]+: \d+ frames, ', line):
                count += 1
                if count % 20 == 0:
                    print(f'Preparing {stage}... {count} animations processed', flush=True)
            if line.startswith("{'frames':"):
                match = re.search(r"'frames': (\d+), 'total': (\d+)", line)
                if match and (int(match[1]) % 240 == 0 or int(match[1]) == int(match[2])):
                    print(f'Preparing ending... {int(match[1]) * 100 // int(match[2])}%', flush=True)
        code = process.wait()
    except BaseException:
        if process.poll() is None:
            process.terminate()
            process.wait()
        raise
    finally:
        process.stdout.close()
    if code:
        raise ValueError('Preparation stopped. Details are in setup.log. Your existing game data and saves were kept.')
    print('Checking prepared files...', flush=True)
    backup = install_prepared(staging, destination)
    try:
        staging.parent.rmdir()  # Only the empty directory created above.
    except OSError:
        pass
    return backup


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--iso', type=Path, help='Skip the ISO picker')
    parser.add_argument('--output', type=Path, help='Prepared data folder; default: PSP/GAME/PVZPSP/data')
    parser.add_argument('--check-only', action='store_true', help='Check the source and installed dependencies without preparing')
    parser.add_argument('--no-install', action='store_true', help='Report missing dependencies without prompting to install')
    args = parser.parse_args()
    if sys.version_info < (3, 10):
        print('Python 3.10 or newer is required. Update Python, then run Setup.cmd again.')
        return 1
    destination = (args.output or ROOT / 'PSP/GAME/PVZPSP/data').resolve()
    log_path = ROOT / 'setup.log'
    lock = None
    try:
        lock = acquire_setup_lock(ROOT / 'setup.lock')
        binary = ROOT / 'PSP/GAME/PVZPSP/EBOOT.PBP'
        if not binary.is_file():
            raise ValueError('EBOOT.PBP is missing. Extract the game download ZIP first. GitHub Source code downloads do not include the game executable.')
        source = args.iso or file_dialog('Choose your ORIGINAL PS3 Plants vs. Zombies disc ISO', [('Disc image', '*.iso'), ('All files', '*.*')])
        if source is None:
            print('Setup cancelled. No game files were changed.')
            return 0
        source = source.resolve()
        with log_path.open('w', encoding='utf-8') as log:
            print('Checking your PS3 disc...', flush=True)
            with contextlib.redirect_stdout(log), contextlib.redirect_stderr(log):
                inspect_or_import(source, None)
            print('PS3 disc verified.', flush=True)
            ffmpeg = ensure_dependencies(not args.no_install and not args.check_only)
            if args.check_only:
                print('Checks passed: supported PS3 disc, Pillow, FFmpeg music support and game executable.', flush=True)
                return 0
            if destination.exists() and not ask('Replace prepared data?', 'Game data already exists.\n\nPrepare a new copy? The old data will be kept in a backup folder. Player saves will stay unchanged.'):
                print('Setup cancelled. Existing data was kept.')
                return 0
            backup = prepare_game(source, destination, ffmpeg, log)
        print('\nREADY! Copy this folder to PSP/GAME on your PSP:\n' + str(binary.parent), flush=True)
        if backup:
            print('Old game data was kept at: ' + str(backup), flush=True)
        print('You can now start the game from the PSP Game menu.', flush=True)
        return 0
    except KeyboardInterrupt:
        print('\nSetup stopped. Existing game data and saves were kept.')
        return 1
    except (OSError, ValueError, subprocess.SubprocessError, json.JSONDecodeError) as error:
        print('\nSetup could not finish: ' + str(error), flush=True)
        print('If you need help, include the error and setup.log in your bug report. Remove personal information first.', flush=True)
        return 1
    finally:
        if lock is not None:
            lock.close()


if __name__ == '__main__':
    raise SystemExit(main())
