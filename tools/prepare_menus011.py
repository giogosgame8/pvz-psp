"""Fit original PS3 menu layers and widgets to the handheld viewport."""
from pathlib import Path
import math,json
from PIL import Image
from affine_render import AffineLayerCache,composite_affine,render_frames

def prepare_menus011(imported,destination,previews,write_pva):
 from prepare_assets import frames_from_xml
 files={p.relative_to(imported/'archives'/a).as_posix().lower():p for a in ('main','compressed','en') for p in (imported/'archives'/a).rglob('*') if p.is_file()};assets=[]
 def im(key):return Image.open(files[key.lower()]).convert('RGBA')
 def save(name,frames,fps=1):
  assets.append(write_pva(destination/(name+'.pva'),frames,fps))
  if previews:frames[0].save(previews/(name+'.png'))
 _,tracks=frames_from_xml(files['reanim/mainmenu3/mainmenu.reanim'])
 # UI states are placed by the original scene transforms; no painted substitute art.
 selectors={'adventure','start adventure','more ways','more','help','puzzle button','minigames button','survival button','back stone button','shop button','almanac button','zengardenbutton','mail button','back ball button'}
 states={}
 for b in selectors:
  stem=b.removesuffix(' button') if b not in ('back ball button',) else b
  for suffix in ('',' selected',' pressed'):states[b if not suffix else stem+suffix]=(b,suffix)
 layers=[]
 for name,frames in tracks:
  n=name.strip().lower();f=frames[0]
  if f['f']<0 or f['a']<=0 or not f['i']:continue
  if any(k in n for k in ('leaderboard','mail alert')):continue
  if n.endswith((' selected',' pressed')) and n not in states:continue
  if n in ('vs button','vs selected','vs pressed','vs coop','vs coop selected','vs coop pressed'):continue
  key=f['i'].lower().removeprefix('image_reanim_layer_').removeprefix('image_reanim_').removesuffix('.png').strip()
  p=files.get('reanim/mainmenu3/'+key+'.ptx') or files.get('reanim/'+key+'.ptx')
  if not p:continue
  src=Image.open(p).convert('RGBA');kx,ky=map(math.radians,(f['kx'],f['ky']));a,b=math.cos(kx)*f['sx'],-math.sin(ky)*f['sy'];c,d=math.sin(kx)*f['sx'],math.cos(ky)*f['sy'];det=a*d-b*c
  if abs(det)<1e-8:continue
  layers.append((n,f,src,(a,b,c,d,det)))
 layer_cache=AffineLayerCache()
 def page(offset,selected,adventure='adventure'):
  out=Image.new('RGBA',(1280,720))
  for n,f,src,(a,b,c,d,det) in layers:
   if n in states:
    base,suffix=states[n]
    if base in ('adventure','start adventure') and base!=adventure:continue
    if suffix!=(' selected' if base==selected else ''):continue
   x,y=f['x']-offset,f['y']
   if x>1300 or x+src.width*abs(a)+src.height*abs(b)<-20:continue
   composite_affine(out,src,(a,b,c,d,x,y),(d/det,-b/det,(b*y-d*x)/det,-c/det,a/det,(c*x-a*y)/det),f['a'],layer_cache)
  return out.resize((480,272),Image.Resampling.LANCZOS)
 save('menu_main',render_frames(lambda args:page(*args),[(1360,s,a) for a in ('adventure','start adventure') for s in (a,'more ways','more','help','player name')]))
 save('menu_modes',render_frames(lambda s:page(0,s),('minigames button','puzzle button','survival button','back stone button')))
 save('menu_collection',render_frames(lambda s:page(2560,s),('shop button','almanac button','zengardenbutton','mail button','back ball button')))
 # Original nine-piece stone dialog and three-piece stone button.
 def panel(width,height):
  out=Image.new('RGBA',(width,height));xs=[0,34,width-34];ys=[0,31,height-18];ws=[34,width-68,34];hs=[31,height-49,18]
  for row,v in enumerate(('top','center','bottom')):
   for col,h in enumerate(('left','middle','right')):out.alpha_composite(im('images/dialog_'+v+h+'.ptx').resize((ws[col],hs[row]),Image.Resampling.LANCZOS),(xs[col],ys[row]))
  return out
 save('ui_dialog',[panel(360,200)])
 buttons=[]
 for glow in ('','glow_'):
  out=Image.new('RGBA',(180,28))
  for x,w,key in [(0,20,'left'),(20,140,'middle'),(160,20,'right')]:out.alpha_composite(im('images/button_'+glow+key+'.ptx').resize((w,28),Image.Resampling.LANCZOS),(x,0))
  buttons.append(out)
 save('ui_button',buttons)
 save('ui_slider',[im('images/options_sliderslot.ptx')]);save('ui_knob',[im('images/options_sliderknob2.ptx')])
 save('challenge_frames',[im('images/challenge_window.ptx'),im('images/challenge_window_highlight.ptx')])
 sheet=im('images/challenge_thumbnails.ptx');w=sheet.width//22;save('challenge_thumbs',[sheet.crop((i*w,0,(i+1)*w,sheet.height)).resize((92,69),Image.Resampling.LANCZOS) for i in range(22)])
 save('book_pages',[im('images/almanac_plantback.ptx').resize((480,272),Image.Resampling.LANCZOS),im('images/almanac_zombieback.ptx').resize((480,272),Image.Resampling.LANCZOS)])
 save('seed_chooser',[im('images/seedchooser_background.ptx').resize((278,206),Image.Resampling.LANCZOS)])
 paper=im('images/zombienote.jpg');paper.putalpha(im('images/zombienote_.png').convert('L'));save('note_paper',[paper.resize((372,236),Image.Resampling.LANCZOS)])
 # Keep Dave's original car and hatch visible behind the shop's items.
 car=im('images/store_background.ptx').resize((480,272),Image.Resampling.LANCZOS)
 car.alpha_composite(im('images/store_car.ptx').resize((350,220),Image.Resampling.LANCZOS),(122,52))
 car.alpha_composite(im('images/store_hatchbackopen.ptx').resize((212,87),Image.Resampling.LANCZOS),(174,12))
 save('shop_scene',[car]);save('shop_bubble',[im('images/store_speechbubble2.ptx')])
 icon_names={13:'reanim/zengarden-wateringcan4.ptx',14:'reanim/zengarden_fertilize_bag1.ptx',15:'reanim/zengarden_bugspray_bottle.ptx',16:'reanim/zengarden_phonograph3.ptx',17:'images/zen_gardenglove.ptx',18:'images/store_mushroomgardenicon.ptx',19:'images/zen_wheelbarrow.ptx',21:'images/store_packetupgrade.ptx',25:'images/store_aquariumgardenicon.ptx',27:'images/store_treeofwisdomicon.ptx',28:'images/treefood.ptx',29:'images/store_firstaidwallnuticon.ptx'}
 icons=[]
 for k in range(37):
  out=Image.new('RGBA',(48,48));key=icon_names.get(k)
  if key not in files:
   candidates={16:['reanim/zengarden_phonograph.ptx','images/phonograph.ptx'],19:['images/zen_wheelbarrow.ptx','images/wheelbarrow.ptx'],28:['images/treefood.ptx','reanim/treefood.ptx']}.get(k,[]);key=next((v for v in candidates if v in files),None)
  if key:
   art=im(key);art.thumbnail((44,44),Image.Resampling.LANCZOS);out.alpha_composite(art,((48-art.width)//2,48-art.height))
  icons.append(out)
 save('shop_icons',icons)
 # Generic blast clouds use particle art and never include a Cherry Bomb plant.
 dust=im('particles/dust_puffs.ptx');save('impact_dust',[dust.crop((i*24,0,(i+1)*24,27)) for i in range(4)],12)
 from prepare_menu_scene020 import prepare_menu_scene
 assets.extend(prepare_menu_scene(imported,destination,write_pva))
 return assets

if __name__=='__main__':
 import sys,shutil
 sys.path.insert(0,str(Path(__file__).parent));from prepare_assets import write_pva
 r=Path(__file__).resolve().parents[1];dest=r/'private/update-0.11-data';preview=r/'private/review011';preview.mkdir(parents=True,exist_ok=True)
 if not dest.exists():shutil.copytree(r/'private/update-0.10-data',dest)
 assets=prepare_menus011(r.parent/'private-pvz-import',dest,preview,write_pva);m=json.loads((dest/'manifest.json').read_text());changed={a['file'] for a in assets};m['assets']=[a for a in m['assets'] if a['file'] not in changed]+assets;m['texture_bytes']=sum(a.get('decoded_bytes',a['bytes']-a.get('header_bytes',32)) for a in m['assets']);(dest/'manifest.json').write_text(json.dumps(m,indent=2));print('Prepared',len(assets),'menu assets')
