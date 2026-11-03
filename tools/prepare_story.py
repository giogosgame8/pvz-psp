"""Original night chapter and story-note resources from the supported PS3 disc."""
from PIL import Image
NIGHT_SPECS=[]
for output,animation,clip in [('puff','PuffShroom','anim_idle'),('puff_shoot','PuffShroom','anim_shooting'),('sunshroom','SunShroom','anim_idle'),('sunshroom_big','SunShroom','anim_bigidle'),('fume','FumeShroom','anim_idle'),('fume_shoot','FumeShroom','anim_shooting'),('gravebuster','Gravebuster','anim_idle'),('hypno','HypnoShroom','anim_idle'),('scaredy','ScaredyShroom','anim_idle'),('scaredy_shoot','ScaredyShroom','anim_shooting'),('scaredy_hide','ScaredyShroom','anim_scaredidle'),('iceshroom','IceShroom','anim_idle'),('doom','DoomShroom','anim_idle')]:NIGHT_SPECS.append((output,animation,clip,.66,''))
for output,animation in [('paper','Zombie_paper'),('football','Zombie_football'),('dancer','Zombie_disco'),('backup','Zombie_backup')]:
    for pose in ['walk','eat','death']:NIGHT_SPECS.append((output+'_'+pose,animation,'anim_'+pose,.48,''))
for pose in ['walk','eat']:
    NIGHT_SPECS.extend([('paper_angry_'+pose,'Zombie_paper','anim_'+pose+'_nopaper',.48,''),('door_'+pose,'Zombie','anim_'+pose,.48,'door')])
def prepare_story(imported,destination,previews,write_pva):
    files={p.relative_to(imported/'archives'/archive).as_posix().lower():p for archive in ('main','compressed','en') for p in (imported/'archives'/archive).rglob('*') if p.is_file()}
    result=[]
    def save(name,im):
        result.append(write_pva(destination/(name+'.pva'),[im],1))
        if previews:im.save(previews/(name+'.png'))
    bg=Image.open(files['images/background2.ptx']).convert('RGBA').crop((350,0,1390,720)).resize((480,272),Image.Resampling.LANCZOS);save('background_night',bg)
    for i in range(1,6):
        path=files['images/'+('zombienote'+str(i)+'.ptx' if i<5 else 'zombiefinalnote.ptx')]
        im=Image.open(path).convert('RGBA');im.thumbnail((360,183),Image.Resampling.LANCZOS);save('note'+str(i),im)
    for name,key,size in [('gravestone','images/mp_tombstone.ptx',(32,35)),('crater','images/crater.ptx',(30,15)),('projectile_spore','particles/puffshroom_puff2.ptx',(10,10))]:
        im=Image.open(files[key]).convert('RGBA');im.thumbnail(size,Image.Resampling.LANCZOS);save(name,im)
    return result
