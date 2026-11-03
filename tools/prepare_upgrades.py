"""Original purchased plant and economy resources from the supplied disc."""
from PIL import Image
UPGRADE_SPECS=[
    ('gatling','GatlingPea','anim_idle',.66,''),('gatling_shoot','GatlingPea','anim_shooting',.66,''),
    ('twin','TwinSunflower','anim_idle',.66,''),('gloom','GloomShroom','anim_idle',.66,''),('gloom_shoot','GloomShroom','anim_shooting',.66,''),
    ('cattail','Cattail','anim_idle',.66,''),('cattail_shoot','Cattail','anim_shooting',.66,''),
    ('winter','WinterMelon','anim_idle',.66,''),('winter_shoot','WinterMelon','anim_shooting',.66,''),
    ('goldmagnet','GoldMagnet','anim_idle',.66,''),('spikerock','SpikeRock','anim_idle',.66,''),
    ('cob','CobCannon','anim_idle',.66,''),('cob_unarmed','CobCannon','anim_unarmed_idle',.66,''),('cob_shoot','CobCannon','anim_shooting',.66,''),
    ('imitater','Imitater','anim_idle',.66,''),('imitater_morph','Imitater','anim_explode',.66,''),
    ('rake','Rake',None,.66,''),('coin_silver','Coin_silver',None,.25,''),('coin_gold','Coin_gold',None,.25,'')]
def prepare_upgrade_images(imported,destination,previews,write_pva):
    files={p.relative_to(imported/'archives'/a).as_posix().lower():p for a in ('main','compressed','en') for p in (imported/'archives'/a).rglob('*') if p.is_file()};assets=[]
    for name,key in [('projectile_winter','reanim/wintermelon_melon.ptx'),('projectile_cob','reanim/cobcannon_cob.ptx'),('projectile_cattail','reanim/cattail_spike.ptx'),('coin_diamond','reanim/diamond.ptx')]:
        im=Image.open(files[key]).convert('RGBA');im.thumbnail((30,30) if name.endswith('cob') else (16,16),Image.Resampling.LANCZOS);assets.append(write_pva(destination/(name+'.pva'),[im],1))
    im=Image.open(files['images/store_car.ptx']).convert('RGBA');im.thumbnail((480,272),Image.Resampling.LANCZOS);bg=Image.new('RGBA',(480,272),(35,42,31,255));bg.alpha_composite(im,((480-im.width)//2,(272-im.height)//2));assets.append(write_pva(destination/'store_background.pva',[bg],1))
    return assets
