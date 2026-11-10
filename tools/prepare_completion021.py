"""Additional owned-disc rewards, house styles and secret-effect artwork."""
from functools import lru_cache
import math
from PIL import Image
from affine_render import AffineLayerCache,composite_affine


def prepare_completion021(imported,destination,previews,write_pva):
    from prepare_assets import frames_from_xml
    files={p.relative_to(imported/'archives'/a).as_posix().lower():p for a in ('main','compressed','en') for p in (imported/'archives'/a).rglob('*') if p.is_file()}
    assets=[]
    @lru_cache(maxsize=192)
    def image(key):
        with Image.open(files[key.lower()]) as source:return source.convert('RGBA')
    def save(name,frames):
        assets.append(write_pva(destination/(name+'.pva'),frames,1))
        if previews:frames[0].save(previews/(name+'.png'))
    trophy=image('images/sunflower_trophy.ptx');gold=trophy.crop((trophy.width//2,0,trophy.width,trophy.height));gold.thumbnail((49,58),Image.Resampling.LANCZOS)
    save('reward_gold_sunflower',[gold])
    bag=image('images/moneybag_hi_res.ptx').copy();bag.thumbnail((49,58),Image.Resampling.LANCZOS);save('reward_moneybag',[bag])
    for name,key,size in [('future','particles/zombiefutureglasses.ptx',(27,12)),('mustache','reanim/zombie_mustache1.ptx',(24,13)),('daisy','particles/daisy.ptx',(20,20)),('pinata','particles/pinata.ptx',(20,20))]:
        art=image(key).copy();frames=[art.crop((i*(art.width//5),0,(i+1)*(art.width//5),art.height)) for i in range(5)] if name=='pinata' else [art]
        for frame in frames:frame.thumbnail(size,Image.Resampling.LANCZOS)
        save('secret_'+name,frames)
    _,tracks=frames_from_xml(files['reanim/house.reanim']);cache=AffineLayerCache();houses=[]
    # Each house style has five authored growth tiers. Never mix other styles'
    # decorative layers into the selected house.
    for style in range(1,6):
        prefix='house_'+str(style)
        for tier in range(5):
            canvas=Image.new('RGBA',(2048,1536))
            for name,frames in tracks:
                if name!=prefix and not name.startswith(prefix+'_'):continue
                f=frames[tier*16]
                if f['f']<0 or f['a']<=0 or not f['i']:continue
                source=image('reanim/'+f['i'].lower().removeprefix('image_reanim_')+'.ptx')
                kx,ky=map(math.radians,(f['kx'],f['ky']));a,b=math.cos(kx)*f['sx'],-math.sin(ky)*f['sy'];c,d=math.sin(kx)*f['sx'],math.cos(ky)*f['sy'];det=a*d-b*c
                if abs(det)<1e-8:continue
                x,y=f['x']+768,f['y']+384
                composite_affine(canvas,source,(a,b,c,d,x,y),(d/det,-b/det,(b*y-d*x)/det,-c/det,a/det,(c*x-a*y)/det),f['a'],cache)
            box=canvas.getbbox()
            if not box or box[0]==0 or box[1]==0 or box[2]>=2048 or box[3]>=1536:raise ValueError('House source canvas is empty or clipped')
            art=canvas.crop(box);scale=min(.26,282/art.width,156/art.height);art=art.resize((max(1,round(art.width*scale)),max(1,round(art.height*scale))),Image.Resampling.LANCZOS)
            frame=Image.new('RGBA',(282,156));frame.alpha_composite(art,((282-art.width)//2,156-art.height));houses.append(frame)
    save('house_previews',houses)
    return assets
