"""Bake owned Dave props relative to the original handing-hand transform."""
from pathlib import Path
import math

PROPS=(
 ('wallnut','Wallnut','anim_idle',(100,393),1.2,12),
 ('hammer','Hammer','anim_whack_zombie',(62,445),1.5,24),
 ('fertilizer','ZenGarden_fertilizer',None,(102,412),1,0),
 ('treefood','TreeFood',None,(102,412),1,0),
 ('moneybag','ZenGarden_fertilizer',None,(90,405),1,0),
)
STORY_SPECS=[('dave_smalltalk','CrazyDave','anim_smalltalk',.32,dict(anchor_parent='dave_idle')),
 ('dave_blahblah','CrazyDave','anim_blahblah',.32,dict(anchor_parent='dave_idle')),
 ('dave_crazy','CrazyDave','anim_crazy',.32,dict(anchor_parent='dave_idle'))]
for prop,*_ in PROPS:
 for pose in ('enter','idle','talk'):
  STORY_SPECS.append(('dave_'+prop+'_'+pose,'CrazyDave','anim_'+pose+'_handing',.32,dict(anchor_parent='dave_idle',prop=prop)))

def matrix(frame):
 kx,ky=math.radians(frame['kx']),math.radians(frame['ky'])
 return math.cos(kx)*frame['sx'],-math.sin(ky)*frame['sy'],math.sin(kx)*frame['sx'],math.cos(ky)*frame['sy'],frame['x'],frame['y']

class DaveProp:
 def __init__(self,kind,files,parent_tracks,parse,clip,texture,multiply,invert):
  _,source,marker,self.offset,self.scale,self.rate=next(p for p in PROPS if p[0]==kind)
  self.kind=kind;self.multiply=multiply;self.invert=invert;self.texture=texture;self.files=files
  self.hand=next(frames for name,frames in parent_tracks if name=='Dave_handinghand')
  self.reference=matrix(self.hand[249])
  _,self.tracks=parse(files['reanim/'+source.lower()+'.reanim'])
  self.start,self.stop=clip(self.tracks,marker) if marker else (0,1)
 def draw(self,canvas,parent_index,elapsed,draw):
  from PIL import Image
  overlay=self.multiply(matrix(self.hand[parent_index]),self.invert(self.reference))
  x,y=self.offset;s=self.scale
  transform=self.multiply(overlay,(s,0,0,s,x,y))
  frame=self.start+min(self.stop-self.start-1,int(elapsed*self.rate)) if self.kind=='hammer' else self.start+int(elapsed*self.rate)%max(1,self.stop-self.start)
  for name,frames in self.tracks:
   if self.kind in ('fertilizer','treefood','moneybag') and name!='bag':continue
   f=frames[min(frame,len(frames)-1)]
   if f['f']<0 or not f['i'] or f['a']<=0:continue
   if self.kind=='moneybag':
    with Image.open(self.files['images/moneybag.ptx']) as im:image=im.convert('RGBA')
   else:image=self.texture(f['i'])
   draw(canvas,image,self.multiply(transform,matrix(f)),f['a'])
