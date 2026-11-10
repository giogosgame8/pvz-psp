"""Unlock-dependent menu art must stay separable from the animated background."""
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch
import zlib
from PIL import Image
from prepare_assets import write_pva
from prepare_menu_scene020 import prepare_menu_scene,extras_group


class MenuSceneTests(unittest.TestCase):
    def test_export_preserves_layer_order_and_separate_unlock_groups(self):
        def key(x,name):return dict(x=x,y=200,kx=0,ky=0,sx=1,sy=1,a=1,f=0,i='IMAGE_REANIM_LAYER_'+name)
        definitions=[('background',1360,'background'),('ambient grass',1600,'grass'),('foreground',1600,'front'),('shop',3000,'shop'),('almanac pot',3150,'book'),('zengarden',3350,'garden'),('mail',2900,'mail')]
        tracks=[]
        with tempfile.TemporaryDirectory() as temporary:
            root=Path(temporary);imported=root/'import';destination=root/'data';destination.mkdir()
            for archive in ('main','compressed','en'):(imported/'archives'/archive).mkdir(parents=True)
            folder=imported/'archives/main/reanim/mainmenu3';folder.mkdir(parents=True)
            (folder/'mainmenu.reanim').write_text('<stub/>')
            for name,x,image in definitions:
                Image.new('RGBA',(32,32),(40,160,70,255)).save(folder/(image+'.ptx'),format='PNG')
                a=key(x,image);b=key(x+20 if name=='ambient grass' else x,image);tracks.append((name,[a,b]))
            with patch('prepare_assets.frames_from_xml',return_value=(12,tracks)),patch('prepare_assets.clip_range',return_value=(0,2)):
                assets=prepare_menu_scene(imported,destination,write_pva)
            raw=(destination/'menu-motion.bin').read_bytes();magic,version,count,crc=struct.unpack_from('<4s3I',raw)
            self.assertEqual((magic,version,count),(b'PVMU',1,len(assets)));self.assertEqual(zlib.crc32(raw[16:]),crc)
            details=json.loads((destination/'menu-motion.json').read_text())['layers']
            home=[d for d in details if d['page']==0]
            self.assertEqual([d['track'].split(' / ')[0] for d in home],['background','ambient grass','foreground'])
            self.assertEqual(home[1]['keys'],2)
            extras=[d for d in details if d['page']==2]
            for group,name in enumerate(('shop','almanac pot','zengarden','mail'),1):
                self.assertTrue(any(d['group']==group and name in d['track'] for d in extras))
            self.assertEqual(extras_group('Back ball button'),0)
            self.assertEqual(extras_group('Almanac plant 10'),2)
            self.assertEqual([extras_group(n) for n in ('packets','bonsai','envelopes')],[2,3,4])


if __name__=='__main__':unittest.main()
