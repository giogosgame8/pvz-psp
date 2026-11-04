"""Worker limits must adapt to small CPUs and constrained available memory."""
import unittest
from unittest.mock import patch
from preparation_workers import worker_count


class PreparationWorkerTests(unittest.TestCase):
    def count(self, kind, logical, megabytes, jobs=None):
        with patch('preparation_workers.os.cpu_count', return_value=logical), \
             patch('preparation_workers.available_memory', return_value=None if megabytes is None else megabytes*1048576), \
             patch.dict('preparation_workers.os.environ', {}, clear=True):
            return worker_count(kind, jobs)

    def test_large_pc_uses_the_benchmarked_limits(self):
        self.assertEqual([self.count(k,20,16384) for k in ('graphics','audio','music','ending')],[16,8,4,8])

    def test_small_cpu_and_low_memory_keep_a_valid_single_worker(self):
        for kind in ('graphics','audio','ending'):
            self.assertEqual(self.count(kind,1,512),1)
        self.assertEqual(self.count('ending',20,1536),2)
        self.assertEqual(self.count('ending',20,None),2)

    def test_explicit_counts_cannot_exceed_the_memory_budget(self):
        self.assertEqual(self.count('ending',20,1536,8),2)
        for count in (0,-1,33):
            with self.assertRaises(ValueError):self.count('ending',20,16384,count)


if __name__=='__main__':unittest.main()
