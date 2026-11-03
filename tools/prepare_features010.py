"""Private story, garden and combat data for the native PSP expansion."""
from pathlib import Path
import re,struct,json
from PIL import Image

EXPANSION_SPECS=[('dave_idle','CrazyDave','anim_idle',.32,''),('dave_talk','CrazyDave','anim_mediumtalk',.32,''),('dave_enter','CrazyDave','anim_enter',.32,''),('dave_leave','CrazyDave','anim_leave',.32,''),('dave_grab','CrazyDave','anim_grab',.32,''),('garden_sprout','ZenGarden_sprout','anim_idle',.55,''),('snail_walk','Stinky','anim_crawl',.65,''),('heavy_cattail','tank_cattail','anim_drive',.32,''),('bobsled_seated','Zombie_bobsled','anim_idle',.48,''),('bobsled_onfoot','Zombie_bobsled','anim_walk',.48,'')]
for name,clip in [('head_enter','head_enter'),('head_leave','head_leave'),('rv','RV_1'),('bungee','bungee_1_enter')]+[(f'stomp{n}',f'stomp_{n}') for n in range(1,5)]+[(f'spawn{n}',f'spawn_{n}') for n in range(1,6)]+[(f'spit{n}',f'head_attack_{n}') for n in range(1,6)]:
 EXPANSION_SPECS.append(('boss_'+name,'Zombie_boss','anim_'+clip,.25,''))

def prepare_features010(imported,dest,previews,write_pva):
 files={p.relative_to(imported/'archives'/a).as_posix().lower():p for a in ('main','compressed','en') for p in (imported/'archives'/a).rglob('*') if p.is_file()};assets=[]
 def save(name,frames,fps=1):
  assets.append(write_pva(dest/(name+'.pva'),frames,fps))
  if previews:frames[0].save(previews/(name+'.png'))
 for name,key,scale in [('present','images/present.ptx',.4),('chocolate','images/chocolate.ptx',.4),('sled','images/zombie_bobsled1.ptx',.48)]:
  im=Image.open(files[key]).convert('RGBA');save(name,[im.resize((round(im.width*scale),round(im.height*scale)),Image.Resampling.LANCZOS)])
 text=files['data/briannetod16.txt'].read_text(encoding='utf-8-sig')
 def block(k):return re.search(r'Define\s+'+k+r'\s*\((.*?)\);',text,re.S).group(1)
 chars=[a or b for a,b in re.findall(r"'([^']*)'|\"([^\"]*)\"",block('CharList'))];widths=list(map(int,re.findall(r'-?\d+',block('WidthList'))));rects=[tuple(map(int,v)) for v in re.findall(r'\(\s*(-?\d+),\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)\)',block('RectList'))]
 source=Image.open(files['data/briannetod16_.gif']).convert('L');mask=Image.new('L',(512,512));x=y=row=0;glyphs=[(8,0,0,0,0) for _ in range(256)]
 for i,ch in enumerate(chars):
  if len(ch)!=1 or ord(ch)>=256:continue
  sx,sy,w,h=rects[i]
  if x+w+2>512:x=0;y+=row+2;row=0
  if y+h+2>512:raise ValueError('Font atlas overflow')
  mask.paste(source.crop((sx,sy,sx+w,sy+h)),(x,y));glyphs[ord(ch)]=(widths[i],x,y,w,h);x+=w+2;row=max(row,h)
 im=Image.new('RGBA',mask.size,(255,255,255,255));im.putalpha(mask);save('disc_font',[im]);(dest/'disc_font.bin').write_bytes(struct.pack('<4sII',b'PVDF',256,24)+b''.join(struct.pack('<5h',*g) for g in glyphs))
 from reanim_scene import Scene
 from prepare_assets import clip_range
 scene=Scene(imported);_,tracks=scene.animation('treeofWisdom');frames=[]
 for n in range(1,52):
  _,stop=clip_range(tracks,'anim_grow'+str(n));im=Image.new('RGBA',(480,272),(50,100,140,255));scene.draw(im,'treeofWisdom',stop-1,(480/1280,0,0,272/720,0,0));frames.append(im)
 save('tree_stages',frames)
 return assets
