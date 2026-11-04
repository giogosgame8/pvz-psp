"""Prepare private original soundtrack sections from the supplied module."""
from pathlib import Path
import array,json,struct,subprocess,tempfile,shutil,sys,hashlib,math
from pc_runtime import hidden_process_options

TRACKS=[('menu',0x98),('day',0),('seeds',0x7a),('bowling',0xa6),('conveyor',0xd4),
        ('night',0x30),('pool',0x5e),('fog',0x7d),('roof',0xb8),('puzzle',0xb1),('boss',0x9e),('zen',0xdd)]

def encode_music(ffmpeg,pcm,target):
    """4-bit mono IMA WAV; keep the exact source duration in its fact chunk."""
    raw=pcm.read_bytes();count=len(raw)//2
    subprocess.run([ffmpeg,'-v','error','-f','s16le','-ar','22050','-ac','1','-i',str(pcm),'-c:a','adpcm_ima_wav','-block_size','256','-bitexact',str(target)],check=True,**hidden_process_options())
    data=bytearray(target.read_bytes());at=12
    while at+8<=len(data):
        tag=data[at:at+4];size=struct.unpack_from('<I',data,at+4)[0]
        if tag==b'fact':struct.pack_into('<I',data,at+8,count)
        at+=8+size+(size&1)
    target.write_bytes(data)
    reference=subprocess.run([ffmpeg,'-v','error','-i',str(target),'-t','10','-f','s16le','-acodec','pcm_s16le','-'],check=True,stdout=subprocess.PIPE,**hidden_process_options()).stdout
    fingerprint=2166136261
    for byte in reference[:8192]:fingerprint=((fingerprint^byte)*16777619)&0xffffffff
    original=array.array('h',raw[:len(reference)]);decoded=array.array('h',reference)
    energy=sum(s*s for s in original);noise=sum((a-b)**2 for a,b in zip(original,decoded))
    return dict(codec='IMA ADPCM',sample_rate=22050,samples=count,bytes=len(data),pcm_bytes=len(raw),sha256=hashlib.sha256(data).hexdigest(),reference_samples=4096,reference_fnv32=fingerprint,snr_first_10s_db=round(10*math.log10(energy/max(1,noise)),2))

def write_music_index(destination,facts):
    (destination/'music-index.json').write_text(json.dumps(facts,indent=2))
    checks=b'PVMC'+struct.pack('<I',len(facts))+b''.join(struct.pack('<II',f['reference_samples'],f['reference_fnv32']) for f in facts)
    (destination/'music-checks.bin').write_bytes(checks)

def module_section(source,start):
    data=bytearray(source);length=struct.unpack_from('<H',data,64)[0];orders=list(data[80:80+length]);end=start+1
    while end<length and orders[end]!=199:end+=1
    selection=orders[start:end];struct.pack_into('<HH',data,64,len(selection),0);data[80:336]=bytes(selection)+bytes(256-len(selection))
    # Position-jump effects refer to order indices, so localize their targets
    # for a stand-alone section while preserving notes, instruments and tempo.
    offset=60+struct.unpack_from('<I',data,60)[0];channels=struct.unpack_from('<H',data,68)[0];patterns=struct.unpack_from('<H',data,70)[0]
    for pattern in range(patterns):
        header=struct.unpack_from('<I',data,offset)[0];rows,size=struct.unpack_from('<HH',data,offset+5);cursor=offset+header;limit=cursor+size
        if size:
            for event in range(rows*channels):
                first=data[cursor];cursor+=1;values=[0]*5;locations=[None]*5
                if first&128:
                    for column in range(5):
                        if first&(1<<column):locations[column]=cursor;values[column]=data[cursor];cursor+=1
                else:
                    values=[first,*data[cursor:cursor+4]];locations=[cursor-1,*range(cursor,cursor+4)];cursor+=4
                if values[3]==11 and locations[4] is not None:
                    target=values[4];data[locations[4]]=target-start if start<=target<end else 0
            if cursor!=limit:raise ValueError('Invalid XM pattern boundary')
        offset+=header+size
    return bytes(data),end

def prepare_music(imported,destination):
    ffmpeg=shutil.which('ffmpeg');source=(imported/'music/MAINMUSIC.XM').read_bytes();files=[];facts=[]
    with tempfile.TemporaryDirectory(prefix='pvz-private-music-') as temporary:
        temp=Path(temporary)
        for index,(name,start) in enumerate(TRACKS+[('credits',None)]):
            target=temp/f'music{index}.pcm'
            if start is None:input_path=imported/'music/ZOMBIESONYOURLAWN.OGG';end=None
            else:
                section,end=module_section(source,start);input_path=temp/(name+'.xm');input_path.write_bytes(section)
            subprocess.run([ffmpeg,'-v','error','-i',str(input_path),'-t','180','-ac','1','-ar','22050','-f','s16le',str(target)],check=True,**hidden_process_options())
            samples=array.array('h',target.read_bytes());peak=max((abs(s) for s in samples),default=0)
            if peak:
                gain=5000/peak;samples=array.array('h',(max(-32768,min(32767,round(s*gain))) for s in samples));target.write_bytes(samples.tobytes())
            compressed=destination/f'music{index}.wav';info=encode_music(ffmpeg,target,compressed)
            files.append(compressed.name);facts.append(dict(file=compressed.name,name=name,start_order=start,end_order=end,seconds=len(samples)/22050,normalized_peak=max((abs(s) for s in samples),default=0),**info))
            print(f'Prepared original music: {name}, {len(samples)/22050:.1f}s',flush=True)
    write_music_index(destination,facts)
    files.extend(['music-index.json','music-checks.bin']);return files
