"""Pool, fog and roof resources imported from the owner's PS3 disc."""
from PIL import Image
PLANTS=[('lily','LilyPad'),('squash','Squash'),('threepeater','ThreePeater'),('kelp','TangleKelp'),('jalapeno','Jalapeno'),('spikeweed','SpikeRock'),('torchwood','TorchWood'),('tallnut','Tallnut'),('sea','SeaShroom'),('plantern','Plantern'),('cactus','Cactus'),('blover','Blover'),('split','SplitPea'),('star','Starfruit'),('pumpkin','Pumpkin'),('magnet','MagnetShroom'),('cabbage','Cabbagepult'),('pot','Pot'),('kernel','Cornpult'),('coffee','CoffeeBean'),('garlic','Garlic'),('umbrella','UmbrellaLeaf'),('marigold','Marigold'),('melon','Melonpult')]
CHAPTER_SPECS=[(name,source,'anim_full_idle' if name=='kernel' else 'anim_idle',.66,'') for name,source in PLANTS]
ZOMBIES=[('duck','Zombie','walk','eat','death'),('snorkel','Zombie_snorkle','swim','eat','death'),('zamboni','Zombie_zamboni','drive','drive','drive'),('bobsled','Zombie_bobsled','push','eat','death'),('dolphin','Zombie_dolphinrider','ride','eat','death'),('jack','Zombie_jackbox','walk','eat','death'),('balloon','Zombie_balloon','idle','walk','death'),('digger','Zombie_digger','dig','eat','death'),('pogo','Zombie_pogo','pogo','eat','death'),('yeti','Zombie_yeti','walk','eat','death'),('bungee','Zombie_bungi','drop','grab','raise'),('ladder','Zombie_ladder','ladderwalk','laddereat','death'),('catapult','Zombie_catapult','walk','shoot','bounce'),('garg','Zombie_gargantuar','walk','smash','death'),('imp','Zombie_imp','walk','eat','death'),('boss','Zombie_boss','idle','head_idle','death')]
for name,source,*clips in ZOMBIES:
    for pose,clip in zip(('walk','eat','death'),clips):
        CHAPTER_SPECS.append((name+'_'+pose,source,'anim_'+clip,.25 if name=='boss' else .38 if name=='garg' else .48,'duck' if name=='duck' else ''))
CHAPTER_SPECS.extend([('pool_cleaner','PoolCleaner','anim_land',.44,''),('roof_cleaner','RoofCleaner',None,.44,'')])
def prepare_chapters(imported,destination,previews,write_pva):
    files={p.relative_to(imported/'archives'/a).as_posix().lower():p for a in ('main','compressed','en') for p in (imported/'archives'/a).rglob('*') if p.is_file()};result=[]
    def save(name,im):
        result.append(write_pva(destination/(name+'.pva'),[im],1))
        if previews:im.save(previews/(name+'.png'))
    for name,key in [('pool','background3'),('fog','background4'),('roof','background5'),('boss','background6boss')]:
        im=Image.open(files['images/'+key+'.ptx']).convert('RGBA')
        save('background_'+name,im.crop((350,0,1390,720)).resize((480,272),Image.Resampling.LANCZOS))
    for name,key in [('cabbage','reanim/cabbagepult_cabbage.ptx'),('kernel','reanim/cornpult_kernal.ptx'),('butter','reanim/cornpult_butter.ptx'),('melon','reanim/melonpult_melon.ptx'),('star','images/projectile_star.ptx'),('cactus','images/projectilecactus.ptx')]:
        im=Image.open(files[key]).convert('RGBA');im.thumbnail((18,18) if name=='melon' else (12,12),Image.Resampling.LANCZOS);save('projectile_'+name,im)
    im=Image.open(files['images/scary_pot.ptx']).convert('RGBA');save('vase',im.crop((0,im.height//2,im.width//3,im.height)).resize((29,36),Image.Resampling.LANCZOS))
    for name in ('fireball','iceball'):
        im=Image.open(files['reanim/zombie_boss_'+name+'.ptx']).convert('RGBA');im.thumbnail((38,38),Image.Resampling.LANCZOS);save('boss_'+name,im)
    fog=Image.open(files['images/fog.jpg']).convert('RGBA');fog.putalpha(Image.open(files['images/fog_.jpg']).convert('L'))
    frames=[fog.crop((i*fog.width//7,0,(i+1)*fog.width//7,fog.height)).resize((100,80),Image.Resampling.LANCZOS) for i in range(7)]
    result.append(write_pva(destination/'fog_cloud.pva',frames,.35))
    return result
