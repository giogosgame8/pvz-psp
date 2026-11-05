"""Compose the owner's original layered ending into an indexed PSP movie."""
from pathlib import Path
import argparse,io,json,math,struct,time,zlib,hashlib
from PIL import Image
from reanim_scene import Scene,multiply
import subprocess,sys,tempfile
from pc_runtime import hidden_process_options,ending_command
from preparation_workers import worker_count

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

def render_ending_frame(scene,name,rate,length,offset,local,fps):
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
 return im


def initialize_ending_worker(imported,timings):
 global _ending_scene
 _ending_scene=Scene(Path(imported));_ending_scene.timings=timings


def render_ending_chunk(job):
 name,rate,length,offset,start,stop,fps,global_start,previews=job
 frames=[]
 for local in range(start,stop):
  im=render_ending_frame(_ending_scene,name,rate,length,offset,local,fps)
  data=io.BytesIO();im.convert('RGB').save(data,'JPEG',quality=75,subsampling=2,optimize=False)
  frames.append(data.getvalue())
  n=global_start+local
  if previews and n%max(1,fps*15)==0:im.save(Path(previews)/f'ending-{n:05}.png')
 return frames,tuple(_ending_scene.missing)


def ending_file_access(operation):
 # Windows may briefly keep a newly published file open (for example, during
 # indexing). Retry only sharing/permission failures; never accept partial data.
 for attempt in range(40):
  try:return operation()
  except PermissionError:
   if attempt==39:raise
   time.sleep(.025)


def ending_worker_main(configuration):
 config=json.loads(Path(configuration).read_text(encoding='utf-8'))
 folder=Path(config['folder']);initialize_ending_worker(config['imported'],config['timings'])
 assigned=config['assigned']
 for position,(index,job) in enumerate(assigned):
  if position>=2:
   previous=folder/f'{assigned[position-2][0]}.json';deadline=time.monotonic()+300
   while previous.exists():
    if time.monotonic()>deadline or not folder.exists():raise ValueError('Ending writer stopped responding')
    time.sleep(.03)
  frames,missing=render_ending_chunk(job)
  with (folder/f'{index}.bin').open('wb') as out:
   for frame in frames:out.write(struct.pack('<I',len(frame)));out.write(frame)
  # Publish a complete chunk atomically; the parent never reads partial JPEGs.
  temporary=folder/f'{index}.pending'
  temporary.write_text(json.dumps({'frames':len(frames),'missing':missing}),encoding='utf-8')
  ending_file_access(lambda:temporary.replace(folder/f'{index}.json'))


def ending_chunks(jobs,workers,imported,timings):
 if workers==1:
  initialize_ending_worker(imported,timings)
  for job in jobs:yield render_ending_chunk(job)
  return
 # Each hidden worker owns its scene clock and caches. At most two chunks per
 # worker can wait for the writer; files are consumed in the original order.
 with tempfile.TemporaryDirectory(prefix='pvz-ending-') as temporary:
  folder=Path(temporary);processes=[];logs=[];consumed=0
  try:
   for index in range(workers):
    configuration=folder/f'worker-{index}.json'
    configuration.write_text(json.dumps({'folder':str(folder),'imported':str(imported),'timings':timings,
                                         'assigned':list(enumerate(jobs))[index::workers]}),encoding='utf-8')
    log=(folder/f'worker-{index}.log').open('wb');logs.append(log)
    processes.append(subprocess.Popen(ending_command(configuration),stdout=log,stderr=subprocess.STDOUT,
                                      **hidden_process_options()))
   for index in range(len(jobs)):
    report=folder/f'{index}.json';process=processes[index%workers]
    while not report.exists():
     if process.poll() is not None:
      details=(folder/f'worker-{index%workers}.log').read_text(encoding='utf-8',errors='replace')
      raise ValueError('Ending render worker stopped before finishing. '+details[-2000:])
     time.sleep(.03)
    meta=json.loads(ending_file_access(lambda:report.read_text(encoding='utf-8')));frames=[];file=folder/f'{index}.bin'
    with ending_file_access(lambda:file.open('rb')) as source:
     for _ in range(meta['frames']):
      length=struct.unpack('<I',source.read(4))[0];data=source.read(length)
      if len(data)!=length:raise ValueError('Incomplete ending frame')
      frames.append(data)
    ending_file_access(file.unlink);ending_file_access(report.unlink);consumed+=1
    yield frames,tuple(meta['missing'])
  finally:
   errors=[]
   try:
    for process in processes:
     if consumed<len(jobs) and process.poll() is None:process.terminate()
    for process in processes:
     try:code=process.wait(timeout=10)
     except subprocess.TimeoutExpired:
      process.kill();process.wait();errors.append('Ending render worker did not exit');continue
     if consumed==len(jobs) and code:errors.append('Ending render worker failed')
   finally:
    for log in logs:log.close()
   if errors:raise ValueError('; '.join(errors))



def prepare_ending(imported,dest,previews=None,fps=24,timing=None,jobs=None):
 scene=Scene(imported)
 if timing and timing.exists():
  rows=json.loads(timing.read_text())['rows'];scene.timings=[(r[0],['AA','EE','AW','OH','OFF'][r[1]],r[2],r[3]) for r in rows]
 tasks=[];global_start=0
 for name,offset in [('Credits_Main',2),('Credits_Main2',400),('Credits_Main3',785)]:
  rate,tracks=scene.animation(name);length=max(len(v) for _,v in tracks);count=math.ceil((length-1)/rate*fps)
  for start in range(0,count,48):tasks.append((name,rate,length,offset,start,min(count,start+48),fps,global_start,str(previews) if previews else None))
  global_start+=count
 total=global_start;path=dest/'ending.pvj';index=[];maximum=0;start_time=time.monotonic();n=0;missing=set(scene.missing)
 workers=worker_count('ending',jobs)
 print(f'Ending render workers: {workers}',flush=True)
 results=ending_chunks(tasks,workers,imported,scene.timings)
 from contextlib import closing
 with closing(results), path.open('wb') as out:
  out.write(struct.pack('<4s7I',b'PVJ1',480,272,total,fps,1,0,1));out.write(bytes(total*12))
  for task,(frames,unavailable) in zip(tasks,results):
   missing.update(unavailable);name,rate,length,offset,local_start,_,_,_,_=task
   for local,raw in enumerate(frames,local_start):
    index.append((out.tell(),len(raw),zlib.crc32(raw)));out.write(raw);maximum=max(maximum,len(raw));n+=1
    if n%fps==0 or n==total:
     pos=min(length-1,local/fps*rate)
     progress={'frames':n,'total':total,'seconds':round(time.monotonic()-start_time,1),'part':name,'source_frame':round(pos,2)};(dest/'ending-progress.json').write_text(json.dumps(progress));print(progress,flush=True)
  out.seek(0);out.write(struct.pack('<4s7I',b'PVJ1',480,272,total,fps,1,maximum,1))
  for record in index:out.write(struct.pack('<3I',*record))
 meta={'file':path.name,'codec':'indexed baseline JPEG 4:2:0','width':480,'height':272,'fps':fps,'frames':total,'duration':total/fps,'audio':'music12.wav','max_frame_bytes':maximum,'bytes':path.stat().st_size,'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'missing_resources':sorted(missing),'source':'Original disc reanimations, textures and text; private preparation','lip_sync_from_disc':bool(scene.timings)}
 (dest/'ending.json').write_text(json.dumps(meta,indent=2));return meta

def main():
 parser=argparse.ArgumentParser();parser.add_argument('--imported-dir',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--timing',type=Path);parser.add_argument('--fps',type=int,default=24);parser.add_argument('--previews',type=Path);parser.add_argument('--jobs',type=int,help='Ending render processes; default up to 8, limited by CPU and memory');args=parser.parse_args()
 if args.previews:args.previews.mkdir(exist_ok=True,parents=True)
 print(json.dumps(prepare_ending(args.imported_dir,args.output,args.previews,args.fps,args.timing,args.jobs),indent=2))


if __name__=='__main__':
 if len(sys.argv)>1 and sys.argv[1]=='--ending-worker':ending_worker_main(sys.argv[2])
 else:main()
