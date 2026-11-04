"""Private console award, seed-card and interpolated shooter resources."""
from pathlib import Path
import math,re,struct,zlib,array
from PIL import Image,ImageChops,ImageDraw
from prepare_almanac import PLANT_KEYS
from texture_layout import untile_rgba4444


def first_prepared_frame(path):
    """Portraits need linear pixels, even when their source is GPU-tiled PVA4."""
    b=path.read_bytes();magic,w,h,tw,th,n,stride,fps,ax,ay=struct.unpack_from('<4s6If2f',b)
    if magic not in (b'PVA3',b'PVA4'):raise ValueError('Unsupported prepared image')
    off,length=struct.unpack_from('<II',b,40)
    raw=zlib.decompress(b[40+8*n+off:40+8*n+off+length])
    if magic==b'PVA4':raw=untile_rgba4444(raw,tw,th)
    values=array.array('H',raw)
    rgba=bytes(v for p in values for v in ((p&15)*17,((p>>4)&15)*17,((p>>8)&15)*17,((p>>12)&15)*17))
    return Image.frombytes('RGBA',(tw,th),rgba).crop((0,0,w,h))

def prepare_features09(imported,destination,previews,write_pva):
    from prepare_assets import frames_from_xml,clip_range
    files={p.relative_to(imported/'archives'/a).as_posix().lower():p for a in ('main','compressed','en') for p in (imported/'archives'/a).rglob('*') if p.is_file()};result=[]
    def im(key):return Image.open(files[key.lower()]).convert('RGBA')
    def save(name,frames,fps=1,anchor=None):
        result.append(write_pva(destination/(name+'.pva'),frames,fps,anchor))
        if previews:frames[0].save(previews/(name+'.png'))
    def sample(frames,pos):
        i=int(pos);f=frames[min(i,len(frames)-1)].copy();n=frames[min(i+1,len(frames)-1)];t=pos-i
        for k in ('x','y','kx','ky','sx','sy','a'):f[k]+=(n[k]-f[k])*t
        return f
    textures={}
    def draw(canvas,f):
        if f['f']<0 or f['a']<=0 or not f['i']:return
        key='reanim/'+f['i'].lower().removeprefix('image_reanim_')+'.ptx'
        if key not in textures:textures[key]=im(key)
        image=textures[key];kx,ky=map(math.radians,(f['kx'],f['ky']));a,b=math.cos(kx)*f['sx'],-math.sin(ky)*f['sy'];c,d=math.sin(kx)*f['sx'],math.cos(ky)*f['sy'];det=a*d-b*c
        if abs(det)<1e-7:return
        x,y=f['x']+96,f['y']+96;layer=image.transform(canvas.size,Image.Transform.AFFINE,(d/det,-b/det,(b*y-d*x)/det,-c/det,a/det,(c*x-a*y)/det),resample=Image.Resampling.BICUBIC)
        if f['a']<1:layer.putalpha(layer.getchannel('A').point(lambda v:round(v*f['a'])))
        canvas.alpha_composite(layer)
    alignment=[]
    for prefix,source,rate in [('pea','PeaShooterSingle',35),('repeater','PeaShooter',45),('snow','SnowPea',35)]:
        _,tracks=frames_from_xml(files['reanim/'+source.lower()+'.reanim']);clips={}
        face=next(keys for name,keys in tracks if name=='anim_face')
        origin=sample(face,54)
        # Move the firing head with the same idle stem pose as the body. The
        # original head's recoil remains relative to its authored firing pose.
        for n in range(80):
            idle=sample(face,79+min(24,n/60*18))
            alignment.extend(((idle['x']-origin['x'])*.66,(idle['y']-origin['y'])*.66))
        for mode,start,playrate,part in [('idle',79,18,'all'),('body',79,18,'body'),('head',79,18,'head'),('fire',54,rate,'head'),('shoot',54,rate,'all')]:
            frames=[];count=math.ceil(24/playrate*60)+(mode in ('fire','shoot'))
            for n in range(count):
                canvas=Image.new('RGBA',(384,384));pos=min(24,n/60*playrate)
                for name,keys in tracks:
                    body=any(v in name.lower() for v in ('backleaf','frontleaf','stalk','stem'))
                    if (part=='body' and not body) or (part=='head' and body):continue
                    draw(canvas,sample(keys,(79 if body else start)+pos))
                frames.append(canvas)
            clips[mode]=frames
        union=Image.new('L',(384,384))
        for frames in clips.values():
            for f in frames:union=ImageChops.lighter(union,f.getchannel('A'))
        box=union.getbbox();box=(box[0]-2,box[1]-2,box[2]+2,box[3]+2);size=(round((box[2]-box[0])*.66),round((box[3]-box[1])*.66))
        for mode,frames in clips.items():save(prefix+'_'+mode,[f.crop(box).resize(size,Image.Resampling.LANCZOS) for f in frames],60)
    (destination/'shoot-alignment.bin').write_bytes(struct.pack('<4s3I',b'PPA1',3,80,60)+struct.pack('<480f',*alignment))
    # Decode first prepared plant frames to build the console's packet portraits.
    names=['pea_idle','sunflower','wallnut','cherry','mine','snow_idle','chomper','repeater_idle','puff','sunshroom','fume','gravebuster','hypno','scaredy','iceshroom','doom','lily','squash','threepeater','kelp','jalapeno','spikeweed','torchwood','tallnut','sea','plantern','cactus','blover','split','star','pumpkin','magnet','cabbage','pot','kernel','coffee','garlic','umbrella','marigold','melon','gatling','twin','gloom','cattail','winter','goldmagnet','spikerock','cob','imitater']
    def first(name):
        return first_prepared_frame(destination/(name+'.pva'))
    def font(name):
        txt=files['data/'+name.lower()+'.txt'].read_text(encoding='utf-8-sig')
        def block(k):return re.search(r'Define\s+'+k+r'\s*\((.*?)\);',txt,re.S).group(1)
        chars=[a or b for a,b in re.findall(r"'([^']*)'|\"([^\"]*)\"",block('CharList'))];widths=list(map(int,re.findall(r'-?\d+',block('WidthList'))));rects=[tuple(map(int,v)) for v in re.findall(r'\(\s*(-?\d+),\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)\)',block('RectList'))];key=next(k for k in files if k in ('data/'+name.lower()+'.png','data/'+name.lower()+'_.gif'));atlas=Image.open(files[key]);atlas=atlas.getchannel('A') if atlas.mode=='RGBA' else atlas.convert('L');return atlas,{ch:(widths[i],rects[i]) for i,ch in enumerate(chars)}
    fonts={n:font(n) for n in ('DwarvenTodcraft24','BrianneTod16')}
    def label(text,name,width,color,scale=1):
        atlas,glyphs=fonts[name];out=Image.new('L',(2400,100));x=2
        for ch in text:
            if ch==' ':x+=10;continue
            if ch not in glyphs:continue
            advance,(gx,gy,gw,gh)=glyphs[ch];out.paste(atlas.crop((gx,gy,gx+gw,gy+gh)),(x,2));x+=advance
        box=out.getbbox()
        if not box:return Image.new('RGBA',(1,1))
        out=out.crop((0,0,max(x,box[2]),box[3]+2));rgba=Image.new('RGBA',out.size,color);rgba.putalpha(out);factor=min(scale,width/rgba.width);return rgba.resize((max(1,round(rgba.width*factor)),max(1,round(rgba.height*factor))),Image.Resampling.LANCZOS)
    text=files['properties/lawnstrings.txt'].read_text(encoding='utf-16');strings={k:v.strip() for k,v in re.findall(r'\[([^\]]+)\]\s*([^\[]*)',text)}
    def clean(s):return re.sub(r'\{[^}]*\}','',s).replace('\r','').replace('\n',' ')
    packet=im('images/seeds.ptx').crop((0,0,50,70)).resize((100,140));awards=[];cards=[]
    costs=[100,50,50,150,25,175,150,200,0,25,75,75,75,25,75,125,25,50,325,25,125,100,175,125,0,25,125,100,125,125,125,100,100,25,100,75,50,100,50,300,250,150,150,225,200,50,125,500,0]
    for i,name in enumerate(names):
        card=packet.copy();portrait=first(name);portrait.thumbnail((86,88),Image.Resampling.LANCZOS);card.alpha_composite(portrait,((100-portrait.width)//2,23+(88-portrait.height)//2))
        heading=label(clean(strings[PLANT_KEYS[i]]).upper(),'DwarvenTodcraft24',90,(35,44,23,255),.26);card.alpha_composite(heading,((100-heading.width)//2,5))
        cost=label(str(costs[i]),'DwarvenTodcraft24',52,(32,30,14,255),.72);card.alpha_composite(cost,((100-cost.width)//2,116));cards.append(card.resize((32,43),Image.Resampling.LANCZOS))
        out=im('images/awardscreen_back.ptx');title=label(clean(strings.get('NEW_PLANT','You got a new plant!')),'DwarvenTodcraft24',620,(248,211,67,255));out.alpha_composite(title,((1280-title.width)//2,87));out.alpha_composite(card.resize((116,162),Image.Resampling.LANCZOS),(582,183))
        title=label(clean(strings[PLANT_KEYS[i]]),'DwarvenTodcraft24',280,(255,216,72,255),.85);out.alpha_composite(title,((1280-title.width)//2,363))
        words=clean(strings.get(PLANT_KEYS[i]+'_TOOLTIP','')).split();lines=[];line=''
        for word in words:
            new=(line+' '+word).strip()
            if label(new,'BrianneTod16',2000,(40,46,65,255)).width>262 and line:lines.append(line);line=word
            else:line=new
        if line:lines.append(line)
        for j,line in enumerate(lines[:4]):
            txt=label(line,'BrianneTod16',260,(40,46,65,255));out.alpha_composite(txt,((1280-txt.width)//2,421+j*26))
        awards.append(out.resize((480,272),Image.Resampling.LANCZOS))
    save('seed_cards',cards);save('plant_awards',awards)
    # No generated substitute artwork: the disc provides these backgrounds.
    for name,key in [('mini_background','challenge_background'),('aquarium_background','aquarium1'),('slot_overlay','slotmachine_overlay')]:
        image=im('images/'+key+'.ptx');image=image.resize((480,272),Image.Resampling.LANCZOS) if name!='slot_overlay' else image.resize((150,42),Image.Resampling.LANCZOS);save(name,[image])
    # Sunbeam transparency around the award pickup, evaluated cheaply by GU.
    rays=Image.new('RGBA',(192,192));d=ImageDraw.Draw(rays)
    for n in range(12):
        a=n*math.tau/12;d.polygon([(96,96),(96+94*math.cos(a),96+94*math.sin(a)),(96+94*math.cos(a+.16),96+94*math.sin(a+.16))],fill=(255,244,140,130))
    save('award_rays',[rays])
    brain=im('images/brain.ptx');brain.thumbnail((20,18));save('brain',[brain])
    for name,source in [('normal','ZombieHead'),('pole','ZombiePoleVaulterHead'),('football','ZombieFootballHead'),('dancer','ZombieDancerHead_disco'),('backup','ZombieBackupDancerHead_disco')]:
        head=im('particles/'+source+'.ptx')
        head=head.resize((max(1,round(head.width*.48)),max(1,round(head.height*.48))),Image.Resampling.LANCZOS)
        save('head_'+name,[head])
    return result

if __name__=='__main__':
    import sys,json
    from prepare_assets import write_pva
    root=Path(__file__).resolve().parents[1];destination=root/'private/update-0.9-data';preview=root/'private/review09';preview.mkdir(exist_ok=True)
    new=prepare_features09(root.parent/'private-pvz-import',destination,preview,write_pva);manifest=json.loads((destination/'manifest.json').read_text());replace={a['file'] for a in new};manifest['assets']=[a for a in manifest['assets'] if a['file'] not in replace]+new;manifest['texture_bytes']=sum(a.get('decoded_bytes',0) for a in manifest['assets']);(destination/'manifest.json').write_text(json.dumps(manifest,indent=2));print('Prepared',len(new),'feature assets')
