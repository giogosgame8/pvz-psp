"""Keep every clip in a zombie family on the same original canvas origin."""
import hashlib
import struct


def align_zombie_clips(destination,specs,geometry,assets):
    families={};names={}
    for name,source,marker,scale,gear in specs:
        if source.startswith('Zombie') and source!='Zombie_charred' and name in geometry:
            names[name]=source
            preferred=name in ('zombie_walk','pole_walk') or name.endswith('_walk') and not isinstance(gear,dict)
            if source not in families or preferred and families[source].endswith(('_run','_death')):families[source]=name
    for source in families:
        candidates=[name for name in names if names[name]==source and name.endswith('_walk') and 'damage' not in name and not name.startswith(('avatar_','bot_'))]
        if source=='Zombie':families[source]='zombie_walk'
        elif source=='Zombie_polevaulter':families[source]='pole_walk'
        elif candidates:families[source]=candidates[0]
    changed=[]
    def patch(name,anchor):
        path=destination/(name+'.pva');raw=bytearray(path.read_bytes());value=struct.pack('<2f',*anchor)
        if raw[32:40]!=value:raw[32:40]=value;path.write_bytes(raw);changed.append(name)
    for name,source in names.items():
        box,size=geometry[name];ref,_=geometry[families[source]]
        anchor=(((ref[0]+ref[2])*.5-box[0])*size[0]/(box[2]-box[0]),(ref[3]-box[1])*size[1]/(box[3]-box[1]))
        patch(name,anchor)
    for action in ('walk','eat','dance'):
        base='zombie_dance' if action=='dance' else 'avatar_body_'+action
        if not (destination/(base+'.pva')).exists():continue
        anchor=struct.unpack_from('<2f',(destination/(base+'.pva')).read_bytes(),32)
        for path in destination.glob('avatar_*_'+action+'.pva'):patch(path.stem,anchor)
    for asset in assets:
        if asset['file'][:-4] in changed:asset['sha256']=hashlib.sha256((destination/asset['file']).read_bytes()).hexdigest()
    return {'aligned':len(changed),'clips':changed,'method':'shared source-canvas origin; compressed frame pixels unchanged'}
