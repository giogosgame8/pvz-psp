"""Layered PSP zombie customization made from the owner's PS3 artwork."""
import math
import struct
from PIL import Image
from affine_render import composite_affine,render_frames


def prepare_customizer(imported,destination,previews,write_pva,geometry):
    from prepare_assets import frames_from_xml,clip_range
    files={p.relative_to(imported/'archives'/a).as_posix().lower():p for a in ('main','compressed','en') for p in (imported/'archives'/a).rglob('*') if p.is_file()}
    fps,tracks=frames_from_xml(files['reanim/zombie.reanim']);tracks=dict(tracks);result=[]
    def image(name):return Image.open(files['reanim/'+name.lower()+'.ptx']).convert('RGBA')
    choices=[('face','anim_head1',['Zombie_head']+['Zombie_head_sunglasses'+str(i) for i in range(1,5)]),
             ('hair','anim_hair',['Zombie_hair']),
             ('cone','anim_cone',['Zombie_cone'+str(i) for i in range(1,4)]),
             ('bucket','anim_bucket',['Zombie_bucket'+str(i) for i in range(1,4)]),
             ('helmet','anim_head1',['Zombie_football_helmet'+('' if i==1 else str(i)) for i in range(1,4)]),
             ('mustache','Zombie_mustache',['Zombie_mustache'+str(i) for i in range(1,4)])]
    for action,marker in [('walk','anim_walk'),('eat','anim_eat')]:
        start,stop=clip_range(list(tracks.items()),marker);count=stop-start
        box,size=geometry['avatar_body_'+action]
        header=(destination/('avatar_body_'+action+'.pva')).read_bytes()[:40]
        anchor=struct.unpack_from('<2f',header,32)
        for part,track,names in choices:
            textures=[image(name) for name in names];keys=tracks[track]
            def frame(index):
                variant,offset=divmod(index,count);f=keys[start+offset]
                canvas=Image.new('RGBA',(384,384));kx,ky=map(math.radians,(f['kx'],f['ky']))
                a,b,c,d=math.cos(kx)*f['sx'],-math.sin(ky)*f['sy'],math.sin(kx)*f['sx'],math.cos(ky)*f['sy']
                x,y=f['x']+96,f['y']+96
                if part=='face' and variant:x-=a*4;y-=c*4
                if part=='helmet':x-=a*6+b*23;y-=c*6+d*23
                det=a*d-b*c
                if abs(det)>1e-8:
                    inverse=(d/det,-b/det,(b*y-d*x)/det,-c/det,a/det,(c*x-a*y)/det)
                    composite_affine(canvas,textures[variant],(a,b,c,d,x,y),inverse,f['a'])
                return canvas.crop(box).resize(size,Image.Resampling.LANCZOS)
            frames=render_frames(frame,range(count*len(textures)))
            name='avatar_'+part+'_'+action
            result.append(write_pva(destination/(name+'.pva'),frames,fps,anchor))
            if previews:frames[0].save(previews/(name+'.png'))
    return result
