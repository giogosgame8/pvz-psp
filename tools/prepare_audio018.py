"""Pack every owned-disc sample into bounded, seekable mono IMA ADPCM data."""
from pathlib import Path
import argparse,hashlib,json,re,struct,subprocess,tempfile,xml.etree.ElementTree as ET

ROOT=Path(__file__).resolve().parents[1]
REFERENCE=ROOT/'tools/reference/FOLEY-REFERENCE-0.18.json'
if not REFERENCE.is_file():
    REFERENCE=ROOT/'FOLEY-REFERENCE-0.18.json'  # Earlier release ZIP layout.
def normalize_sound_names(sounds):
    """The bundled resource IDs omit the SOUND_ prefix used by Foley groups."""
    normalized={}
    for name,path in sounds.items():
        key=name if name.startswith('SOUND_') else 'SOUND_'+name
        if key in normalized and normalized[key]!=path:
            raise ValueError(f'Conflicting sound declarations for {key}')
        normalized[key]=path
    return normalized

def read_facts(elf,manifest):
    raw=Path(elf).read_bytes();names={};delta=0x29e30;at=0x833c7c
    def name_at(pos):
        offset=struct.unpack_from('>I',raw,pos)[0]-0x10000
        if not 0<=offset<len(raw) or raw[offset:offset+6]!=b'SOUND_':return None
        return raw[offset:raw.index(b'\0',offset)].decode('ascii')
    while name_at(at-4):at-=4
    while (name:=name_at(at)):
        names[struct.unpack_from('>I',raw,at-delta)[0]]=name;at+=4
    # Enum labels are factual names; the variants, weighting, pitch and flags
    # are independently read from the owner's supported PS3 executable.
    labels=json.loads(REFERENCE.read_text())['group_names']
    groups=[]
    for i in range(104):
        row=struct.unpack_from('>If10II',raw,0x8538c4+i*52)
        if row[0]!=i:raise ValueError('Unsupported PS3 Foley table')
        groups.append(dict(id=i,name=labels[i],pitch=row[1],flags=row[-1],variants=[names[p] for p in row[2:12] if p]))
    sound_tags=re.findall(r'<Sound\b[^>]*/>',Path(manifest).read_text(encoding='utf-8-sig'))
    sounds={}
    for tag in sound_tags:
        # Some original Sound tags contain non-XML pitch/comment tokens.
        name=re.search(r'\bid="([^"]+)"',tag);path=re.search(r'\bpath="([^"]+)"',tag)
        if name and path:sounds['SOUND_'+name[1]]=path[1]
    if len(sounds)!=194:raise ValueError('Unsupported sound resource manifest')
    return groups,sounds

def prepare(imported,dest,reference_elf=None):
    manifest=imported/'archives/compressed/properties/resources.xml'
    if reference_elf:groups,sounds=read_facts(reference_elf,manifest)
    else:
        facts=json.loads(REFERENCE.read_text());groups,sounds=facts['groups'],facts['sounds']
    sounds=normalize_sound_names(sounds)
    source={p.stem.lower():p for p in (imported/'archives/sounds/sounds').glob('*.mpg')}
    filenames=sorted(source);assert len(filenames)==194
    ids={name:i for i,name in enumerate(filenames)}
    primary=[next((k for k,v in sounds.items() if v.lower()==name),'SOUND_'+name.upper()) for name in filenames]
    key_ids={k:ids.get(v.lower(),-1) for k,v in sounds.items()}
    missing={k:v for k,v in sounds.items() if v.lower() not in ids}
    enum=['/* Sample IDs and factual PS3 Foley groups; no audio embedded. */','#ifndef PVZ_FOLEY_DEFS_H','#define PVZ_FOLEY_DEFS_H','enum { '+','.join('FX_'+k[6:].upper() for k in primary)+', FX_SAMPLE_COUNT };','enum { '+','.join('FX_'+k[6:].upper()+'='+str(v) for k,v in key_ids.items() if k not in primary)+' };','enum { '+','.join(g['name'] for g in groups)+', FOLEY_COUNT };','typedef struct {float pitch;unsigned char flags,count;short samples[10];} FoleyDefinition;','static const FoleyDefinition foley_definitions[FOLEY_COUNT]={']
    for g in groups:
        variants=[key_ids[k] for k in g['variants']];assert all(i>=0 for i in variants);enum.append('{'+str(g['pitch'])+'f,'+str(g['flags'])+','+str(len(variants))+',{'+','.join(map(str,variants))+'}},')
    enum+=['};','#endif'];(ROOT/'src/foley_defs.h').write_text('\n'.join(enum)+'\n')
    entries=[];payload=[];facts=[]
    with tempfile.TemporaryDirectory(prefix='pvz-foley-') as temp:
        target=Path(temp)/'sound.wav';pcm=Path(temp)/'sound.pcm'
        for i,name in enumerate(filenames):
            key=primary[i];source_file=source[name]
            subprocess.run(['ffmpeg','-v','error','-y','-i',str(source_file),'-ac','1','-ar','22050','-c:a','adpcm_ima_wav','-block_size','256','-bitexact',str(target),'-map','0:a:0','-ac','1','-ar','22050','-f','s16le',str(pcm)],check=True)
            data=target.read_bytes();at=12;samples=None;blocks=None
            while at+8<=len(data):
                tag,size=struct.unpack_from('<4sI',data,at);chunk=data[at+8:at+8+size]
                if tag==b'fact':samples=struct.unpack_from('<I',chunk)[0]
                if tag==b'data':blocks=chunk
                at+=8+size+(size&1)
            samples=pcm.stat().st_size//2
            assert samples and blocks and len(blocks)%256==0 and (len(blocks)//256-1)*505<samples<=len(blocks)//256*505
            payload.append(blocks);facts.append(dict(id=i,key=key,file=source_file.name,samples=samples,seconds=round(samples/22050,6),bytes=len(blocks),sha256=hashlib.sha256(blocks).hexdigest()))
            print(f'Prepared {i+1}/194 {source_file.name}',flush=True)
    offset=16+len(filenames)*16
    with (dest/'foley.pvf').open('wb') as f:
        f.write(struct.pack('<4sIII',b'PVFX',1,len(filenames),22050))
        for data,record in zip(payload,facts):
            import zlib
            f.write(struct.pack('<4I',offset,len(data),record['samples'],zlib.crc32(data)));offset+=len(data)
        for data in payload:f.write(data)
    result={'source':'PS3 Foley table: file offset 0x8538c4; 104 records, 52 bytes each','codec':'mono 22050 Hz IMA ADPCM, 256-byte blocks / 505 samples','groups':groups,'sounds':sounds,'unavailable_manifest_declarations':missing,'samples':facts,'bank_bytes':(dest/'foley.pvf').stat().st_size,'bank_sha256':hashlib.sha256((dest/'foley.pvf').read_bytes()).hexdigest()}
    (dest/'foley-index.json').write_text(json.dumps(result,indent=2));return result

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--imported',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--reference-elf',type=Path);a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True);prepare(a.imported,a.output,a.reference_elf)
