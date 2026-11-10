"""Keep original menu layers in painter order, with compact GPU motion tracks."""
from functools import lru_cache
import json
import math
import struct
import zlib
from PIL import Image
from affine_render import AffineLayerCache,composite_affine


def extras_group(name):
    name=name.strip().lower()
    if name in ('packets','bonsai','envelopes'):return {'packets':2,'bonsai':3,'envelopes':4}[name]
    for index,prefix in enumerate(('shop','almanac','zengarden','mail'),1):
        if name.startswith(prefix):return index
    return 0


def prepare_menu_scene(imported,destination,write_pva):
    from prepare_assets import frames_from_xml,clip_range
    files={p.relative_to(imported/'archives'/a).as_posix().lower():p for a in ('main','compressed','en') for p in (imported/'archives'/a).rglob('*') if p.is_file()}
    fps,tracks=frames_from_xml(files['reanim/mainmenu3/mainmenu.reanim'])
    selectors={'adventure','start adventure','more ways','more','help','puzzle button','minigames button','survival button','back stone button','shop button','almanac button','zengardenbutton','mail button','back ball button'}
    states={}
    for base in selectors:
        stem=base.removesuffix(' button') if base!='back ball button' else base
        for suffix in ('',' selected',' pressed'):states[base if not suffix else stem+suffix]=(base,suffix)
    settings=[[(s,a) for a in ('adventure','start adventure') for s in (a,'more ways','more','help','player name')],[(s,'adventure') for s in ('minigames button','puzzle button','survival button','back stone button')],[(s,'adventure') for s in ('shop button','almanac button','zengardenbutton','mail button','back ball button')]]
    def included(name,selected,adventure):
        if any(word in name for word in ('leaderboard','mail alert','shocked crow')) or name=='crow2':return False
        if name in ('vs button','vs selected','vs pressed','vs coop','vs coop selected','vs coop pressed'):return False
        if name.endswith((' selected',' pressed')) and name not in states:return False
        if name in states:
            base,suffix=states[name]
            if base in ('adventure','start adventure') and base!=adventure:return False
            if suffix!=(' selected' if base==selected else ''):return False
        return True
    @lru_cache(maxsize=256)
    def texture(identifier):
        key=identifier.lower().removeprefix('image_reanim_layer_').removeprefix('image_reanim_').removesuffix('.png').strip()
        path=files.get('reanim/mainmenu3/'+key+'.ptx') or files.get('reanim/'+key+'.ptx')
        if path:return Image.open(path).convert('RGBA')
        return None
    def matrix(f,offset):
        kx,ky=map(math.radians,(f['kx'],f['ky']));a,b=math.cos(kx)*f['sx'],-math.sin(ky)*f['sy'];c,d=math.sin(kx)*f['sx'],math.cos(ky)*f['sy']
        return a,b,c,d,f['x']-offset,f['y']
    assets=[];records=[];details=[];cache=AffineLayerCache()
    def emit(page,group,mask,frames,rate,image,poses,name):
        if len(records)>=192:raise ValueError('Too many menu layers')
        index=len(records);asset=write_pva(destination/f'menu_object{index:03}.pva',[image],1,(0,0));assets.append(asset)
        records.append(struct.pack('<4If',page,group,mask,frames,rate)+b''.join(struct.pack('<7f',*pose) for pose in poses))
        details.append({'index':index,'page':page,'group':group,'mask':mask,'keys':frames,'track':name})
    for page,(offset,marker) in enumerate(((1360,'anim_idle'),(0,'anim_idle_left'),(2560,'anim_idle_right'))):
        start,stop=clip_range(tracks,marker);run=[];run_kind=None
        def flush():
            nonlocal run,run_kind
            if not run:return
            canvas=Image.new('RGBA',(1280,720))
            for name,f,image in run:
                if f['f']<0 or f['a']<=0:continue
                m=matrix(f,offset);a,b,c,d,x,y=m;det=a*d-b*c
                if abs(det)<1e-8:continue
                inverse=(d/det,-b/det,(b*y-d*x)/det,-c/det,a/det,(c*x-a*y)/det)
                composite_affine(canvas,image,m,inverse,f['a'],cache)
            frame=canvas.resize((480,272),Image.Resampling.LANCZOS);box=frame.getbbox()
            if box:
                emit(page,*run_kind,1,1,frame.crop(box),[(1,0,0,1,box[0],box[1],1)],' / '.join(name for name,_,_ in run))
            run=[];run_kind=None
        for raw_name,keys in tracks:
            name=raw_name.strip().lower()
            if name.startswith('anim_') or name.startswith('_'):continue
            mask=sum(1<<n for n,(selected,adventure) in enumerate(settings[page]) if included(name,selected,adventure))
            if not mask:continue
            group=extras_group(name) if page==2 else 0
            active=[f for f in keys[start:stop] if f['f']>=0 and f['a']>0 and f['i']]
            first=keys[0];identifier=first['i'] if first['i'] else active[0]['i'] if active else ''
            image=texture(identifier) if identifier else None
            if image is None:continue
            animated=len({(f['x'],f['y'],f['kx'],f['ky'],f['sx'],f['sy'],f['a'],f['f']) for f in keys[start:stop]})>1
            animated=animated and active and len({f['i'] for f in active})==1
            if animated:image=texture(active[0]['i']) or image
            box=image.getbbox()
            if animated and box:
                left,top,right,bottom=box;width,height=right-left,bottom-top
                w,h=max(1,round(width*.375)),max(1,round(height*272/720));poses=[];visible=False
                if max(w,h)>512:animated=False
                else:
                    for f in keys[start:stop]:
                        a,b,c,d,x,y=matrix(f,offset);x+=a*left+b*top;y+=c*left+d*top
                        pose=(a*width/w*.375,b*height/h*.375,c*width/w*272/720,d*height/h*272/720,x*.375,y*272/720,f['a'] if f['f']>=0 and f['i'] else 0)
                        poses.append(pose)
                        corners=[(pose[4]+pose[0]*u+pose[1]*v,pose[5]+pose[2]*u+pose[3]*v) for u,v in ((0,0),(w,0),(0,h),(w,h))]
                        visible|=pose[6]>0 and max(v[0] for v in corners)>0 and min(v[0] for v in corners)<480 and max(v[1] for v in corners)>0 and min(v[1] for v in corners)<272
                    if visible:
                        flush();emit(page,group,mask,len(poses),fps,image.crop(box).resize((w,h),Image.Resampling.LANCZOS),poses,name)
                        continue
                    # No part of this track reaches the current page.
                    continue
            if first['f']<0 or first['a']<=0:continue
            if run_kind!=(group,mask):flush();run_kind=(group,mask)
            run.append((name,first,image))
        flush()
    payload=b''.join(records)
    (destination/'menu-motion.bin').write_bytes(struct.pack('<4s3I',b'PVMU',1,len(records),zlib.crc32(payload))+payload)
    (destination/'menu-motion.json').write_text(json.dumps({'schema':1,'layers':details,'method':'Original painter order; static runs and interpolated GPU transforms'},indent=2),encoding='utf-8')
    return assets
