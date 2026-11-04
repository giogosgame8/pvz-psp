"""Validate and embed PSP XMB artwork/audio without changing game code."""
from pathlib import Path
import argparse
import hashlib
import io
import json
import struct
import tempfile
from PIL import Image

NAMES = ('PARAM.SFO', 'ICON0.PNG', 'ICON1.PMF', 'PIC0.PNG', 'PIC1.PNG',
         'SND0.AT3', 'DATA.PSP', 'DATA.PSAR')

def sha(data):
    return hashlib.sha256(data).hexdigest()


def repack_images(source, destination, icon, background):
    """Add owned-disc pictures during setup while retaining executable/audio."""
    version,entries=read_pbp(source)
    original=entries[6:]
    for index,path,size in ((1,icon,(144,80)),(4,background,(480,272))):
        data=Path(path).read_bytes()
        with Image.open(io.BytesIO(data)) as image:
            if image.format!='PNG' or image.size!=size or image.mode!='RGB':raise ValueError('Invalid PSP launcher picture')
            image.load()
        entries[index]=data
    offsets=[];offset=40
    for entry in entries:offsets.append(offset);offset+=len(entry)
    packed=b'\0PBP'+version+struct.pack('<8I',*offsets)+b''.join(entries)
    target=Path(destination)
    with tempfile.NamedTemporaryFile(dir=target.parent,prefix=target.name+'.',suffix='.tmp',delete=False) as temporary:
        temporary.write(packed);path=Path(temporary.name)
    if read_pbp(path)[1][6:]!=original:
        path.unlink();raise ValueError('Launcher preparation changed game code')
    path.replace(target)

def read_pbp(path):
    data = Path(path).read_bytes()
    if len(data) < 40 or data[:4] != b'\0PBP':
        raise ValueError('Not a PBP file')
    offsets = [*struct.unpack_from('<8I', data, 8), len(data)]
    if offsets[0] != 40 or offsets != sorted(offsets):
        raise ValueError('Invalid PBP section offsets')
    return data[4:8], [data[offsets[i]:offsets[i+1]] for i in range(8)]

def set_title(sfo, title):
    out = bytearray(sfo)
    magic, version, keys, values, count = struct.unpack_from('<4s4I', out)
    if magic != b'\0PSF':
        raise ValueError('Invalid PARAM.SFO')
    for i in range(count):
        at = 20 + i * 16
        key, kind, size, capacity, offset = struct.unpack_from('<HH3I', out, at)
        name = bytes(out[keys+key:out.index(0, keys+key)]).decode('utf-8')
        if name == 'TITLE':
            encoded = title.encode('utf-8') + b'\0'
            if kind != 0x0204 or len(encoded) > capacity:
                raise ValueError('Title does not fit the existing SFO field')
            out[values+offset:values+offset+capacity] = encoded.ljust(capacity, b'\0')
            struct.pack_into('<I', out, at+4, len(encoded))
            return bytes(out)
    raise ValueError('SFO has no TITLE field')

def chunks(data):
    if data[:4] != b'RIFF' or data[8:12] != b'WAVE' or struct.unpack_from('<I', data, 4)[0] != len(data)-8:
        raise ValueError('SND0 must be a complete RIFF/WAVE file')
    result = {}; at = 12
    while at + 8 <= len(data):
        name = data[at:at+4]; size = struct.unpack_from('<I', data, at+4)[0]
        if at+8+size > len(data) or name in result:
            raise ValueError('Invalid or duplicate RIFF chunk')
        result[name] = data[at+8:at+8+size]
        at += 8+size+(size & 1)
    if at != len(data):
        raise ValueError('Trailing incomplete RIFF data')
    return result

def validate_media(media):
    facts = {}
    for name, size in [('ICON0.PNG', (144, 80)), ('PIC1.PNG', (480, 272))]:
        raw = (Path(media)/name).read_bytes()
        with Image.open(io.BytesIO(raw)) as im:
            if im.format != 'PNG' or im.size != size or im.mode != 'RGB':
                raise ValueError(f'{name} must be a {size} RGB PNG')
            im.load()
        facts[name] = dict(size=list(size), bytes=len(raw), sha256=sha(raw))
    sound = (Path(media)/'SND0.AT3').read_bytes(); wave = chunks(sound)
    if set(wave) != {b'fmt ', b'fact', b'smpl', b'data'} or len(sound) > 500000:
        raise ValueError('Unsupported XMB audio chunks or size')
    tag, channels, rate, byte_rate, align, bits = struct.unpack_from('<HHIIHH', wave[b'fmt '])
    if (tag, channels, rate, align, bits) != (0x270, 2, 44100, 192, 0) or byte_rate != align*rate//1024:
        raise ValueError('SND0 must be 44.1 kHz stereo ATRAC3 LP4')
    samples, delay = struct.unpack('<2I', wave[b'fact'])
    loop = struct.unpack_from('<6I', wave[b'smpl'], 36)
    audio = wave[b'data']; frames = len(audio)//align
    if len(audio) % align or samples <= 0 or (len(wave[b'smpl']) != 60):
        raise ValueError('Incomplete ATRAC frames or loop')
    if loop != (0, 0, delay, delay+samples-1, 0, 0) or samples+delay+368 > frames*1024:
        raise ValueError('Invalid loop or claimed sample count')
    if samples/rate > 55 or frames*1024/rate > 55:
        raise ValueError('SND0 exceeds the XMB duration limit')
    # The encoder's strict inspection also checks the reversed side unit.
    if any((audio[n] >> 2) != 0x28 or (audio[n] & 3) > 2 for n in range(0, len(audio), align)):
        raise ValueError('ATRAC frame does not use the supported three-band profile')
    facts['SND0.AT3'] = dict(codec='ATRAC3 LP4', rate=rate, channels=channels,
        seconds=samples/rate, loop_start=delay, loop_end=delay+samples-1,
        frames=frames, bytes=len(sound), sha256=sha(sound))
    return facts

def repack(source, destination, media, title='Plants vs. Zombies'):
    facts = validate_media(media)
    original_pbp_hash = sha(Path(source).read_bytes())
    version, original = read_pbp(source)
    entries = list(original); entries[0] = set_title(entries[0], title)
    for index in (1, 4, 5):
        entries[index] = (Path(media)/NAMES[index]).read_bytes()
    offsets = []; offset = 40
    for item in entries:
        offsets.append(offset); offset += len(item)
    packed = b'\0PBP'+version+struct.pack('<8I', *offsets)+b''.join(entries)
    target = Path(destination); target.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(dir=target.parent, prefix=target.name+'.', suffix='.tmp', delete=False) as temporary:
        temporary.write(packed)
        temporary_path = Path(temporary.name)
    temporary_path.replace(target)
    _, result = read_pbp(target)
    if result[6:] != original[6:]:
        raise AssertionError('Game payload changed while adding launcher media')
    return dict(title=title, source_pbp_sha256=original_pbp_hash,
        pbp_sha256=sha(packed), pbp_bytes=len(packed), game_code_identical=True,
        game_payload_sha256=sha(original[6]), media=facts)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    parser.add_argument('destination', type=Path)
    parser.add_argument('--media', type=Path, required=True)
    parser.add_argument('--title', default='Plants vs. Zombies')
    args = parser.parse_args()
    print(json.dumps(repack(args.source, args.destination, args.media, args.title), indent=2))

if __name__ == '__main__':
    main()
