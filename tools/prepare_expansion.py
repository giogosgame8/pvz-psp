"""Prepare private garden, dialogue and story resources from the supplied disc."""
from pathlib import Path
import json,re,struct,sys,shutil
import xml.etree.ElementTree as ET
from PIL import Image

def garden_need_frames(files):
    """Original need cels composed in the original speech bubble."""
    # The disc manifest has unrelated non-XML tokens; parse its image tag alone.
    manifest=files['properties/resources.xml'].read_text(encoding='utf-8-sig')
    tag=re.search(r'<Image\b[^>]*\bid="ZEN_NEED_ICONS"[^>]*/>',manifest)
    if not tag:raise ValueError('Garden need atlas definition is missing')
    definition=ET.fromstring(tag.group(0))
    columns=int(definition.get('cols','1'));rows=int(definition.get('rows','1'))
    icons=Image.open(files['images/zen_need_icons.ptx']).convert('RGBA')
    if columns!=3 or rows!=1 or icons.width%columns:raise ValueError('Unexpected garden need atlas layout')
    width=icons.width//columns;bubble=Image.open(files['images/plantspeechbubble.ptx']).convert('RGBA')
    frames=[]
    for need in range(4):
        frame=bubble.copy()
        if need==0:art=Image.open(files['images/waterdrop.ptx']).convert('RGBA');position=(17,9)
        else:art=icons.crop(((need-1)*width,0,need*width,icons.height));position=(10 if need==3 else 11,7)
        frame.alpha_composite(art,position);frames.append(frame)
    return frames

def hash_key(key):
    n=2166136261
    for b in key.encode('utf-8'):n=((n^b)*16777619)&0xffffffff
    return n

def prepare_expansion(imported,dest,previews,write_pva):
    files={p.relative_to(imported/'archives'/a).as_posix().lower():p for a in ('main','compressed','en') for p in (imported/'archives'/a).rglob('*') if p.is_file()};assets=[]
    text=files['properties/lawnstrings.txt'].read_text(encoding='utf-16');entries={k:v.strip() for k,v in re.findall(r'\[([^\]]+)\]\s*([^\[]*)',text)}
    def clean(value):
        value=re.sub(r'\{[^}]*\}','',value).replace('<X>','(X)').replace('<l>','analog').replace('<r>','buttons');return value.replace('\r','').strip()
    def save(name,frames,fps=1):
        assets.append(write_pva(dest/(name+'.pva'),frames,fps))
        if previews:frames[0].save(previews/(name+'.png'))
    for name,key in [('garden_background','background_greenhouse'),('mushroom_garden','background_mushroomgarden')]:
        im=Image.open(files['images/'+key+'.ptx']).convert('RGBA');save(name,[im.resize((480,272),Image.Resampling.LANCZOS)])
    save('garden_needs',garden_need_frames(files))
    # Original Brianne atlas, preserving sentence case and punctuation.
    font=files['data/briannetod16.txt'].read_text(encoding='utf-8-sig')
    def block(k):return re.search(r'Define\s+'+k+r'\s*\((.*?)\);',font,re.S).group(1)
    chars=[a or b for a,b in re.findall(r"'([^']*)'|\"([^\"]*)\"",block('CharList'))];widths=list(map(int,re.findall(r'-?\d+',block('WidthList'))));rects=[tuple(map(int,v)) for v in re.findall(r'\(\s*(-?\d+),\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)\)',block('RectList'))];glyphs={c:(widths[i],rects[i]) for i,c in enumerate(chars)};atlas=Image.open(files['data/briannetod16_.gif']).convert('L')
    def card(text,width=300,height=133):
        scale=1.15;wrap=round((width-26)/scale);lines=[]
        for para in text.split('\n'):
            line=''
            for word in para.split():
                attempt=(line+' '+word).strip();length=sum(glyphs[c][0] if c in glyphs else 8 for c in attempt)
                if length>wrap and line:lines.append(line);line=word
                else:line=attempt
            lines.append(line)
        if len(lines)>6:scale=min(scale,(height-24)/(len(lines)*22));wrap=round((width-26)/scale);lines=[];line=''
        if not lines:
            for word in text.split():
                attempt=(line+' '+word).strip()
                if sum(glyphs[c][0] if c in glyphs else 8 for c in attempt)>wrap and line:lines.append(line);line=word
                else:line=attempt
            lines.append(line)
        mask=Image.new('L',(round(width/scale),max(round(height/scale),len(lines)*24+10)));y=6
        for line in lines:
            x=10
            for c in line:
                if c not in glyphs:x+=8;continue
                advance,(gx,gy,gw,gh)=glyphs[c];mask.paste(atlas.crop((gx,gy,gx+gw,gy+gh)),(x,y));x+=advance
            y+=24
        glyph=Image.new('RGBA',mask.size,(38,31,22,255));glyph.putalpha(mask);glyph=glyph.resize((round(glyph.width*scale),round(glyph.height*scale)),Image.Resampling.LANCZOS)
        out=Image.new('RGBA',(width,height),(244,231,194,255));out.alpha_composite(glyph,(0,0));return out
    dave=sorted([k for k in entries if k.startswith('CRAZY_DAVE_')]);cards=[]
    for i,k in enumerate(dave):cards.append(card(clean(entries[k])));entries['FRAME_'+k]=str(i)
    save('dialogue_cards',cards)
    tree=sorted([k for k in entries if k.startswith('TREE_OF_WISDOM_') and k.removeprefix('TREE_OF_WISDOM_').isdigit()]);cards=[]
    for i,k in enumerate(tree):cards.append(card(clean(entries[k]),330,112));entries['FRAME_'+k]=str(i)
    save('wisdom_cards',cards)
    # Indexed original text also serves tooltips and story command metadata.
    rows=[]
    for key,value in entries.items():
        a=key.encode('utf-8');b=value.encode('utf-8');rows.append((hash_key(key),struct.pack('<II',len(a),len(b))+a+b))
    rows.sort(key=lambda r:r[0]);offset=8+8*len(rows);index=[]
    for h,payload in rows:index.append((h,offset));offset+=len(payload)
    with (dest/'strings.bin').open('wb') as f:
        f.write(struct.pack('<4sI',b'PVS2',len(rows)))
        for row in index:f.write(struct.pack('<II',*row))
        for _,payload in rows:f.write(payload)
    (dest/'story-index.json').write_text(json.dumps({'dave_keys':dave,'tree_keys':tree,'strings':len(rows),'dialogue_frames':len(dave)},indent=2))
    return assets

if __name__=='__main__':
    sys.path.insert(0,str(Path(__file__).parent));import prepare_assets as a
    root=Path(__file__).resolve().parents[1];imported=root.parent/'private-pvz-import';dest=root/'private/update-0.10-data';preview=root/'private/review010';preview.mkdir(exist_ok=True)
    assets=prepare_expansion(imported,dest,preview,a.write_pva)
    for source in ['CrazyDave','Stinky','ZenGarden_sprout','ZenGarden_wateringcan','treeofWisdom']:
        fps,tracks=a.frames_from_xml(imported/'archives/compressed/reanim'/(source+'.reanim'));print(source,[(n,a.clip_range(tracks,n)) for n,f in tracks if n.startswith('anim_')])
    a.SPECS=[('dave_idle','CrazyDave','anim_idle',.32,''),('dave_talk','CrazyDave','anim_mediumtalk',.32,''),('dave_enter','CrazyDave','anim_enter',.32,''),('dave_leave','CrazyDave','anim_leave',.32,''),('garden_sprout','ZenGarden_sprout','anim_idle',.55,''),('snail_walk','Stinky','anim_crawl',.65,'')]
    temp=root/'private/expansion010-bake-v2';new=a.prepare(imported,temp,None,animations_only=True)
    for item in new:shutil.copy2(temp/item['file'],dest/item['file'])
    assets+=new;manifest=json.loads((dest/'manifest.json').read_text());changed={r['file'] for r in assets};manifest['assets']=[r for r in manifest['assets'] if r['file'] not in changed]+assets;manifest['texture_bytes']=sum(r.get('decoded_bytes',0) for r in manifest['assets']);(dest/'manifest.json').write_text(json.dumps(manifest,indent=2));print('Prepared',len(assets),'expansion assets')
