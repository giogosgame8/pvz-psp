"""Regression coverage for the separately animated mouth in all face styles."""
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch
from PIL import Image
from prepare_customizer import prepare_customizer


class CustomizerPreparationTests(unittest.TestCase):
    def test_every_face_contains_independently_moving_jaw_in_walk_eat_and_dance(self):
        def key(x=20,y=20):return dict(x=x,y=y,kx=0,ky=0,sx=1,sy=1,a=1,f=0)
        tracks=[(name,[key(),key()]) for name in ('anim_head1','anim_hair','anim_cone','anim_bucket','Zombie_mustache')]
        tracks.append(('anim_head2',[key(20,30),key(28,30)]))
        with tempfile.TemporaryDirectory() as root:
            root=Path(root);images=root/'import/archives/main/reanim';images.mkdir(parents=True)
            (images/'zombie.reanim').write_text('<stub/>')
            names=['Zombie_head','Zombie_hair','Zombie_jaw']
            names += ['Zombie_head_sunglasses'+str(i) for i in range(1,5)]
            names += [part+str(i) for part in ('Zombie_cone','Zombie_bucket','Zombie_mustache') for i in range(1,4)]
            names += ['Zombie_football_helmet'+('' if i==1 else str(i)) for i in range(1,4)]
            for name in names:Image.new('RGBA',(4,4),(255,0,255,255) if name=='Zombie_jaw' else (0,255,0,255)).save(images/(name.lower()+'.ptx'),format='PNG')
            destination=root/'out';destination.mkdir();geometry={};captured={}
            for base in ('avatar_body_walk','avatar_body_eat','zombie_dance'):
                (destination/(base+'.pva')).write_bytes(bytes(32)+struct.pack('<2f',30,60))
                geometry[base]=((90,90,150,150),(60,60))
            def write(path,frames,fps,anchor):captured[path.stem]=frames;return {'file':path.name}
            with patch('prepare_assets.frames_from_xml',return_value=(12,tracks)),patch('prepare_assets.clip_range',return_value=(0,2)):
                prepare_customizer(root/'import',destination,None,write,geometry)
            self.assertNotIn('avatar_helmet_dance',captured)
            self.assertEqual(len(captured['avatar_mustache_dance']),6)
            for action in ('walk','eat','dance'):
                frames=captured['avatar_face_'+action];self.assertEqual(len(frames),10)
                for variant in range(5):
                    self.assertEqual(frames[variant*2].getpixel((27,37)),(255,0,255,255))
                    self.assertEqual(frames[variant*2+1].getpixel((35,37)),(255,0,255,255))
                    self.assertEqual(frames[variant*2+1].getpixel((27,37))[3],0)


if __name__=='__main__':unittest.main()
