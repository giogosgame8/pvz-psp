"""Regression checks for portrait pixels and nested ending animation clocks."""
from pathlib import Path
import tempfile
import unittest
import random
import struct
from PIL import Image

from prepare_assets import write_pva
from prepare_features09 import first_prepared_frame
from reanim_scene import Scene
from texture_layout import pack_rgba4444,unpack_rgba4444


class PresentationPreparationTests(unittest.TestCase):
    def test_compiled_decoding_roundtrips_every_16_bit_pixel_value(self):
        raw=b''.join(struct.pack('<H',value) for value in range(65536))
        image=unpack_rgba4444(raw,256,256)
        self.assertEqual(pack_rgba4444(image),raw)
        self.assertEqual(image.getpixel((52,18)),(68,51,34,17))

    def test_fast_texture_packing_matches_reference_for_all_channel_values(self):
        rng=random.Random(185)
        pixels=[tuple((v if channel==fixed else rng.randrange(256)) for channel in range(4))
                for fixed in range(4) for v in range(256)]
        image=Image.new('RGBA',(32,32));image.putdata(pixels)
        expected=b''.join(struct.pack('<H',(r>>4)|((g>>4)<<4)|((b>>4)<<8)|((a>>4)<<12)) for r,g,b,a in pixels)
        self.assertEqual(pack_rgba4444(image),expected)

    def test_packet_portrait_reads_tiled_psp_texture_in_correct_pixel_order(self):
        image=Image.new('RGBA',(29,35))
        for y in range(image.height):
            for x in range(image.width):image.putpixel((x,y),((x%16)*17,(y%16)*17,((x+y)%16)*17,255))
        with tempfile.TemporaryDirectory() as folder:
            file=Path(folder)/'portrait.pva'
            write_pva(file,[image],1)
            self.assertEqual(first_prepared_frame(file).tobytes(),image.tobytes())

    def test_static_nested_scene_does_not_restart_dancing_children(self):
        def frame(**values):
            result=dict(x=0,y=0,kx=0,ky=0,sx=1,sy=1,a=1,f=0,i='',text='',font='',born=0)
            result.update(values);return result
        class Example(Scene):
            def __init__(self):self.positions=[]
            def animation(self,name):
                if name=='root':return 7,[('actor',[frame(text='attacher__static[7]')]*8)]
                if name=='static':return 7,[('dancer',[frame(text='attacher__dance[21]')]*2)]
                return 21,[('body',[frame(x=i,i='body') for i in range(24)])]
            def texture(self,name):return Image.new('RGBA',(1,1),(255,255,255,255))
            def paint(self,canvas,image,m,alpha):self.positions.append(m[4])
        scene=Example();canvas=Image.new('RGBA',(40,40))
        scene.draw(canvas,'root',.9);before=scene.positions[-1]
        scene.draw(canvas,'root',1.1);after=scene.positions[-1]
        self.assertGreater(after,before)
        self.assertAlmostEqual(after-before,.6)


if __name__=='__main__':unittest.main()
