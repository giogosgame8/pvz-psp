import unittest
from prepare_polish024 import sample_transform

def pose(**values):
    return dict(x=0,y=0,kx=0,ky=0,sx=1,sy=1,a=1,f=0,i='image',**values)

class AuthoredSamplingTests(unittest.TestCase):
    def test_integer_pose_is_unchanged(self):
        frames=[pose(),pose()]
        self.assertIs(sample_transform(frames,1,0,2),frames[1])

    def test_transform_interpolation_preserves_discrete_keys(self):
        a=pose();b=pose();b.update(x=-20,y=30,kx=90,sx=.5,a=0,f=-1,i='other')
        sampled=sample_transform([a,b],.5,0,2)
        self.assertEqual((sampled['x'],sampled['y'],sampled['kx'],sampled['sx'],sampled['a']),(-10,15,45,.75,.5))
        self.assertEqual((sampled['f'],sampled['i']),(0,'image'))
        self.assertEqual(a['x'],0)

    def test_nonlooping_clip_holds_its_last_pose(self):
        a=pose();b=pose();b['x']=20
        self.assertEqual(sample_transform([a,b],1.5,0,2)['x'],20)

    def test_loop_interpolates_to_its_own_first_pose(self):
        a=pose();b=pose();b['x']=20
        self.assertEqual(sample_transform([a,b],1.5,0,2,True)['x'],10)

    def test_clip_cannot_read_a_neighbouring_sequence(self):
        with self.assertRaises(ValueError):sample_transform([pose(),pose()],2,0,2)

if __name__=='__main__':unittest.main()
