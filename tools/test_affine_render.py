"""The optimized compositor must preserve the original full-canvas pixels."""
import math
import random
import unittest
from PIL import Image

from affine_render import composite_affine


class AffineRenderTests(unittest.TestCase):
    def test_matches_full_canvas_for_rotation_scale_shear_and_clipping(self):
        rng = random.Random(7083)
        image = Image.frombytes('RGBA', (23, 19), bytes(rng.randrange(256) for _ in range(23 * 19 * 4)))
        matrices = [(1,0,0,1,0,0), (1,0,0,1,17.25,13.125),
                    (-1,0,0,1,53,12), (1,0,0,-1,12,53),
                    (1,0,0,1,-18,-14), (1,0,0,1,200,200)]
        for _ in range(70):
            angle = rng.uniform(-math.pi, math.pi)
            sx, sy = rng.uniform(.1,3), rng.uniform(.1,3)
            matrices.append((math.cos(angle)*sx,-math.sin(angle)*sy,
                             math.sin(angle)*sx,math.cos(angle)*sy,
                             rng.uniform(-45,100),rng.uniform(-40,90)))
        matrices += [(1,.6,.2,1,20,8), (.1,2,-1,.1,40,30)]
        for m in matrices:
            a,b,c,d,x,y=m;det=a*d-b*c
            inverse=(d/det,-b/det,(b*y-d*x)/det,-c/det,a/det,(c*x-a*y)/det)
            for alpha in (1,.61,.02):
                with self.subTest(matrix=m,alpha=alpha):
                    background=Image.frombytes('RGBA',(112,104),bytes(rng.randrange(256) for _ in range(112*104*4)))
                    expected=background.copy();actual=background.copy()
                    layer=image.transform(expected.size,Image.Transform.AFFINE,inverse,resample=Image.Resampling.BICUBIC)
                    if alpha<1:layer.putalpha(layer.getchannel('A').point(lambda v:round(v*alpha)))
                    expected.alpha_composite(layer)
                    composite_affine(actual,image,m,inverse,alpha)
                    self.assertEqual(actual.tobytes(),expected.tobytes())


if __name__=='__main__':
    unittest.main()
