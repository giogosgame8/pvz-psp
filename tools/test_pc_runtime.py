"""Portable setup must locate its user folder and dispatch its own worker."""
from pathlib import Path
import subprocess
import sys
import unittest
from unittest.mock import patch

from pc_runtime import package_root, preparation_command, resource_root, ending_command


class PortableSetupTests(unittest.TestCase):
    def test_frozen_paths_keep_user_files_beside_exe_and_resources_in_bundle(self):
        executable = str(Path('portable folder/Setup.exe').resolve())
        resources = str(Path('temporary runtime').resolve())
        with patch.object(sys, 'frozen', True, create=True), patch.object(sys, 'executable', executable), patch.object(sys, '_MEIPASS', resources, create=True):
            self.assertEqual(package_root(), Path(executable).parent)
            self.assertEqual(resource_root(), Path(resources))
            command = preparation_command(Path('owned.iso'), Path('staging/data'), Path('source'))
            self.assertEqual(command, [executable, '--prepare-worker', '--iso', 'owned.iso', '--output', str(Path('staging/data'))])
            self.assertEqual(ending_command(Path('chunk jobs.json')),
                             [executable, '--ending-worker', 'chunk jobs.json'])

    def test_source_worker_entry_point_dispatches_to_converter(self):
        entry = Path(__file__).with_name('windows_setup.py')
        result = subprocess.run([sys.executable, str(entry), '--prepare-worker', '--help'], capture_output=True, text=True, timeout=20)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('--imported-dir', result.stdout)
        self.assertIn('--output', result.stdout)
        self.assertNotIn('--check-only', result.stdout)


if __name__ == '__main__':
    unittest.main()
