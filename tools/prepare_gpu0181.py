"""Convert existing private PVA3 textures to lossless GPU-ready PVA4."""
from pathlib import Path
import argparse,hashlib,json,shutil,struct,zlib
from texture_layout import tile_rgba4444,untile_rgba4444

def prepare(source,output):
 if output.exists():raise ValueError('Choose a new output directory')
 output.mkdir(parents=True);manifest=json.loads((source/'manifest.json').read_text());assets=[];frames=0
 for file in source.iterdir():
  if file.is_file() and file.suffix!='.pva' and file.name!='manifest.json':shutil.copy2(file,output/file.name)
 for asset in manifest['assets']:
  file=source/asset['file'];raw=file.read_bytes();magic,w,h,tw,th,n,stride,fps=struct.unpack_from('<4s6If',raw);assert magic==b'PVA3';header=40+n*8;payload=[]
  for i in range(n):
   at,size=struct.unpack_from('<II',raw,40+i*8);linear=zlib.decompress(raw[header+at:header+at+size]);assert len(linear)==stride
   tiled=tile_rgba4444(linear,tw,th);assert untile_rgba4444(tiled,tw,th)==linear;payload.append(zlib.compress(tiled,9));frames+=1
  dest=output/file.name
  with dest.open('wb') as f:
   f.write(b'PVA4'+raw[4:40]);at=0
   for p in payload:f.write(struct.pack('<II',at,len(p)));at+=len(p)
   for p in payload:f.write(p)
  entry=dict(asset);entry['bytes']=dest.stat().st_size;entry['sha256']=hashlib.sha256(dest.read_bytes()).hexdigest();assets.append(entry);print(f"Converted {len(assets)}/{len(manifest['assets'])} {file.name}",flush=True)
 manifest.update(version='0.18.1',format='PVA4 zlib GPU-tiled RGBA4444',assets=assets)
 (output/'manifest.json').write_text(json.dumps(manifest,indent=2))
 result={'assets':len(assets),'frames':frames,'all_pixels_round_trip_identical':True,'source':str(source),'output':str(output),'texture_bytes':manifest['texture_bytes']};(output/'layout-validation.json').write_text(json.dumps(result,indent=2));print(json.dumps(result),flush=True)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();prepare(a.source,a.output)
