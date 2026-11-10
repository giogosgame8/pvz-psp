"""Installer integrity tests without original resources or dependency downloads."""
import contextlib
import io
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch

from setup_assets import acquire_setup_lock, install_prepared, prepare_game, validate_prepared
from prepared_data import DATA_VERSION,write_integrity


def fixture(folder):
    folder.mkdir(parents=True)
    (folder / 'manifest.json').write_text(json.dumps({
        'version': DATA_VERSION, 'disc_title_id': 'BLUS30852', 'source_iso_sha256': 'test-iso',
        'assets': [{'file': 'plant.pva'}]}), encoding='utf-8')
    for name in ('menu-motion.bin','menu-motion.json','plant.pva', 'foley.pvf', 'ending.pvj', 'ending.json', 'music0.wav', 'assets.idx', 'XMB_ICON0.PNG', 'XMB_PIC1.PNG', 'zombie_motion.bin', 'seeds.bin', 'disc_font.bin', 'strings.bin', 'almanac.bin', 'story-index.json', 'music-checks.bin'):
        (folder / name).write_bytes(b'test fixture')
    (folder / 'shoot-alignment.bin').write_bytes(b'PPA1'+bytes.fromhex('03000000500000003c000000')+bytes(1920))
    (folder / 'music-index.json').write_text('[{"file":"music0.wav"}]', encoding='utf-8')
    write_integrity(folder)


class SetupTests(unittest.TestCase):
    def test_reuse_requires_matching_iso_and_every_file_hash(self):
        with tempfile.TemporaryDirectory() as temporary:
            folder=Path(temporary)/'data';fixture(folder)
            validate_prepared(folder,'test-iso')
            with self.assertRaisesRegex(ValueError,'different source'):
                validate_prepared(folder,'another-iso')
            file=folder/'music0.wav';file.write_bytes(b'x'*file.stat().st_size)
            with self.assertRaisesRegex(ValueError,'failed verification: music0.wav'):
                validate_prepared(folder,'test-iso')

    def test_duplicate_setup_is_blocked_and_lock_releases(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'setup.lock'
            first = acquire_setup_lock(path)
            try:
                with self.assertRaisesRegex(ValueError, 'already running'):
                    acquire_setup_lock(path)
            finally:
                first.close()
            second = acquire_setup_lock(path)
            second.close()

    def test_incomplete_output_keeps_installed_data(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            old = root / 'data'
            old.mkdir()
            (old / 'keep.txt').write_text('existing data')
            staging = root / 'preparing'
            fixture(staging)
            (staging / 'ending.pvj').unlink()
            with self.assertRaisesRegex(ValueError, 'ending.pvj'):
                install_prepared(staging, old)
            self.assertEqual((old / 'keep.txt').read_text(), 'existing data')

    def test_complete_output_keeps_backup_and_player_save(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            old = root / 'data'
            old.mkdir()
            (old / 'keep.txt').write_text('existing data')
            save = root / 'profile.dat'
            save.write_bytes(b'player save')
            staging = root / 'preparing'
            fixture(staging)
            backup = install_prepared(staging, old)
            self.assertEqual((backup / 'keep.txt').read_text(), 'existing data')
            self.assertTrue((old / 'plant.pva').is_file())
            self.assertEqual(save.read_bytes(), b'player save')

    def test_failed_install_restores_previous_data(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            old = root / 'data'
            old.mkdir()
            (old / 'keep.txt').write_text('existing data')
            staging = root / 'preparing'
            fixture(staging)
            original_rename = Path.rename

            def rename(path, target):
                if path == staging:
                    raise OSError('simulated locked destination')
                return original_rename(path, target)

            with patch.object(Path, 'rename', autospec=True, side_effect=rename):
                with self.assertRaisesRegex(OSError, 'locked destination'):
                    install_prepared(staging, old)
            self.assertEqual((old / 'keep.txt').read_text(), 'existing data')

    def test_missing_song_and_escaping_paths_are_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            folder = Path(temp) / 'data'
            fixture(folder)
            (folder / 'music-index.json').write_text('[{"file":"../outside.wav"}]')
            with self.assertRaisesRegex(ValueError, 'music track'):
                validate_prepared(folder)

    def test_real_child_process_handles_spaces_and_unicode_paths(self):
        with tempfile.TemporaryDirectory(prefix='PC setup ') as temp:
            root = Path(temp) / 'folder Î± with spaces'
            tools = root / 'tools'
            tools.mkdir(parents=True)
            # A tiny stand-in exercises subprocess arguments, logs and final placement.
            script = "import sys\n"+"sys.path.insert(0,"+repr(str(Path(__file__).parent))+')\n'+"import argparse\nfrom pathlib import Path\nfrom test_setup_assets import fixture\np=argparse.ArgumentParser();p.add_argument('--iso');p.add_argument('--output');a=p.parse_args()\nf=Path(a.output);fixture(f)\nprint('Prepared test fixture',flush=True)\n"
            (tools / 'prepare_assets.py').write_text(script, encoding='utf-8')
            with (tools / 'prepare_assets.py').open('a', encoding='utf-8') as child:
                child.write("(f/'shoot-alignment.bin').write_bytes(b'PPA1'+bytes.fromhex('03000000500000003c000000')+bytes(1920))\n")
                child.write("print('Stage: music',flush=True)\n")
                child.write("print('Prepared original music: roof, 108.1s',flush=True)\n")
                child.write("print('Stage: menu graphics',flush=True)\n")
                child.write("print('Stage: ending animation',flush=True)\n")
                child.write("print(\"{'frames': 240, 'total': 3810}\",flush=True)\n")
                child.write("print('Stage: sound effects',flush=True)\n")
                child.write("print('Prepared 194/194 zombiesplash.mpg',flush=True)\n")
            destination = root / 'PSP/GAME/PVZPSP/data'
            log = io.StringIO()
            console = io.StringIO()
            with contextlib.redirect_stdout(console):
                backup = prepare_game(root / 'owned copy.iso', destination, Path(sys.executable), log, root=root)
            self.assertIsNone(backup)
            self.assertIn('Prepared test fixture', log.getvalue())
            self.assertTrue((destination / 'manifest.json').is_file())
            self.assertEqual(list(destination.parent.glob('pvz-preparing-*')), [])
            self.assertIn('Preparing music... 1/13 (roof)', console.getvalue())
            self.assertIn('Preparing menu graphics...', console.getvalue())
            self.assertIn('Preparing ending... 6%', console.getvalue())
            self.assertIn('Preparing sound effects... 194/194', console.getvalue())
            self.assertIn('Checking prepared files...', console.getvalue())
            self.assertNotIn('Preparing graphics...', console.getvalue())


if __name__ == '__main__':
    unittest.main()
