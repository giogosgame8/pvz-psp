"""Prepare private PS3 resources for the native PSP prototype. No assets bundled.

Requires Pillow. The ISO is read-only; imported/prepared files stay on this PC.
Animation transforms are evaluated offline to reduce PSP memory and CPU cost.
"""
from __future__ import annotations

import argparse
import array
import hashlib
import json
import math
import shutil
import struct
import subprocess
import sys
import tempfile
import zlib
import xml.etree.ElementTree as ET
from functools import lru_cache
from pathlib import Path

from PIL import Image, ImageChops
from affine_render import composite_affine, render_frames

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "importer"))
from pvz_import import inspect_or_import

# name, PS3 animation, sequence, scale, optional zombie equipment
SPECS = [
    ("pea_idle", "PeaShooterSingle", "anim_full_idle", .66, ""),
    ("pea_shoot", "PeaShooterSingle", "anim_shooting", .66, ""),
    ("repeater_idle", "PeaShooter", "anim_full_idle", .66, ""),
    ("repeater_shoot", "PeaShooter", "anim_shooting", .66, ""),
    ("sunflower", "SunFlower", "anim_idle", .66, ""),
    ("wallnut", "Wallnut", "anim_idle", .66, ""),
    ("cherry", "CherryBomb", "anim_idle", .55, ""),
    ("cherry_explode", "CherryBomb", "anim_explode", .55, ""),
    ("mine", "PotatoMine", "anim_armed", .65, ""),
    ("mine_unarmed", "PotatoMine", (0, 1), .65, ""),
    ("snow_idle", "SnowPea", "anim_full_idle", .66, ""),
    ("snow_shoot", "SnowPea", "anim_shooting", .66, ""),
    ("chomper", "Chomper", "anim_idle", .56, ""),
    ("chomper_bite", "Chomper", "anim_bite", .56, ""),
    ("chomper_chew", "Chomper", "anim_chew", .56, ""),
    ("zombie_walk", "Zombie", "anim_walk", .48, ""),
    ("zombie_eat", "Zombie", "anim_eat", .48, ""),
    ("zombie_death", "Zombie", "anim_death", .48, ""),
    ("cone_walk", "Zombie", "anim_walk", .48, "cone"),
    ("cone_eat", "Zombie", "anim_eat", .48, "cone"),
    ("bucket_walk", "Zombie", "anim_walk", .48, "bucket"),
    ("bucket_eat", "Zombie", "anim_eat", .48, "bucket"),
    ("flag_walk", "Zombie", "anim_walk", .48, "flag"),
    ("flag_eat", "Zombie", "anim_eat", .48, "flag"),
    ("pole_run", "Zombie_polevaulter", "anim_run", .44, ""),
    ("pole_jump", "Zombie_polevaulter", "anim_jump", .44, ""),
    ("pole_walk", "Zombie_polevaulter", "anim_walk", .44, ""),
    ("pole_eat", "Zombie_polevaulter", "anim_eat", .44, ""),
    ("pole_death", "Zombie_polevaulter", "anim_death", .44, ""),
    ("mower", "LawnMower", "anim_normal", .44, ""),
    ("sun", "Sun", None, .38, ""),
]

# Bake original damaged armor and body tracks offline. Reuse each intact clip's
# crop/scale so a lost head or chipped helmet cannot shift the zombie's feet.
for base in tuple(SPECS):
    name,animation,clip,scale,gear=base
    stages=(1,2) if name in ("zombie_walk","zombie_eat","cone_walk","cone_eat","bucket_walk","bucket_eat","pole_walk","wallnut") else (1,) if name in ("pole_run","pole_jump","pole_eat","flag_walk","flag_eat") else (2,) if name in ("zombie_death","pole_death") else ()
    for stage in stages:
        armored=name.startswith(("cone_","bucket_"))
        SPECS.append((name+f"_damage{stage}",animation,clip,scale,
                      dict(gear=gear,armor=stage if armored else 0,body=0 if armored else stage,base=name)))


from prepare_story import NIGHT_SPECS
SPECS.extend(NIGHT_SPECS)
from prepare_chapters import CHAPTER_SPECS
SPECS.extend(CHAPTER_SPECS)
from prepare_upgrades import UPGRADE_SPECS
SPECS.extend(UPGRADE_SPECS)
SPECS.extend([('portal_circle','Portal_Circle','anim_pulse',.40,''),('portal_square','Portal_Square','anim_pulse',.40,''),('aquarium_swim','Zombie_snorkle','anim_aquarium_swim',.48,''),('bot_walk','Zombie','anim_walk',.48,'planthead'),('bot_eat','Zombie','anim_eat',.48,'planthead'),('heavy_tank','tank','anim_drive',.32,''),('heavy_turret','tank_turret','anim_idle',.32,'')])

from prepare_features010 import EXPANSION_SPECS
SPECS.extend(EXPANSION_SPECS)

def frames_from_xml(path):
    root = ET.fromstring("<reanim>" + path.read_text(encoding="utf-8-sig") + "</reanim>")
    tracks = []
    for track in root.findall("track"):
        state = dict(x=0., y=0., kx=0., ky=0., sx=1., sy=1., f=0., a=1., i="")
        frames = []
        for frame in track.findall("t"):
            for value in frame:
                if value.tag in state:
                    state[value.tag] = (value.text or "") if value.tag == "i" else float(value.text)
            frames.append(state.copy())
        tracks.append((track.findtext("name"), frames))
    return float(root.findtext("fps", "12")), tracks


def clip_range(tracks, marker):
    if marker is None:
        return 0, max(len(frames) for _, frames in tracks)
    if isinstance(marker, tuple):
        return marker
    for name, frames in tracks:
        if name == marker:
            active = [index for index, frame in enumerate(frames) if frame["f"] >= 0]
            if active:
                start = active[0]
                stop = start + 1
                while stop < len(frames) and frames[stop]["f"] >= 0:
                    stop += 1
                return start, stop
    raise ValueError(f"No active sequence {marker}")


def write_motion_profiles(imported, destination):
    """Retain the original ground tracks as private data, never in the code."""
    profiles=[]
    path=destination/"zombie_motion.bin"
    with path.open("wb") as output:
        output.write(struct.pack("<4sI",b"PVM1",8))
        for animation,clip in (("Zombie","anim_walk"),("Zombie_polevaulter","anim_run"),("Zombie_polevaulter","anim_walk"),("Zombie_paper","anim_walk"),("Zombie_paper","anim_walk_nopaper"),("Zombie_football","anim_walk"),("Zombie_disco","anim_walk"),("Zombie_backup","anim_walk")):
            fps,tracks=frames_from_xml(imported/"archives/compressed/reanim"/(animation+".reanim"))
            start,stop=clip_range(tracks,clip);ground=dict(tracks)["_ground"]
            origin=ground[start]["x"];values=[f["x"]-origin for f in ground[start:stop]]
            output.write(struct.pack("<If",len(values),fps));output.write(struct.pack("<"+"f"*len(values),*values))
            profiles.append(dict(animation=animation,clip=clip,frames=len(values),ground_distance=values[-1],fps=fps))
    return dict(file=path.name,bytes=path.stat().st_size,sha256=hashlib.sha256(path.read_bytes()).hexdigest(),profiles=profiles)


def hidden_track(name, equipment):
    name = name.lower()
    if equipment=='planthead' and name.startswith(('anim_head','anim_hair','anim_tongue','zombie_eye')):return True
    if equipment=="duck" and "duckytube" in name:return False
    if equipment=='door':
        if name.startswith('anim_innerarm'):return True
        if 'screendoor' in name:return False
    if equipment=="flag":
        if name.startswith("anim_innerarm"):return True
        if name in ("zombie_flaghand","zombie_innerarm_screendoor"):return False
    if "screendoor" in name or any(x in name for x in ("flaghand", "duckytube", "whitewater", "mustache", "bloated")):
        return True
    if name == "anim_tongue":
        return True
    if name == "anim_cone":
        return equipment != "cone"
    if name == "anim_bucket":
        return equipment != "bucket"
    if name == "anim_hair" and equipment:
        return True
    return False


def power2(value):
    return 1 << (max(1, value) - 1).bit_length()


def write_pva(path, images, fps, anchor=None):
    width, height = images[0].size
    tw, th = max(8,power2(width)), max(8,power2(height))
    if max(tw, th) > 512 or len(images) > 256:
        raise ValueError(f"Texture or frame count too large: {path}")
    payloads=[]
    for image in images:
            padded = Image.new("RGBA", (tw, th))
            padded.paste(image, (0, 0))
            flat = padded.get_flattened_data() if hasattr(padded, "get_flattened_data") else padded.getdata()
            pixels = array.array("H", ((r >> 4) | ((g >> 4) << 4) | ((b >> 4) << 8) | ((a >> 4) << 12)
                                      for r, g, b, a in flat))
            if sys.byteorder != "little":
                pixels.byteswap()
            from texture_layout import tile_rgba4444
            payloads.append(zlib.compress(tile_rgba4444(pixels.tobytes(),tw,th),9))
    with path.open('wb') as output:
        output.write(struct.pack('<4s6If2f',b'PVA4',width,height,tw,th,len(images),tw*th*2,fps,*(anchor or (width*.5,height))))
        offset=0
        for payload in payloads:output.write(struct.pack('<II',offset,len(payload)));offset+=len(payload)
        for payload in payloads:output.write(payload)
    return {"file": path.name, "width": width, "height": height, "texture_width": tw,
            "texture_height": th, "frames": len(images), "fps": fps,
            "bytes": path.stat().st_size, "header_bytes":40+len(images)*8,"decoded_bytes":len(images)*tw*th*2,"sha256": hashlib.sha256(path.read_bytes()).hexdigest()}


def prepare(imported, destination, previews=None, animations_only=False, credits_timing=None, reference_elf=None):
    if destination.exists():
        raise ValueError("Choose a new data directory; existing data is never overwritten")
    source_manifest = json.loads((imported / "manifest.json").read_text())
    if source_manifest["disc_title_id"] != "BLUS30852":
        raise ValueError("Unsupported source build")
    # Case-insensitive source resource lookup, with the locale archive taking priority.
    files = {}
    for name in ("main", "compressed", "en"):
        prefix = imported / "archives" / name
        for path in prefix.rglob("*"):
            if path.is_file():
                files[path.relative_to(prefix).as_posix().lower()] = path

    @lru_cache(maxsize=512)
    def texture(identifier):
        key = identifier.lower().removeprefix("image_reanim_")
        filename = "reanim/" + key + ".ptx"
        if filename not in files:
            raise ValueError(f"Animation texture not found: {identifier}")
        with Image.open(files[filename]) as image:
            return image.convert("RGBA")

    destination.mkdir(parents=True)
    if previews:
        previews.mkdir(parents=True, exist_ok=True)
    assets = []
    geometry={}
    flag_fps,flag_tracks=frames_from_xml(files['reanim/zombie_flagpole.reanim'])
    def matrix(frame):
        kx,ky=math.radians(frame['kx']),math.radians(frame['ky'])
        return (math.cos(kx)*frame['sx'],-math.sin(ky)*frame['sy'],math.sin(kx)*frame['sx'],math.cos(ky)*frame['sy'],frame['x'],frame['y'])
    def multiply(m,n):
        a,b,c,d,x,y=m;e,f,g,h,u,v=n
        return (a*e+b*g,a*f+b*h,c*e+d*g,c*f+d*h,a*u+b*v+x,c*u+d*v+y)
    def invert(m):
        a,b,c,d,x,y=m;det=a*d-b*c
        return (d/det,-b/det,-c/det,a/det,(b*y-d*x)/det,(c*x-a*y)/det)
    def draw_matrix(canvas,im,m,alpha=1):
        a,b,c,d,x,y=m;inverse=invert((a,b,c,d,x+96,y+96))
        ia,ib,ic,id,ix,iy=inverse
        composite_affine(canvas,im,(a,b,c,d,x+96,y+96),(ia,ib,ix,ic,id,iy),alpha)
    for output_name, animation, marker, scale, equipment in SPECS:
        damage=equipment if isinstance(equipment,dict) else {}
        gear=damage.get("gear",equipment)
        source = files["reanim/" + animation.lower() + ".reanim"]
        fps, tracks = frames_from_xml(source)
        start, stop = clip_range(tracks, marker)
        # A generous source canvas handles rotation and negative source coordinates.
        def render_frame(index):
            canvas_size=1536 if animation in ("Zombie_boss","treeofWisdom") else 768 if animation in ("Zombie_gargantuar","CrazyDave") else 384
            canvas = Image.new("RGBA", (canvas_size, canvas_size))
            for name, frames in tracks:
                if damage.get('only_track') and name not in damage['only_track']:continue
                if name in damage.get('omit_track',()):continue
                if index >= len(frames) or (animation == "Zombie" and hidden_track(name, gear)):
                    continue
                body=damage.get("body",0)
                if body and ("outerarm_lower" in name.lower() or "outerarm_hand" in name.lower()):
                    continue
                if body>=2 and (name.startswith(("anim_head","anim_hair","anim_tongue","zombie_eye")) or
                                (animation=="Zombie_polevaulter" and (name.lower().startswith("zombie_polevaulter_pole") or any(part in name.lower() for part in ("_innerarm","_innerhand"))))):
                    continue
                frame = frames[index]
                if animation=="GatlingPea":
                    if output_name=='gatling' and (frame['f']<0 or not frame['i']):
                        candidate=frames[29+(index-start)%25]
                        if candidate['f']>=0 and candidate['i']:frame=candidate
                    elif output_name=='gatling_shoot' and any(part in name.lower() for part in ('leaf','stem','stalk')):frame=frames[4+(index-start)%25]
                if animation=="ThreePeater":
                    head=next((n for n in (1,2,3) if ("head"+str(n)) in name or name in ("anim_face"+str(n),"ThreePeater_mouth"+str(n))),None)
                    if head:frame=frames[{1:4,2:86,3:45}[head]+(index-start)%25]
                if animation=="SplitPea" and (frame["f"]<0 or not frame["i"]):
                    for alternative in (32,82):
                        candidate=frames[alternative+(index-start)%25]
                        if candidate["f"]>=0 and candidate["i"]:frame=candidate;break
                if output_name=="spikeweed" and name.startswith("bigspike"):continue
                if animation in ('PeaShooter','PeaShooterSingle','SnowPea') and output_name.endswith("_shoot") and any(part in name.lower() for part in ("leaf", "stem", "stalk")):
                    # The source uses a separate stem animation while its head shoots.
                    frame = frames[79 + (index - start) % 25]
                if frame["f"] < 0 or frame["a"] <= 0 or not frame["i"]:
                    continue
                identifier=frame["i"]
                # The tank's MOW layer is a runtime attachment placeholder.
                if identifier=='IMAGE_REANIM_MOW':continue
                if damage.get("armor",0) and name in ("anim_cone","anim_bucket"):
                    identifier=f"IMAGE_REANIM_ZOMBIE_{'CONE' if name=='anim_cone' else 'BUCKET'}{damage['armor']+1}"
                if body and name in ("Zombie_outerarm_upper","Zombie_polevaulter_outerarm_upper"):
                    identifier="IMAGE_REANIM_"+name.upper()+"2"
                if animation=="Wallnut" and damage and name=="anim_face":identifier=f"IMAGE_REANIM_WALLNUT_CRACKED{damage['body']}"
                image = texture(identifier)
                kx, ky = math.radians(frame["kx"]), math.radians(frame["ky"])
                a, b = math.cos(kx) * frame["sx"], -math.sin(ky) * frame["sy"]
                c, d = math.sin(kx) * frame["sx"], math.cos(ky) * frame["sy"]
                determinant = a * d - b * c
                if abs(determinant) < 1e-6:
                    continue
                tx, ty = frame["x"] + 96, frame["y"] + 96
                inverse = (d / determinant, -b / determinant, (b * ty - d * tx) / determinant,
                           -c / determinant, a / determinant, (c * tx - a * ty) / determinant)
                composite_affine(canvas,image,(a,b,c,d,tx,ty),inverse,frame["a"])
                if gear=="flag" and name=="Zombie_flaghand":
                    overlay=multiply(matrix(frame),invert(matrix(frames[0])))
                    for child_name,child_frames in flag_tracks:
                        child=child_frames[int((index-start)*15/fps)%len(child_frames)]
                        if child['f']<0 or not child['i']:continue
                        identifier=child['i']
                        if body and child_name=='Zombie_flag':identifier='IMAGE_REANIM_ZOMBIE_FLAG3'
                        draw_matrix(canvas,texture(identifier),multiply(overlay,matrix(child)),child['a'])
            return canvas
        rendered = render_frames(render_frame,range(start,stop))
        union = Image.new("L", rendered[0].size)
        for frame in rendered:
            union = ImageChops.lighter(union, frame.getchannel("A"))
        box = union.getbbox()
        if box is None:
            raise ValueError(f"Empty animation {output_name}")
        box = (max(0, box[0] - 2), max(0, box[1] - 2), min(rendered[0].width, box[2] + 2), min(rendered[0].height, box[3] + 2))
        size = (max(1, round((box[2] - box[0]) * scale)), max(1, round((box[3] - box[1]) * scale)))
        if 'base' in damage:
            box,size=geometry[damage["base"]]
        geometry[output_name]=(box,size)
        baked = [frame.crop(box).resize(size, Image.Resampling.LANCZOS) for frame in rendered]
        anchor=None
        parent={'puff_shoot':'puff','fume_shoot':'fume','scaredy_shoot':'scaredy','scaredy_hide':'scaredy','sunshroom_big':'sunshroom'}.get(output_name)
        if '_death' in output_name:
            root=output_name.split('_death')[0]
            parent=root+'_walk' if root+'_walk' in geometry else parent
        if output_name.startswith('boss_') and output_name not in ('boss_walk','boss_death') and 'boss_walk' in geometry:parent='boss_walk'
        if parent:
            base_box,_=geometry[parent]
            anchor=(((base_box[0]+base_box[2])*.5-box[0])*size[0]/(box[2]-box[0]),(base_box[3]-box[1])*size[1]/(box[3]-box[1]))
        if gear=="flag":
            base_box,_=geometry['zombie_eat' if marker=='anim_eat' else 'zombie_walk']
            anchor=(((base_box[0]+base_box[2])*.5-box[0])*size[0]/(box[2]-box[0]),(base_box[3]-box[1])*size[1]/(box[3]-box[1]))
        assets.append(write_pva(destination / (output_name + ".pva"), baked, 24 if output_name.startswith("pole_jump") else fps,anchor))
        if previews:
            baked[0].resize((size[0] * 4, size[1] * 4)).save(previews / (output_name + ".png"))
        print(f"Prepared {output_name}: {len(baked)} frames, {size[0]}x{size[1]}", flush=True)
    if animations_only:
        (destination/'animations.json').write_text(json.dumps(assets,indent=2))
        return assets
    with Image.open(files["images/background1.ptx"]) as image:
        background = image.convert("RGBA").crop((350, 0, 1390, 720)).resize((480, 272), Image.Resampling.LANCZOS)
        assets.append(write_pva(destination / "background.pva", [background], 1))
        if previews:
            background.save(previews / "background.png")
    # Use source lawn/dirt pixels for tutorial rows. The same row boundaries are
    # used by the handheld grid, so dirt, planting and spawning agree visually.
    with Image.open(files["images/background1unsodded.ptx"]) as image:
        dirt=image.convert("RGBA").crop((350,0,1390,720)).resize((480,272),Image.Resampling.LANCZOS)
    for name,rows in (("background_one",(2,)),("background_three",(1,2,3))):
        tutorial=dirt.copy()
        for row in rows:
            top=58+row*36;bottom=top+36
            tutorial.paste(background.crop((61,top,411,bottom)),(61,top))
        assets.append(write_pva(destination/(name+".pva"),[tutorial],1))
        if previews:tutorial.save(previews/(name+".png"))
    with Image.open(files["images/zombienote1.ptx"]) as image:
        note=image.convert("RGBA");note.thumbnail((345,170),Image.Resampling.LANCZOS)
        assets.append(write_pva(destination/"note.pva",[note],1))
    for name,source in (("projectile_pea","projectilepea"),("projectile_snow","projectilesnowpea")):
        with Image.open(files[f"images/{source}.ptx"]) as image:
            # Same ~0.45 source-to-handheld scale as the rendered Peashooter.
            pea=image.convert("RGBA").resize((13,13),Image.Resampling.LANCZOS)
            assets.append(write_pva(destination/(name+".pva"),[pea],1))
    # Export the actual console seed costs and refresh intervals as data.
    tuning = ET.parse(files["data/adventuretuning.xml"])
    seeds = {int(row.attrib["id"]): row.attrib for row in tuning.getroot().findall("SeedPacket")}
    ids = (0, 1, 3, 2, 4, 5, 6, 7)
    with (destination / "seeds.bin").open("wb") as output:
        output.write(b"PVS1")
        for seed in ids:
            row = seeds[seed]
            output.write(struct.pack("<II", int(row["cost"]), int(row["refresh"])))
    audio = []
    ffmpeg = shutil.which("ffmpeg")
    if ffmpeg:
        sound_sources = ("plant", "throw", "points", "chomp", "cherrybomb", "groan", "lawnmower", "winmusic","hugewave","finalwave","seedlift","rain","thunder","swing","bonk","plastichit","shieldhit","crazydaveshort1","crazydavelong1","crazydavescream","crazydavescream2","squash_hmm","gargantuar_thump")
        for index, name in enumerate(sound_sources):
            source = imported / "archives" / "sounds" / "sounds" / (name + ".mpg")
            target = destination / f"sfx{index}.pcm"
            subprocess.run([ffmpeg, "-v", "error", "-i", str(source), "-t", "8", "-ac", "1", "-ar", "44100",
                            "-f", "s16le", str(target)], check=True)
            audio.append(target.name)
        from prepare_music import prepare_music
        audio.extend(prepare_music(imported,destination))
        print("Prepared original effects and soundtrack sections", flush=True)
    else:
        print("FFmpeg not found: preparing a silent build; install FFmpeg for audio", flush=True)
    manifest = {"schema": 1, "version": "0.18.1", "format": "PVA4 zlib GPU-tiled RGBA4444", "source_iso_sha256": source_manifest["source_sha256"],
                "disc_title_id": "BLUS30852", "assets": assets,
                "texture_bytes": sum(asset["bytes"] - 32 for asset in assets), "audio": audio,
                "note": "Private game data, generated from the user's copy; do not redistribute."}
    manifest["motion"]=write_motion_profiles(imported,destination)
    from prepare_ui import prepare_ui
    assets.extend(prepare_ui(imported,destination,previews,write_pva))
    from prepare_almanac import prepare_almanac
    assets.extend(prepare_almanac(imported,destination,previews,write_pva))
    from prepare_upgrades import prepare_upgrade_images
    assets.extend(prepare_upgrade_images(imported,destination,previews,write_pva))
    from prepare_rewards import prepare_rewards
    assets.extend(prepare_rewards(imported,destination,previews,write_pva))
    from prepare_story import prepare_story
    assets.extend(prepare_story(imported,destination,previews,write_pva))
    from prepare_chapters import prepare_chapters
    assets.extend(prepare_chapters(imported,destination,previews,write_pva))
    from prepare_features09 import prepare_features09
    feature_assets=prepare_features09(imported,destination,previews,write_pva)
    changed={a['file'] for a in feature_assets};assets[:]=[a for a in assets if a['file'] not in changed]+feature_assets
    from prepare_expansion import prepare_expansion
    from prepare_features010 import prepare_features010
    assets.extend(prepare_expansion(imported,destination,previews,write_pva))
    assets.extend(prepare_features010(imported,destination,previews,write_pva))
    from prepare_menus011 import prepare_menus011
    assets.extend(prepare_menus011(imported,destination,previews,write_pva))
    from prepare_fixes012 import prepare_fixes012
    fixes=prepare_fixes012(imported,destination,previews,write_pva)
    replaced={a["file"] for a in fixes};assets[:]=[a for a in assets if a["file"] not in replaced]+fixes
    from prepare_ending import prepare_ending, extract_timing
    if reference_elf:credits_timing=extract_timing(reference_elf,destination)
    manifest['ending']=prepare_ending(imported,destination,previews,24,credits_timing)
    manifest['texture_bytes']=sum(a.get('decoded_bytes',a['bytes']-a.get('header_bytes',32)) for a in assets)
    if ffmpeg:
        songs=json.loads((destination/'music-index.json').read_text())
        manifest['music']={'codec':'IMA ADPCM','sample_rate':22050,'files':songs,'total_bytes':sum(s['bytes'] for s in songs),'old_pcm_bytes':sum(s['pcm_bytes'] for s in songs)}
    from prepare_garden018 import prepare_garden018
    with tempfile.TemporaryDirectory(prefix='pvz-garden-') as garden_temp:
        garden_dir=Path(garden_temp)/'data'
        garden_assets=prepare_garden018(imported,garden_dir,previews)
        for file in garden_dir.glob('*.pva'):shutil.copy2(file,destination/file.name)
        assets.extend(garden_assets)
    if ffmpeg:
        from prepare_audio018 import prepare as prepare_foley
        bank=prepare_foley(imported,destination,reference_elf)
        # Old resident PCM files are superseded by the streaming effects bank.
        for file in destination.glob('sfx*.pcm'):file.unlink()
        manifest['audio']={'file':'foley.pvf','samples':194,'groups':104,'bytes':bank['bank_bytes'],'sha256':bank['bank_sha256'],'codec':bank['codec']}
    manifest['texture_bytes']=sum(a.get('decoded_bytes',a['bytes']-a.get('header_bytes',32)) for a in assets)
    (destination / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(f"Prepared {len(assets)} assets; texture memory {manifest['texture_bytes'] / 1048576:.2f} MiB")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sources = parser.add_mutually_exclusive_group(required=True)
    sources.add_argument("--iso", type=Path, help="Your original PS3 BLUS30852 version 01.00 disc dump containing NPUA_30059; not a PSP ISO")
    sources.add_argument("--imported-dir", type=Path)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--previews", type=Path)
    parser.add_argument("--credits-timing", type=Path, help="Optional timing table prepared from your own decrypted PS3 executable")
    parser.add_argument("--reference-elf", type=Path, help="Optional personal decrypted executable for original ending lip sync")
    args = parser.parse_args()
    try:
        if args.iso:
            with tempfile.TemporaryDirectory(prefix="pvz-import-") as temporary:
                imported = Path(temporary) / "resources"
                inspect_or_import(args.iso, imported)
                prepare(imported, args.output, args.previews, credits_timing=args.credits_timing, reference_elf=args.reference_elf)
        else:
            prepare(args.imported_dir, args.output, args.previews, credits_timing=args.credits_timing, reference_elf=args.reference_elf)
    except (OSError, ValueError, zlib.error, struct.error) as error:
        parser.exit(1, f"Asset preparation failed: {error}\n")


if __name__ == "__main__":
    main()
