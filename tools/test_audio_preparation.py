"""Check bundled declarations against the original Foley group names."""
import json
import unittest

from prepare_audio018 import REFERENCE, normalize_sound_names


class SoundDeclarationTests(unittest.TestCase):
    def test_all_bundled_foley_variants_resolve_without_reference_executable(self):
        facts=json.loads(REFERENCE.read_text())
        sounds=normalize_sound_names(facts['sounds'])
        self.assertEqual(len(sounds),194)
        self.assertEqual(len(facts['groups']),104)
        for group in facts['groups']:
            for variant in group['variants']:
                with self.subTest(group=group['name'],variant=variant):
                    self.assertIn(variant,sounds)
        self.assertEqual(sounds['SOUND_POINTS'],'points')

    def test_short_and_executable_names_have_identical_sample_mappings(self):
        short={'POINTS':'points','CHOMP':'chomp'}
        full={'SOUND_POINTS':'points','SOUND_CHOMP':'chomp'}
        self.assertEqual(normalize_sound_names(short),normalize_sound_names(full))

    def test_conflicting_aliases_are_rejected(self):
        with self.assertRaisesRegex(ValueError,'Conflicting sound declarations'):
            normalize_sound_names({'POINTS':'points','SOUND_POINTS':'different'})


if __name__=='__main__':
    unittest.main()
