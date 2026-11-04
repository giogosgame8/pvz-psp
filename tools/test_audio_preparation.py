"""Check bundled declarations against the original Foley group names."""
import json
import unittest
import array
import random

from prepare_audio018 import REFERENCE, normalize_sound_names
from prepare_music import normalize_samples


class SoundDeclarationTests(unittest.TestCase):
    def test_music_normalization_matches_original_rounding_for_signed_samples(self):
        rng=random.Random(186)
        for values in ([0]*20,[-1,0,1],list(range(-32768,32768)),
                       [rng.randrange(-8100,8101) for _ in range(12000)]):
            samples=array.array('h',values);peak=max((abs(s) for s in samples),default=0)
            expected=array.array('h',(max(-32768,min(32767,round(s*(5000/peak)))) for s in samples)) if peak else samples
            actual,normalized_peak=normalize_samples(samples)
            self.assertEqual(actual.tobytes(),expected.tobytes())
            self.assertEqual(normalized_peak,max((abs(s) for s in expected),default=0))

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
