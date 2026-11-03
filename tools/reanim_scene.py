"""Offline composition of original nested reanimation scenes."""
from pathlib import Path
import math,re,xml.etree.ElementTree as ET,bisect
from functools import lru_cache
from PIL import Image

def multiply(m,n):
 a,b,c,d,x,y=m;e,f,g,h,u,v=n;return a*e+b*g,a*f+b*h,c*e+d*g,c*f+d*h,a*u+b*v+x,c*u+d*v+y
def matrix(f):
 x,y=map(math.radians,(f['kx'],f['ky']));return math.cos(x)*f['sx'],-math.sin(y)*f['sy'],math.sin(x)*f['sx'],math.cos(y)*f['sy'],f['x'],f['y']

class Scene:
 def __init__(self,imported):
  self.files={p.relative_to(imported/'archives'/a).as_posix().lower():p for a in ('main','compressed','en') for p in (imported/'archives'/a).rglob('*') if p.is_file()};self.missing=set();self.time=0;self.timings=[]
  self.fonts={}
 @lru_cache(maxsize=96)
 def animation(self,name):
  path=self.files.get('reanim/'+name.lower()+'.reanim')
  if not path:
   # This authoring-only attachment has no registered animation on the disc.
   if name.lower()!='zombie9':self.missing.add('animation:'+name)
   return 7,[]
  root=ET.fromstring('<reanim>'+path.read_text(encoding='utf-8-sig')+'</reanim>');tracks=[]
  for track in root.findall('track'):
   f=dict(x=0.,y=0.,kx=0.,ky=0.,sx=1.,sy=1.,a=1.,f=0.,i='',text='',font='',born=0);frames=[]
   for index,t in enumerate(track.findall('t')):
    old=f.copy()
    for v in t:
     if v.tag in f:f[v.tag]=(v.text or '') if v.tag in ('i','text','font') else float(v.text or 0)
    if f['text']!=old['text'] or (old['f']<0<=f['f']):f['born']=index
    frames.append(f.copy())
   tracks.append((track.findtext('name'),frames))
  return float(root.findtext('fps','12')),tracks
 @lru_cache(maxsize=384)
 def texture(self,key):
  name=key.lower().removeprefix('image_reanim_').removeprefix('image_');p=next((self.files[k] for folder in ('reanim','images') for suffix in ('.ptx','.jpg','.png') if (k:=folder+'/'+name+suffix) in self.files),None)
  if not p:self.missing.add('texture:'+key);return None
  image=Image.open(p).convert('RGBA')
  if p.suffix.lower()=='.jpg':
   alpha=self.files.get(p.parent.name.lower()+'/'+name+'_.png') or self.files.get(p.parent.name.lower()+'/'+name+'_.jpg')
   if alpha:image.putalpha(Image.open(alpha).convert('L'))
  return image
 def font(self,name):
  name=name.lower().removeprefix('font_').replace('black','')
  if name in self.fonts:return self.fonts[name]
  path=self.files.get('data/'+name+'.txt')
  if not path:name='briannetod32';path=self.files['data/briannetod32.txt']
  t=path.read_text(encoding='utf-8-sig')
  def block(k):return re.search(r'Define\s+'+k+r'\s*\((.*?)\);',t,re.S).group(1)
  chars=[a or b for a,b in re.findall(r"'([^']*)'|\"([^\"]*)\"",block('CharList'))];widths=list(map(int,re.findall(r'-?\d+',block('WidthList'))));rects=[tuple(map(int,v)) for v in re.findall(r'\(\s*(-?\d+),\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)\)',block('RectList'))];p=self.files.get('data/'+name+'_.gif') or self.files.get('data/'+name+'.png');atlas=Image.open(p);atlas=atlas.getchannel('A') if atlas.mode=='RGBA' else atlas.convert('L');result=atlas,{c:(widths[i],rects[i]) for i,c in enumerate(chars)};self.fonts[name]=result;return result
 @lru_cache(maxsize=256)
 def text(self,text,font,color):
  atlas,glyphs=self.font(font);width=sum(glyphs[c][0] if c in glyphs else 11 for c in text);mask=Image.new('L',(max(1,width+4),80));x=2
  for c in text:
   if c not in glyphs:x+=11;continue
   advance,(gx,gy,gw,gh)=glyphs[c];mask.paste(atlas.crop((gx,gy,gx+gw,gy+gh)),(x,0));x+=advance
  box=mask.getbbox();h=box[3] if box else 1;mask=mask.crop((0,0,mask.width,h+2));im=Image.new('RGBA',mask.size,color);im.putalpha(mask);return im
 def sample(self,frames,pos):
  pos=max(0,min(len(frames)-1,pos));i=int(pos);f=frames[i].copy();n=frames[min(i+1,len(frames)-1)];t=pos-i
  for k in ('x','y','kx','ky','sx','sy','a'):f[k]+=(n[k]-f[k])*t
  return f
 def paint(self,canvas,image,m,alpha):
  if image is None or alpha<=.001:return
  a,b,c,d,x,y=m;det=a*d-b*c
  if abs(det)<1e-7:return
  corners=[(x,y),(x+a*image.width,y+c*image.width),(x+b*image.height,y+d*image.height),(x+a*image.width+b*image.height,y+c*image.width+d*image.height)];left=max(0,math.floor(min(p[0] for p in corners))-1);top=max(0,math.floor(min(p[1] for p in corners))-1);right=min(canvas.width,math.ceil(max(p[0] for p in corners))+1);bottom=min(canvas.height,math.ceil(max(p[1] for p in corners))+1)
  if left>=right or top>=bottom:return
  xx,yy=x-left,y-top;layer=image.transform((right-left,bottom-top),Image.Transform.AFFINE,(d/det,-b/det,(b*yy-d*xx)/det,-c/det,a/det,(c*xx-a*yy)/det),resample=Image.Resampling.BILINEAR)
  if alpha<.999:layer.putalpha(layer.getchannel('A').point(lambda v:round(v*max(0,min(1,alpha)))))
  canvas.alpha_composite(layer,(left,top))
 def mouth(self):
  if not self.timings:return 0
  frame=self.time*7;idx=bisect.bisect_right([v[0] for v in self.timings],frame)-1
  if idx<0:return 0
  before=self.timings[idx];after=self.timings[min(idx+1,len(self.timings)-1)]
  return 0 if before[1]=='OFF' else 1 if frame-before[0]<.2 or after[0]-frame<.4 else {'AA':2,'EE':3,'AW':4,'OH':5}.get(before[1],0)
 def draw(self,canvas,name,pos,parent=(1,0,0,1,0,0),alpha=1,rate=None,depth=0):
  if depth>12:return
  fps,tracks=self.animation(name)
  if not tracks:return
  rate=rate or fps
  # Backgrounds, actors, regular layers, then words/front lights.
  def group(t):
   n=t[0].lower();return 0 if n.startswith('background') else 1 if n.startswith(('attacher__zombie','attacher__undead','attacher__discolights')) else 3 if n.startswith(('words','spotfront','attacher__cattail')) else 2
  ordered=sorted(tracks,key=group) if name.lower().startswith('credits_main') else tracks
  for track,frames in ordered:
   f=self.sample(frames,pos)
   if f['f']<0 or f['a']<=0:continue
   m=multiply(parent,matrix(f));a=alpha*f['a'];text=f['text']
   if text.startswith('attacher__'):
    spec=text[10:];fields=spec.split('[')[0].split('__',1);child=fields[0];marker=fields[1] if len(fields)>1 else None;tags=re.findall(r'\[([^]]+)\]',spec);child_rate=12
    for tag in tags:
     try:child_rate=float(tag)
     except ValueError:pass
    _,child_tracks=self.animation(child)
    if not child_tracks:continue
    start=0;stop=max(len(v) for _,v in child_tracks)
    if marker:
     keys=next((v for n,v in child_tracks if n==marker),None)
     if keys:
      active=[i for i,k in enumerate(keys) if k['f']>=0]
      if active:
       start=active[0];stop=start+1
       while stop<len(keys) and keys[stop]['f']>=0:stop+=1
    elapsed=max(0,pos-f['born'])/rate;phase=elapsed*child_rate;length=max(1,stop-start-1);phase=min(length,phase) if 'hold' in tags or 'once' in tags else phase%length
    self.draw(canvas,child,start+phase,m,a,child_rate,depth+1);continue
   image=None
   if f['i']:
    identifier=f['i']
    if name.lower()=='sunflower' and track=='anim_idle':
     mouth=self.mouth()
     if mouth:identifier='IMAGE_REANIM_SUNFLOWER_HEAD_SING'+str(mouth)
    image=self.texture(identifier)
   elif f['font'] and text:
    black=('black' in f['font'].lower() or (name.lower() in ('credits_main2','credits_main3') and pos<124) or name.lower()=='credits_wearetheundead');image=self.text(text,f['font'],(0,0,0,255) if black else (255,255,255,255));m=multiply(m,(1,0,0,1,-image.width*.5,-image.height))
   if name.lower()=='zombie':
    low=track.lower()
    if any(s in low for s in ('cone','bucket','flaghand','screendoor','mustache','bloated','duckytube')):continue
   self.paint(canvas,image,m,a)

if __name__=='__main__':
 import sys,json
 root=Path(__file__).resolve().parents[1];s=Scene(root.parent/'private-pvz-import');preview=root/'private/review010';preview.mkdir(exist_ok=True)
 for n,p in [('Credits_Main',10),('Credits_Main',145),('Credits_Main2',150),('Credits_Main3',160)]:
  im=Image.new('RGBA',(480,272),(0,0,0,255));s.time=p/7;s.draw(im,n,p,(.6,0,0,272/600,0,0));im.save(preview/(n+'-'+str(p)+'.png'))
 print(json.dumps(sorted(s.missing),indent=2))
