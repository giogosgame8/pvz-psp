"""Separate ending processes must preserve frame order and report failures."""
from pathlib import Path
import tempfile
import unittest
from PIL import Image
from unittest.mock import patch
from prepare_ending import ending_chunks,ending_file_access


class EndingWorkerTests(unittest.TestCase):
    def test_temporary_windows_sharing_failure_retries_without_altering_data(self):
        with patch('prepare_ending.time.sleep'),patch('pathlib.Path.read_bytes',side_effect=[PermissionError('locked'),b'exact original bytes']) as read:
            self.assertEqual(ending_file_access(Path('chunk').read_bytes),b'exact original bytes')
            self.assertEqual(read.call_count,2)

    def test_persistent_permission_failure_and_corrupt_data_errors_are_not_hidden(self):
        with patch('prepare_ending.time.sleep'),patch('pathlib.Path.read_bytes',side_effect=PermissionError('locked')) as read:
            with self.assertRaises(PermissionError):ending_file_access(Path('chunk').read_bytes)
            self.assertEqual(read.call_count,40)
        with self.assertRaises(ValueError):ending_file_access(lambda:(_ for _ in ()).throw(ValueError('bad frame')))

    def test_parallel_output_matches_serial_across_chunk_boundaries(self):
        with tempfile.TemporaryDirectory(prefix='ending worker test ') as temporary:
            imported=Path(temporary)
            for archive in ('main','compressed','en'):
                (imported/'archives'/archive).mkdir(parents=True)
            directory=imported/'archives/main/reanim';directory.mkdir()
            Image.new('RGBA',(12,12),(20,190,70,255)).save(directory/'dot.ptx',format='PNG')
            frames=''.join(f'<t><x>{n*20}</x><y>20</y><i>IMAGE_REANIM_DOT</i></t>' for n in range(5))
            (directory/'test.reanim').write_text('<fps>24</fps><track><name>body</name>'+frames+'</track>')
            jobs=[('Test',24,5,0,start,start+1,24,0,None) for start in range(4)]
            expected=list(ending_chunks(jobs,1,imported,[]))
            self.assertEqual(list(ending_chunks(jobs,2,imported,[])),expected)
            self.assertNotEqual(expected[0][0],expected[1][0])

    def test_worker_error_reaches_parent_without_hanging(self):
        with tempfile.TemporaryDirectory() as temporary:
            jobs=[('Test',24,5,0,0,1,0,0,None)]
            with self.assertRaisesRegex(ValueError,'worker stopped'):
                list(ending_chunks(jobs,2,Path(temporary),[]))


if __name__=='__main__':unittest.main()
