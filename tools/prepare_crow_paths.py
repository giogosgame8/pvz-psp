"""Project the disc's crow motion onto the PSP's three menu pages."""
import argparse
from pathlib import Path
import struct
import zlib


PERCHES = ((75, 125), (235, 114), (112, 128))
OFFSETS = (1360, 0, 2560)
CLIPS = (
    ('anim_enter', -1, 0), ('anim_enter_right', -1, 2),
    ('anim_center_left', 0, 1), ('anim_left_center', 1, 0),
    ('anim_center_right', 0, 2), ('anim_right_center', 2, 0),
)


def prepare_crow_paths(imported, output):
    from prepare_assets import frames_from_xml, clip_range
    relative = 'reanim/mainmenu3/mainmenu.reanim'
    candidates = [imported/'archives'/folder/relative for folder in ('compressed', 'main', 'en')]
    source = next((path for path in candidates if path.is_file()), None)
    if source is None:
        raise ValueError('The disc menu animation is missing')
    fps, tracks = frames_from_xml(source)
    crow = next(frames for name, frames in tracks if name.lower() == 'crow2')
    payload = bytearray()
    for clip, origin, destination in CLIPS:
        start, stop = clip_range(tracks, clip)
        frames = crow[start:stop]
        if not 2 <= len(frames) <= 24 or any(frame['f']<0 or not frame['i'] for frame in frames):
            raise ValueError('Unexpected crow path: '+clip)
        first, last = frames[0], frames[-1]
        source_start = PERCHES[origin] if origin >= 0 else (
            (first['x']-OFFSETS[destination])*.375, first['y']*272/720)
        source_end = PERCHES[destination]
        landing = next((index for index, frame in enumerate(frames)
                        if 'land' in frame['i'].lower()), len(frames)-1)
        payload.extend(struct.pack('<4B', len(frames), landing, int(last['x']>first['x']), 0))
        for index, frame in enumerate(frames):
            progress = index/(len(frames)-1)
            world_x = first['x']+(last['x']-first['x'])*progress
            world_y = first['y']+(last['y']-first['y'])*progress
            x = source_start[0]+(source_end[0]-source_start[0])*progress+(frame['x']-world_x)*.375
            y = source_start[1]+(source_end[1]-source_start[1])*progress+(frame['y']-world_y)*272/720
            payload.extend(struct.pack('<2h', round(16*x), round(16*y)))
    output.write_bytes(struct.pack('<4s3I', b'PVCR', 1, round(fps), zlib.crc32(payload))+payload)


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--imported', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args=parser.parse_args()
    prepare_crow_paths(args.imported,args.output)
