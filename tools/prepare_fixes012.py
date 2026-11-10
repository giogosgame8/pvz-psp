"""Original Squash attack clips and reward particles for the 0.12 fixes."""
from pathlib import Path
import math
from PIL import Image,ImageChops
from affine_render import AffineLayerCache,composite_affine,render_frames

def prepare_fixes012(imported,destination,previews,write_pva):
 from prepare_assets import frames_from_xml,clip_range
 files={p.relative_to(imported/'archives'/a).as_posix().lower():p for a in ('main','compressed','en') for p in (imported/'archives'/a).rglob('*') if p.is_file()};result=[]
 def image(key):return Image.open(files[key]).convert('RGBA')
 def save(name,frames,rate=1,anchor=None):
  result.append(write_pva(destination/(name+'.pva'),frames,rate,anchor))
  if previews:frames[0].save(previews/(name+'.png'))
 _,tracks=frames_from_xml(files['reanim/squash.reanim']);textures={};clips={};layer_cache=AffineLayerCache()
 for name,marker,rate in [('squash','anim_idle',12),('squash_left','anim_lookleft',24),('squash_right','anim_lookright',24),('squash_up','anim_jumpup',24),('squash_down','anim_jumpdown',60)]:
  start,stop=clip_range(tracks,marker);duration=(stop-start-1)/rate;count=max(1,math.ceil(duration*60)+(name!='squash'));frames=[]
  def render(n):
   pos=min(stop-1,start+n*rate/60);index=int(pos);t=pos-index;canvas=Image.new('RGBA',(384,384))
   for _,keys in tracks:
    f=keys[index].copy();other=keys[min(index+1,len(keys)-1)]
    for key in ('x','y','kx','ky','sx','sy','a'):f[key]+=(other[key]-f[key])*t
    if f['f']<0 or f['a']<=0 or not f['i']:continue
    key='reanim/'+f['i'].lower().removeprefix('image_reanim_')+'.ptx'
    if key not in textures:textures[key]=image(key)
    kx,ky=map(math.radians,(f['kx'],f['ky']));a,b=math.cos(kx)*f['sx'],-math.sin(ky)*f['sy'];c,d=math.sin(kx)*f['sx'],math.cos(ky)*f['sy'];det=a*d-b*c
    if abs(det)<1e-8:continue
    x,y=f['x']+96,f['y']+96
    composite_affine(canvas,textures[key],(a,b,c,d,x,y),(d/det,-b/det,(b*y-d*x)/det,-c/det,a/det,(c*x-a*y)/det),f['a'],layer_cache)
   return canvas
  clips[name]=render_frames(render,range(count))
 union=Image.new('L',(384,384));idle=Image.new('L',(384,384))
 for name,frames in clips.items():
  for f in frames:
   union=ImageChops.lighter(union,f.getchannel('A'))
   if name=='squash':idle=ImageChops.lighter(idle,f.getchannel('A'))
 box=union.getbbox();box=(box[0]-2,box[1]-2,box[2]+2,box[3]+2);base=idle.getbbox();size=(round((box[2]-box[0])*.66),round((box[3]-box[1])*.66));anchor=(((base[0]+base[2])/2-box[0])*size[0]/(box[2]-box[0]),(base[3]-box[1])*size[1]/(box[3]-box[1]))
 for name,frames in clips.items():save(name,[f.crop(box).resize(size,Image.Resampling.LANCZOS) for f in frames],60,anchor)
 save('award_rays',[image('particles/awardrays.ptx')]);save('award_glow',[image('particles/awardpickupglow.ptx').resize((96,96),Image.Resampling.LANCZOS)])
 save('award_arrow',[image('particles/downarrow.ptx').resize((16,13),Image.Resampling.LANCZOS)])
 save('award_background',[image('images/awardscreen_back.ptx').resize((480,272),Image.Resampling.LANCZOS)])
 return result
