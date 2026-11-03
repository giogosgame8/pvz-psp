"""Original collectible level-award art from the supplied PS3 resources."""
from PIL import Image

def prepare_rewards(imported,destination,previews,write_pva):
    files={p.relative_to(imported/'archives'/a).as_posix().lower():p for a in ('main','compressed','en') for p in (imported/'archives'/a).rglob('*') if p.is_file()}
    assets=[]
    for name,key in [('trophy','trophy_hi_res'),('silver_sunflower','sunflower_trophy'),('shovel','shovel_hi_res'),('almanac','almanac'),('keys','carkeys'),('taco','taco'),('watering_can','wateringcan'),('note','zombienotesmall')]:
        im=Image.open(files['images/'+key+'.ptx']).convert('RGBA')
        if name=='silver_sunflower':im=im.crop((0,0,im.width//2,im.height))
        im.thumbnail((49,58),Image.Resampling.LANCZOS)
        assets.append(write_pva(destination/('reward_'+name+'.pva'),[im],1))
        if previews:im.save(previews/('reward_'+name+'.png'))
    return assets
