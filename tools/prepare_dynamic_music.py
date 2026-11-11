"""Streamable original music layers, authored loop points and order boundaries."""
from pathlib import Path
import array
import json
import struct
import subprocess
from pc_runtime import hidden_process_options
from xm_layers import channel_layer, order_timeline, extend_render_cycle

# Native track IDs; the owned PS3 SetupVolumeForTune (0x23d3ac) separates
# main/file 1, percussion/file 2, night/file 3, and hi-hats/file 4.
GROUPS={1:(range(24),range(24,27),[27]),
        6:(range(18),range(18,29),[*range(18,25),29]),
        7:(range(16),range(16,23),[23]),
        8:(range(18),range(18,21),[21])}


def normalize_layers(layers):
    if len(layers)==3 and len({len(s) for s in layers})==1:
        peak=max((max(abs(a),abs(a+b),abs(a+c),abs(a+b+c)) for a,b,c in zip(*layers)),default=0)
    else:
        peak=sum(max(abs(min(s,default=0)),abs(max(s,default=0))) for s in layers)
    if not peak:return layers,0
    gain=5000/peak
    lookup=array.array('h',(max(-32768,min(32767,round(v*gain))) for v in [*range(32768),*range(-32768,0)]))
    return [array.array('h',map(lookup.__getitem__,s)) for s in layers],peak


def prepare_dynamic_track(imported,destination,temp,index,name,start):
    from prepare_music import module_section,encode_music
    import shutil
    ffmpeg=shutil.which('ffmpeg')
    source=(imported/'music/MAINMUSIC.XM').read_bytes()
    section,end=module_section(source,start);timeline=order_timeline(section)
    if len(timeline['orders'])>128:raise ValueError('Music timeline exceeds PSP metadata budget')
    if index==5:
        alternate,_=module_section((imported/'music/MAINMUSIC_DRUMS.XM').read_bytes(),0)
        alt_timeline=order_timeline(alternate)
        sources=[section,alternate];timelines=[timeline,alt_timeline]
    else:
        hats,_=module_section((imported/'music/MAINMUSIC_HIHATS.XM').read_bytes(),start)
        if order_timeline(hats)!=timeline:raise ValueError('Hi-hat sequencer differs from the main song')
        groups=GROUPS[index]
        sources=[channel_layer(section,groups[0]),channel_layer(section,groups[1]),channel_layer(hats,groups[2])]
        timelines=[timeline]*3
    buffers=[]
    for layer,(module,timing) in enumerate(zip(sources,timelines)):
        module_path=temp/f'{name}-{layer}.xm';pcm=temp/f'{name}-{layer}.pcm';module_path.write_bytes(extend_render_cycle(module))
        subprocess.run([ffmpeg,'-v','error','-i',str(module_path),'-t',str(timing['samples']/22050+.15),'-ac','1','-ar','22050','-f','s16le',str(pcm)],check=True,**hidden_process_options())
        raw=pcm.read_bytes()
        if len(raw)<timing['samples']*2:raise ValueError(f'Rendered {name} layer {layer}: {len(raw)//2} samples, expected {timing["samples"]}')
        buffers.append(array.array('h',raw[:timing['samples']*2]))
    buffers,combined_peak=normalize_layers(buffers);facts=[]
    for layer,samples in enumerate(buffers):
        suffix='' if layer==0 else '_drums' if layer==1 else '_hats'
        pcm=temp/f'{name}-{layer}.pcm';pcm.write_bytes(samples.tobytes());target=destination/f'music{index}{suffix}.wav'
        facts.append(dict(file=target.name,layer=layer,**encode_music(ffmpeg,pcm,target)))
    fact=dict(facts[0],name=name,start_order=start,end_order=end,seconds=len(buffers[0])/22050,
              normalized_peak=max((abs(s) for s in buffers[0]),default=0),layers=facts[1:])
    fact['dynamic']=dict(scheme=2 if index==5 else 1,**timeline,normalization_source_peak=combined_peak)
    if index==5:
        fact['dynamic']['night_samples']=alt_timeline['samples'];fact['dynamic']['night_loop']=alt_timeline['loop_start']
        fact['dynamic']['night_order1']=next(sample for sample,order in alt_timeline['orders'] if order==1)
    return fact


def write_dynamic_index(destination,facts):
    active=[(n,f) for n,f in enumerate(facts) if 'dynamic' in f]
    data=bytearray(b'PVAM'+struct.pack('<II',1,len(active)));checks=[]
    for index,fact in active:
        d=fact['dynamic'];orders=d['orders']
        data.extend(struct.pack('<8I',index,d['scheme'],d['samples'],d['loop_start'],len(orders),d.get('night_samples',0),d.get('night_loop',0),d.get('night_order1',0)))
        for sample,order in orders:data.extend(struct.pack('<II',sample,order))
        for layer in fact['layers']:checks.append((index,layer['layer'],layer['reference_samples'],layer['reference_fnv32']))
    (destination/'music-dynamics.bin').write_bytes(data)
    (destination/'music-layer-checks.bin').write_bytes(b'PVLC'+struct.pack('<I',len(checks))+b''.join(struct.pack('<4I',*c) for c in checks))
