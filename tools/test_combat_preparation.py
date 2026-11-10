"""Changing pose anchors must not change compressed pixels or detach avatar parts."""
import hashlib
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch
import prepare_assets
from prepare_garden018 import prepare_garden018
from prepare_combat0190 import align_zombie_clips


class CombatPreparationTests(unittest.TestCase):
    def test_clips_share_original_canvas_point_without_changing_payloads(self):
        with tempfile.TemporaryDirectory() as root:
            root=Path(root)
            geometry={'zombie_walk':((80,60,180,260),(50,100)),
                      'zombie_eat':((60,55,200,270),(70,108)),
                      'avatar_body_walk':((80,60,180,260),(50,100)),
                      'avatar_body_eat':((60,55,200,270),(70,108)),
                      'zombie_dance':((55,65,205,260),(75,98))}
            names=list(geometry)+['avatar_face_walk','avatar_face_eat','avatar_face_dance','avatar_mustache_dance']
            originals={}
            for name in names:
                originals[name]=b'header'.ljust(32,b'\0')+struct.pack('<2f',1,1)+b'unchanged compressed frames'
                (root/(name+'.pva')).write_bytes(originals[name])
            specs=[(name,'Zombie','anim_eat' if name.endswith('eat') else 'anim_walk',.48,'') for name in geometry]
            assets=[{'file':name+'.pva','sha256':'old'} for name in names]
            result=align_zombie_clips(root,specs,geometry,assets)
            self.assertEqual(result['aligned'],9)
            for name,(box,size) in geometry.items():
                raw=(root/(name+'.pva')).read_bytes();ax,ay=struct.unpack_from('<2f',raw,32)
                self.assertAlmostEqual(box[0]+ax*(box[2]-box[0])/size[0],130,places=4)
                self.assertAlmostEqual(box[1]+ay*(box[3]-box[1])/size[1],260,places=4)
            for action in ('walk','eat'):
                self.assertEqual((root/('avatar_body_'+action+'.pva')).read_bytes()[32:40],(root/('avatar_face_'+action+'.pva')).read_bytes()[32:40])
            for part in ('face','mustache'):
                self.assertEqual((root/'zombie_dance.pva').read_bytes()[32:40],(root/('avatar_'+part+'_dance.pva')).read_bytes()[32:40])
            for asset in assets:
                name=asset['file'][:-4];raw=(root/asset['file']).read_bytes()
                self.assertEqual(raw[:32],originals[name][:32]);self.assertEqual(raw[40:],originals[name][40:])
                self.assertEqual(asset['sha256'],hashlib.sha256(raw).hexdigest())
            self.assertEqual(align_zombie_clips(root,specs,geometry,assets)['aligned'],0)

    def test_garden_failure_restores_main_animation_specs(self):
        original=prepare_assets.SPECS
        with patch('prepare_assets.prepare',side_effect=OSError('disk full')):
            with self.assertRaisesRegex(OSError,'disk full'):
                prepare_garden018(Path('unused-import'),Path('unused-output'))
        self.assertIs(prepare_assets.SPECS,original)


if __name__=='__main__':unittest.main()
