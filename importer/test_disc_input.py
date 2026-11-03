"""Input-detection regressions; no original game resources are required."""
from pathlib import Path
import tempfile
import unittest
from unittest.mock import Mock, patch

import pvz_import


class DiscInputTests(unittest.TestCase):
    def inspect(self, entries, metadata=None):
        iso = Mock()
        iso.walk.return_value = entries
        with patch.object(pvz_import, 'ISO', return_value=iso), \
                patch.object(pvz_import, 'parse_sfo', return_value=metadata or {}):
            try:
                return pvz_import.inspect_or_import(Path('owned-copy.iso'), None)
            finally:
                iso.file.close.assert_called_once()

    def test_psp_image_explains_required_source(self):
        entries = [{'path': '/PSP_GAME/PARAM.SFO', 'directory': False, 'size': 64, 'lba': 20}]
        with self.assertRaisesRegex(ValueError, 'PSP image.*PS3.*BLUS30852.*NPUA_30059'):
            self.inspect(entries)

    def test_unrelated_iso_explains_missing_ps3_metadata(self):
        with self.assertRaisesRegex(ValueError, '/PS3_GAME/PARAM.SFO.*BLUS30852'):
            self.inspect([])

    def test_oversized_sfo_is_rejected(self):
        entries = [{'path': '/PS3_GAME/PARAM.SFO', 'directory': False, 'size': 1048577, 'lba': 20}]
        with self.assertRaisesRegex(ValueError, 'too large'):
            self.inspect(entries)

    def test_other_ps3_build_identifies_supported_edition(self):
        entries = [{'path': '/PS3_GAME/PARAM.SFO', 'directory': False, 'size': 64, 'lba': 20}]
        with self.assertRaisesRegex(ValueError, 'Unsupported disc build: OTHER 01.00.*BLUS30852'):
            self.inspect(entries, {'TITLE_ID': 'OTHER', 'APP_VER': '01.00'})

    def test_supported_metadata_still_requires_original_archives(self):
        entries = [{'path': '/PS3_GAME/PARAM.SFO', 'directory': False, 'size': 64, 'lba': 20}]
        with self.assertRaisesRegex(ValueError, 'archives or music files are missing'):
            self.inspect(entries, {'TITLE_ID': 'BLUS30852', 'APP_VER': '01.00'})

    def test_existing_output_remains_untouched(self):
        with tempfile.TemporaryDirectory() as temp:
            target = Path(temp)
            sentinel = target / 'keep.txt'
            sentinel.write_text('keep', encoding='utf-8')
            with self.assertRaisesRegex(ValueError, 'new directory'):
                pvz_import.inspect_or_import(Path('owned-copy.iso'), target)
            self.assertEqual(sentinel.read_text(encoding='utf-8'), 'keep')


if __name__ == '__main__':
    unittest.main()
