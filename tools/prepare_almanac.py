"""Preserve the supplied edition's Almanac text, rather than rewriting it."""
import re,struct
from PIL import Image
PLANT_KEYS=['PEASHOOTER','SUNFLOWER','WALL_NUT','CHERRY_BOMB','POTATO_MINE','SNOW_PEA','CHOMPER','REPEATER','PUFF_SHROOM','SUN_SHROOM','FUME_SHROOM','GRAVE_BUSTER','HYPNO_SHROOM','SCAREDY_SHROOM','ICE_SHROOM','DOOM_SHROOM','LILY_PAD','SQUASH','THREEPEATER','TANGLE_KELP','JALAPENO','SPIKEWEED','TORCHWOOD','TALL_NUT','SEA_SHROOM','PLANTERN','CACTUS','BLOVER','SPLIT_PEA','STARFRUIT','PUMPKIN','MAGNET_SHROOM','CABBAGE_PULT','FLOWER_POT','KERNEL_PULT','COFFEE_BEAN','GARLIC','UMBRELLA_LEAF','MARIGOLD','MELON_PULT','GATLING_PEA','TWIN_SUNFLOWER','GLOOM_SHROOM','CATTAIL','WINTER_MELON','GOLD_MAGNET','SPIKEROCK','COB_CANNON','IMITATER']
ZOMBIE_KEYS=['ZOMBIE','CONEHEAD_ZOMBIE','BUCKETHEAD_ZOMBIE','POLE_VAULTING_ZOMBIE','FLAG_ZOMBIE','NEWSPAPER_ZOMBIE','SCREEN_DOOR_ZOMBIE','FOOTBALL_ZOMBIE','DANCING_ZOMBIE','BACKUP_DANCER','DUCKY_TUBE_ZOMBIE','SNORKEL_ZOMBIE','ZOMBONI','ZOMBIE_BOBSLED_TEAM','DOLPHIN_RIDER_ZOMBIE','JACK_IN_THE_BOX_ZOMBIE','BALLOON_ZOMBIE','DIGGER_ZOMBIE','POGO_ZOMBIE','ZOMBIE_YETI','BUNGEE_ZOMBIE','LADDER_ZOMBIE','CATAPULT_ZOMBIE','GARGANTUAR','IMP','BOSS']
def prepare_almanac(imported,destination,previews,write_pva):
    files={p.relative_to(imported/'archives'/a).as_posix().lower():p for a in ('main','compressed','en') for p in (imported/'archives'/a).rglob('*') if p.is_file()}
    text=files['properties/lawnstrings.txt'].read_text(encoding='utf-16');strings={k:v.strip() for k,v in re.findall(r'\[([^\]]+)\]\s*([^\[]*)',text)}
    def clean(value):return re.sub(r'\{[^}]*\}','',value).replace('\t',' ').replace('\r','')
    records=[]
    for key in PLANT_KEYS+ZOMBIE_KEYS:
        if key not in strings:raise ValueError('Missing disc Almanac title: '+key)
        title=clean(strings[key]);body=clean(strings.get(key+'_DESCRIPTION_HEADER',''))+'\n\n'+clean(strings.get(key+'_DESCRIPTION',''))
        a=title.encode('utf-8');b=body.encode('utf-8');records.append(struct.pack('<II',len(a),len(b))+a+b)
    with (destination/'almanac.bin').open('wb') as f:
        f.write(struct.pack('<4sI',b'PVB1',len(records)));offset=8+len(records)*4
        for row in records:f.write(struct.pack('<I',offset));offset+=len(row)
        for row in records:f.write(row)
    im=Image.open(files['images/almanac_plantback.ptx']).convert('RGBA').resize((480,272),Image.Resampling.LANCZOS)
    return [write_pva(destination/'almanac_background.pva',[im],1)]
