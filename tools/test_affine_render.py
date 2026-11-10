"""The optimized compositor must preserve the original full-canvas pixels."""
import math
import random
import unittest
from PIL import Image
from unittest.mock import patch

from affine_render import AffineLayerCache,affine_layer,composite_affine,render_frames


class AffineRenderTests(unittest.TestCase):
    def test_reused_layers_preserve_each_destination_background(self):
        image=Image.new('RGBA',(11,9),(40,160,70,127));matrix=(1,0,0,1,4,6);inverse=(1,0,-4,0,1,-6)
        cache=AffineLayerCache()
        with patch('affine_render.affine_layer',wraps=affine_layer) as transform:
            for color in ((200,30,70,255),(20,40,220,150)):
                expected=Image.new('RGBA',(32,32),color);actual=expected.copy()
                layer=image.transform(expected.size,Image.Transform.AFFINE,inverse,resample=Image.Resampling.BICUBIC)
                expected.alpha_composite(layer)
                composite_affine(actual,image,matrix,inverse,1,cache)
                self.assertEqual(actual.tobytes(),expected.tobytes())
            self.assertEqual(transform.call_count,1)

    def test_parallel_reuse_and_eviction_preserve_frame_order_and_pixels(self):
        image=Image.new('RGBA',(11,9),(70,130,240,170));cache=AffineLayerCache(2048)
        def draw(n,cached):
            x=(n%8)*3;alpha=(n%3+1)/3;canvas=Image.new('RGBA',(40,30),(n*3,20,80,255))
            composite_affine(canvas,image,(1,0,0,1,x,4),(1,0,-x,0,1,-4),alpha,cache if cached else None)
            return canvas.tobytes()
        expected=[draw(n,False) for n in range(64)]
        with patch('affine_render.worker_count',return_value=4):actual=render_frames(lambda n:draw(n,True),range(64))
        self.assertEqual(actual,expected);self.assertLessEqual(cache.bytes,2048)
        cache.clear();self.assertEqual(cache.bytes,0)

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
