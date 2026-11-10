"""Metadata and integrity checks for private, locally prepared PSP resources."""
import hashlib
import json
from pathlib import Path
import struct
import zlib

DATA_VERSION = '0.21.0'
RECIPE = 'ps3-0210-2'


def file_hash(path):
    digest=hashlib.sha256()
    with Path(path).open('rb') as source:
        for block in iter(lambda:source.read(1048576),b''):digest.update(block)
    return digest.hexdigest()


def write_catalog(folder):
    records=[]
    for path in sorted(folder.glob('*.pva')):
        with path.open('rb') as source:
            header=source.read(40)
            if len(header)!=40 or header[:4] not in (b'PVA3',b'PVA4'):raise ValueError('Invalid animation metadata: '+path.name)
            frames=struct.unpack_from('<I',header,20)[0]
            if not 1<=frames<=256:raise ValueError('Invalid animation frame count')
            index=source.read(frames*8)
            if len(index)!=frames*8:raise ValueError('Incomplete animation index')
        name=path.name.encode('ascii')
        if len(name)>=64:raise ValueError('Animation name is too long')
        metadata=header+index
        records.append(struct.pack('<64sII',name,path.stat().st_size,len(metadata))+metadata)
    payload=b''.join(records)
    (folder/'assets.idx').write_bytes(struct.pack('<4s4I',b'PVAI',1,len(records),len(payload),zlib.crc32(payload))+payload)


def write_launcher_images(folder):
    from PIL import Image,ImageDraw,ImageFont
    from prepare_features09 import first_prepared_frame
    cover=first_prepared_frame(folder/'menu_main.pva').convert('RGB')
    cover.save(folder/'XMB_PIC1.PNG',optimize=True)
    icon=Image.new('RGB',(144,80),(30,52,29))
    logo=cover.crop((68,0,353,126)).resize((144,64),Image.Resampling.LANCZOS)
    icon.paste(logo,(0,0));draw=ImageDraw.Draw(icon)
    draw.text((72,63),'PSP',font=ImageFont.load_default(13),anchor='mt',fill=(243,235,180))
    icon.save(folder/'XMB_ICON0.PNG',optimize=True)


def write_integrity(folder):
    manifest=json.loads((folder/'manifest.json').read_text(encoding='utf-8'))
    entries={p.name:{'bytes':p.stat().st_size,'sha256':file_hash(p)} for p in sorted(folder.iterdir())
             if p.is_file() and p.name not in ('integrity.json','ending-progress.json')}
    report={'recipe':RECIPE,'source_iso_sha256':manifest['source_iso_sha256'],'files':entries}
    (folder/'integrity.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    return report


def verify_integrity(folder,source_sha=None):
    report=json.loads((folder/'integrity.json').read_text(encoding='utf-8'))
    if report.get('recipe')!=RECIPE or (source_sha is not None and report.get('source_iso_sha256')!=source_sha):
        raise ValueError('Prepared data comes from a different source or setup version.')
    if not report.get('files') or 'manifest.json' not in report['files']:raise ValueError('Incomplete integrity index')
    for name,expected in report['files'].items():
        path=Path(name)
        if path.name!=name or name in ('.','..') or path.is_absolute():raise ValueError('Invalid integrity path')
        path=folder/name
        if not path.is_file() or path.stat().st_size!=expected['bytes'] or file_hash(path)!=expected['sha256']:
            raise ValueError('Prepared file failed verification: '+name)
    manifest=json.loads((folder/'manifest.json').read_text(encoding='utf-8'))
    if manifest.get('source_iso_sha256')!=report.get('source_iso_sha256'):raise ValueError('Prepared source identity does not match')
    required={a['file'] for a in manifest['assets']}|{'assets.idx','menu-motion.bin','menu-motion.json','shoot-alignment.bin','foley.pvf','ending.pvj','ending.json','XMB_ICON0.PNG','XMB_PIC1.PNG','zombie_motion.bin','seeds.bin','disc_font.bin','strings.bin','almanac.bin','story-index.json','music-index.json','music-checks.bin'}
    required|={song['file'] for song in json.loads((folder/'music-index.json').read_text(encoding='utf-8'))}
    if not required.issubset(report['files']):raise ValueError('Integrity index omits required game files')
    return report
