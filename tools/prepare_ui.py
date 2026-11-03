"""Bake private UI art from the supplied PS3 resources for the PSP viewport."""
import math,re,sys
from pathlib import Path
from PIL import Image,ImageDraw,ImageFilter

def prepare_ui(imported,destination,previews,write_pva):
    from prepare_assets import frames_from_xml
    root=imported/'archives';files={p.relative_to(root/a).as_posix().lower():p for a in ('main','compressed','en') for p in (root/a).rglob('*') if p.is_file()}
    def image(name):
        with Image.open(files[name.lower()]) as im:return im.convert('RGBA')
    assets=[]
    def save(name,frames,fps=1):
        assets.append(write_pva(destination/(name+'.pva'),frames,fps))
        if previews:frames[0].save(previews/(name+'.png'))
    # Original console panorama, cropped to the main graveyard/town view.
    fps,tracks=frames_from_xml(files['reanim/mainmenu3/mainmenu.reanim']);canvas=Image.new('RGBA',(4400,820))
    for name,frames in tracks:
        low=name.strip().lower();f=frames[0]
        if any(s in low for s in ('selected','pressed','press','leaderboard','mail alert')) or low in ('adventure','adventure coop','start adventure','more ways','more'):continue
        if f['f']<0 or f['a']<=0 or not f['i']:continue
        key=f['i'].lower().removeprefix('image_reanim_layer_').removeprefix('image_reanim_').removesuffix('.png').strip()
        p=files.get('reanim/mainmenu3/'+key+'.ptx') or files.get('reanim/'+key+'.ptx')
        if not p:continue
        with Image.open(p) as source:im=source.convert('RGBA')
        kx,ky=map(math.radians,(f['kx'],f['ky']));a,b=math.cos(kx)*f['sx'],-math.sin(ky)*f['sy'];c,d=math.sin(kx)*f['sx'],math.cos(ky)*f['sy'];det=a*d-b*c
        if abs(det)<1e-8:continue
        x,y=f['x'],f['y'];layer=im.transform(canvas.size,Image.Transform.AFFINE,(d/det,-b/det,(b*y-d*x)/det,-c/det,a/det,(c*x-a*y)/det),resample=Image.Resampling.BICUBIC)
        if f['a']<1:layer.putalpha(layer.getchannel('A').point(lambda v:round(v*f['a'])))
        canvas.alpha_composite(layer)
    save('menu_background',[canvas.crop((1360,0,2640,720)).resize((480,272),Image.Resampling.LANCZOS)])
    for output,key in (('menu_adventure','adventure'),('menu_start','start adventure'),('menu_more','more ways'),('menu_extras','more')):
        frames=[]
        for suffix in ('',' selected'):
            im=image('reanim/mainmenu3/'+key+suffix+'.ptx');frames.append(im.resize((round(im.width*.375),round(im.height*272/720)),Image.Resampling.LANCZOS))
        save(output,frames)
    # Use the original Brianne glyph atlas for the large warning, not scaled
    # prototype bitmap letters. The warning is wrapped and fitted offline.
    text=files['data/briannetod32.txt'].read_text(encoding='utf-8-sig')
    def block(name):return re.search(r'Define\s+'+name+r'\s*\((.*?)\);',text,re.S).group(1)
    chars=[a or b for a,b in re.findall(r"'([^']*)'|\"([^\"]*)\"",block('CharList'))]
    widths=[int(v) for v in re.findall(r'-?\d+',block('WidthList'))]
    rects=[tuple(map(int,v)) for v in re.findall(r'\(\s*(-?\d+),\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)\)',block('RectList'))]
    atlas=Image.open(files['data/briannetod32_.gif']).convert('L');glyphs={ch:(widths[i],rects[i]) for i,ch in enumerate(chars)}
    def title(label,width,color=(245,226,166),height=None):
        lines=label.split('\n');layer=Image.new('L',(1800,len(lines)*58+30))
        lengths=[sum(11 if ch==' ' else glyphs[ch][0] for ch in line) for line in lines]
        for row,line in enumerate(lines):
            x=12+(max(lengths)-lengths[row])//2
            for ch in line:
                if ch==' ':x+=11;continue
                advance,(gx,gy,gw,gh)=glyphs[ch];glyph=atlas.crop((gx,gy,gx+gw,gy+gh));layer.paste(glyph,(x,row*58+8));x+=advance
        box=layer.getbbox();layer=layer.crop((max(0,box[0]-4),max(0,box[1]-4),box[2]+4,box[3]+4))
        outline=layer.filter(ImageFilter.MaxFilter(5));im=Image.new('RGBA',layer.size);im.paste((24,18,15,255),(0,0),outline);im.paste((*color,255),(0,0),layer)
        factor=min(width/im.width,(height/im.height) if height else 99);return im.resize((round(im.width*factor),round(im.height*factor)),Image.Resampling.LANCZOS)
    save('huge_wave',[title('A HUGE WAVE OF ZOMBIES\nIS COMING!',410,(232,39,28),58)])
    for name,label in (('title_replay','REPLAY LEVELS'),('title_options','OPTIONS'),('title_controls','HOW TO PLAY'),('title_seeds','CHOOSE YOUR PLANTS')):
        save(name,[title(label,280,height=31)])
    save('seedbank',[image('images/seedbank.ptx')])
    save('seed_packet',[image('images/seedpacket_larger.ptx').resize((32,40),Image.Resampling.LANCZOS)])
    belt=image('images/conveyorbelt.ptx');frames=[belt.crop((0,i*16,502,(i+1)*16)).resize((386,7),Image.Resampling.LANCZOS) for i in range(6)]
    save('conveyor_strip',frames,25)
    save('conveyor_frame',[image('images/conveyorbelt_backdrop.ptx').resize((398,43),Image.Resampling.LANCZOS)])
    meter=image('images/flagmeter.ptx');save('meter_empty',[meter.crop((0,0,158,27))]);save('meter_full',[meter.crop((0,27,158,54))])
    parts=image('images/flagmeterparts.ptx');save('meter_head',[parts.crop((0,0,25,25))]);save('meter_flag',[parts.crop((50,0,75,25))])
    # Original shovel art, used beside the conveyor and seed bank if present.
    shovel=next((k for k in files if k=='images/shovel.ptx'),None)
    if shovel:save('shovel',[image(shovel)])
    return assets
