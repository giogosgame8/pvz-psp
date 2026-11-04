"""Adding launcher artwork must retain the game executable and existing audio."""
from pathlib import Path
import struct
import tempfile
import unittest
from PIL import Image
from launcher_media import read_pbp,repack_images


class LauncherPictureTests(unittest.TestCase):
    def test_repacking_pictures_preserves_game_sections_and_sound(self):
        with tempfile.TemporaryDirectory() as temporary:
            folder=Path(temporary);file=folder/'EBOOT.PBP'
            entries=[b'sfo',b'',b'',b'',b'',b'owned audio',b'game executable',b'extra payload']
            offsets=[];at=40
            for entry in entries:offsets.append(at);at+=len(entry)
            file.write_bytes(b'\0PBP'+struct.pack('<I8I',0x10000,*offsets)+b''.join(entries))
            icon=folder/'icon.png';cover=folder/'cover.png'
            Image.new('RGB',(144,80),(50,120,40)).save(icon)
            Image.new('RGB',(480,272),(10,60,90)).save(cover)
            repack_images(file,file,icon,cover)
            _,after=read_pbp(file)
            self.assertEqual(after[1],icon.read_bytes());self.assertEqual(after[4],cover.read_bytes())
            for index in (0,2,3,5,6,7):self.assertEqual(after[index],entries[index])


if __name__=='__main__':unittest.main()
