"""Build a portable Setup.exe with the Python runtime, Pillow and ISO importer."""
import argparse
import importlib.metadata
import json
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'build-pc/dist')
    args = parser.parse_args()
    output = args.output.resolve()
    work = ROOT / 'build-pc/pyinstaller'
    output.mkdir(parents=True, exist_ok=True)
    command = [sys.executable, '-m', 'PyInstaller', '--noconfirm', '--clean',
               '--onefile', '--console', '--noupx', '--name', 'Setup',
               '--distpath', str(output), '--workpath', str(work),
               '--specpath', str(ROOT / 'build-pc'),
               '--paths', str(ROOT / 'tools'), '--paths', str(ROOT / 'importer'),
               '--add-data', str(ROOT / 'tools/reference/FOLEY-REFERENCE-0.18.json') + ':tools/reference']
    for module in ('numpy', 'matplotlib', 'IPython', 'PyQt5', 'PySide6'):
        command.extend(['--exclude-module', module])
    command.append(str(ROOT / 'tools/windows_setup.py'))
    subprocess.run(command, cwd=ROOT, check=True)
    licenses = output / 'windows-setup-licenses'
    licenses.mkdir(exist_ok=True)
    shutil.copy2(Path(sys.base_prefix) / 'LICENSE.txt', licenses / 'PYTHON-LICENSE.txt')
    for package, name in (('pillow', 'PILLOW'), ('pyinstaller', 'PYINSTALLER')):
        distribution = importlib.metadata.distribution(package)
        for path in distribution.files:
            if 'licenses/' in str(path).replace('\\', '/') and Path(path).name.upper().startswith(('LICENSE', 'COPYING')):
                shutil.copy2(distribution.locate_file(path), licenses / (name + '-' + Path(path).name + '.txt'))
    tk_license = Path(sys.base_prefix) / 'tcl/tk8.6/license.terms'
    if tk_license.is_file():
        shutil.copy2(tk_license, licenses / 'TCL-TK-license.terms.txt')
    versions = {name: importlib.metadata.version(name) for name in ('pillow', 'pyinstaller', 'pyinstaller-hooks-contrib')}
    versions['python'] = sys.version.split()[0]
    versions['platform'] = sys.platform
    (licenses / 'VERSIONS.json').write_text(json.dumps(versions, indent=2) + '\n', encoding='utf-8')
    print('Built portable setup:', output / 'Setup.exe')


if __name__ == '__main__':
    main()
