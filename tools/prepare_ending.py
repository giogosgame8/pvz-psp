"""Compose the owner's original layered ending into an indexed PSP movie."""
from pathlib import Path
import argparse,io,json,math,struct,time,zlib,hashlib
from PIL import Image
from reanim_scene import Scene,multiply

def extract_timing(elf,dest):
 data=Path(elf).read_bytes();needle=struct.pack('>f',128.5);offset=0
 while (offset:=data.find(needle,offset))>=0:
  if data[offset+16:offset+20]==struct.pack('>f',133.0) and data[offset+32:offset+36]==struct.pack('>f',136.5):
   rows=[];last=0
   for i in range(600):
    if offset+i*16+16>len(data):break
    frame,word,x,brain=struct.unpack_from('>f3I',data,offset+i*16)
    if not(last<frame<2000 and word<5 and x<1500 and brain<6):break
    rows.append([frame,word,x,brain]);last=frame
   if len(rows)>200:
    path=dest/'credits-timing.json';path.write_text(json.dumps({'source':'Personal PS3 executable','rows':rows}));return path
  offset+=1
 raise ValueError('Original ending timing table was not found in the supplied executable')

def prepare_ending(imported,dest,previews=None,fps=24,timing=None):
 scene=Scene(imported)
 if timing and timing.exists():
  rows=json.loads(timing.read_text())['rows'];scene.timings=[(r[0],['AA','EE','AW','OH','OFF'][r[1]],r[2],r[3]) for r in rows]
 parts=[]
 for name,offset in [('Credits_Main',2),('Credits_Main2',400),('Credits_Main3',785)]:
  rate,tracks=scene.animation(name);length=max(len(v) for _,v in tracks);parts.append((name,rate,length,offset,math.ceil((length-1)/rate*fps)))
 total=sum(p[4] for p in parts);path=dest/'ending.pvj';index=[];maximum=0;start=time.monotonic();n=0
 with path.open('wb') as out:
  out.write(struct.pack('<4s7I',b'PVJ1',480,272,total,fps,1,0,1));out.write(bytes(total*12))
  for name,rate,length,offset,count in parts:
   for local in range(count):
    pos=min(length-1,local/fps*rate);scene.time=(offset+pos)/7
    im=Image.new('RGBA',(480,272),(0,0,0,255));scene.draw(im,name,pos,(.6,0,0,272/600,0,0))
    # The original lyric marker bounces along the words using the disc timing.
    if scene.timings:
     import bisect
     idx=bisect.bisect_right([t[0] for t in scene.timings],offset+pos)-1
     if 0<=idx<len(scene.timings)-1:
      before=scene.timings[idx];after=scene.timings[idx+1];t=(offset+pos-before[0])/max(.001,after[0]-before[0]);ease=t*t*(3-2*t);kind=after[3]
      if kind in (0,1):x=(850 if kind==0 else after[2]-50)+(after[2]-15-(850 if kind==0 else after[2]-50))*ease;y=485+20*ease
      elif kind in (3,4):x=before[2]-15+((850 if kind==4 else before[2]+25)-(before[2]-15))*ease;y=505-20*math.sin(t*math.pi)
      elif before[3] in (0,1,2):x=before[2]-15+(after[2]-before[2])*ease;y=505-20*math.sin(t*math.pi)
      else:x=y=-1000
      brain=scene.texture('IMAGE_REANIM_BRAIN');scene.paint(im,brain,(.6,0,0,272/600,x*.6,y*272/600),1)
    data=io.BytesIO();im.convert('RGB').save(data,'JPEG',quality=75,subsampling=2,optimize=False);raw=data.getvalue();index.append((out.tell(),len(raw),zlib.crc32(raw)));out.write(raw);maximum=max(maximum,len(raw))
    if previews and n%max(1,fps*15)==0:im.save(previews/f'ending-{n:05}.png')
    n+=1
    if n%fps==0:
     progress={'frames':n,'total':total,'seconds':round(time.monotonic()-start,1),'part':name,'source_frame':round(pos,2)};(dest/'ending-progress.json').write_text(json.dumps(progress));print(progress,flush=True)
  out.seek(0);out.write(struct.pack('<4s7I',b'PVJ1',480,272,total,fps,1,maximum,1))
  for record in index:out.write(struct.pack('<3I',*record))
 meta={'file':path.name,'codec':'indexed baseline JPEG 4:2:0','width':480,'height':272,'fps':fps,'frames':total,'duration':total/fps,'audio':'music12.wav','max_frame_bytes':maximum,'bytes':path.stat().st_size,'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'missing_resources':sorted(scene.missing),'source':'Original disc reanimations, textures and text; private preparation','lip_sync_from_disc':bool(scene.timings)}
 (dest/'ending.json').write_text(json.dumps(meta,indent=2));return meta

if __name__=='__main__':
 parser=argparse.ArgumentParser();parser.add_argument('--imported-dir',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--timing',type=Path);parser.add_argument('--fps',type=int,default=24);parser.add_argument('--previews',type=Path);args=parser.parse_args()
 if args.previews:args.previews.mkdir(exist_ok=True,parents=True)
 print(json.dumps(prepare_ending(args.imported_dir,args.output,args.previews,args.fps,args.timing),indent=2))
