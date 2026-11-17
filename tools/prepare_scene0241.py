"""Source-canvas and attachment placement fixes for the owned PS3 artwork."""
import struct

PLANT_BANKS='pea_idle sunflower wallnut cherry mine snow_idle chomper repeater_idle puff sunshroom fume gravebuster hypno scaredy iceshroom doom lily squash threepeater kelp jalapeno spikeweed torchwood tallnut sea plantern cactus blover split star pumpkin magnet cabbage pot kernel coffee garlic umbrella marigold melon gatling twin gloom cattail winter goldmagnet spikerock cob imitater mine_unarmed'.split()

def source_canvas(animation):
    # The boss has authored coordinates down to x=-382, y=-593. The old
    # source origin (96,96) discarded his upper body before texture encoding.
    if animation=='Zombie_boss':return (2048,1664),(448,800)
    size=1024 if animation=='Crow' else 1536 if animation=='treeofWisdom' else 768 if animation in ('Zombie_gargantuar','CrazyDave') else 384
    origin=192 if animation=='Crow' else 96
    return (size,size),(origin,origin)

def support_offset(name,geometry):
    box,size=geometry[name]
    height=26 if name=='pot' else -10 if name in ('chomper','plantern') else 0 if name=='scaredy' else -5
    height+=dict(lily=25,star=10,kelp=24,sea=28,coffee=-20,pumpkin=15,puff=5,scaredy=-14,gravebuster=-40,spikeweed=5,spikerock=11).get(name,0)
    # Original plants share an 80x80 placement cell. Crop centers are not
    # attachment points: an asymmetric pult must retain its original origin.
    sx=size[0]/(box[2]-box[0]);sy=size[1]/(box[3]-box[1])
    return (((box[0]+box[2])*.5-96-40)*sx*.68,
            (box[3]-96-80+height)*sy*.68)

def write_support_offsets(destination,geometry):
    values=[support_offset(name,geometry) for name in PLANT_BANKS]
    path=destination/'plant_support.bin'
    path.write_bytes(struct.pack('<4sI',b'PVPO',len(values))+b''.join(struct.pack('<2f',*v) for v in values))
    return {'file':path.name,'entries':len(values),'method':'original 80x80 placement origin and Flower Pot height offsets'}

def swimming_specs(specs):
    result=[]
    for name,source,marker,scale,options in specs:
        gear=options.get('gear','') if isinstance(options,dict) else options
        if source=='Zombie' and isinstance(gear,str) and gear.startswith('duck') and marker=='anim_walk':marker='anim_swim'
        result.append((name,source,marker,scale,options))
    return result

class BossDriver:
    """The original driver attachment follows Boss_head2's base-pose matrix."""
    def __init__(self,files,tracks,parse,clip,multiply,invert,texture,marker,start,stop):
        from prepare_story023 import matrix
        self.matrix=matrix;self.multiply=multiply;self.texture=texture
        self.parent=dict(tracks)['Boss_head2']
        self.reference=invert(matrix(self.parent[0]))
        _,self.tracks=parse(files['reanim/zombie_boss_driver.reanim'])
        self.idle=clip(self.tracks,'anim_idle');self.drive=clip(self.tracks,'anim_drive')
        self.marker=marker;self.start=start;self.stop=stop
    def draw(self,canvas,parent,index,draw,origin):
        if parent['f']<0 or parent['a']<=0:return
        rate=6 if self.marker=='anim_idle' else 16 if self.marker=='anim_RV_1' else 18 if self.marker=='anim_bungee_1_leave' else 12
        elapsed=(index-self.start)/rate
        start,stop=self.drive if self.marker.startswith('anim_head_attack_') else self.idle
        speed=36 if self.marker.startswith('anim_head_attack_') else 18
        at=start+int(elapsed*speed)%(stop-start)
        overlay=self.multiply(self.multiply(self.matrix(parent),self.reference),(1.2,0,0,1.2,28,-84))
        for _,frames in self.tracks:
            f=frames[at]
            if f['f']<0 or not f['i'] or f['a']<=0:continue
            draw(canvas,self.texture(f['i']),self.multiply(overlay,self.matrix(f)),f['a']*parent['a'],origin)
