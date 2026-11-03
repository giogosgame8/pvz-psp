"""Prepare personal XMB media from supplied artwork and owned music."""
from pathlib import Path
import argparse
import json
import shutil
import subprocess
import tempfile
from PIL import Image
from launcher_media import validate_media, sha

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--artwork', type=Path, required=True)
    p.add_argument('--icon', type=Path, required=True)
    source = p.add_mutually_exclusive_group(required=True)
    source.add_argument('--music', type=Path)
    source.add_argument('--imported-dir', type=Path, help='Prepare the original menu section from music/MAINMUSIC.XM')
    p.add_argument('--pspbuild', type=Path, required=True, help='ItsNoHax/pspbuild audio encoder')
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--duration', type=float, default=20)
    args = p.parse_args()
    if not 2 <= args.duration <= 50:
        p.error('Use a preview duration between 2 and 50 seconds')
    ffmpeg = shutil.which('ffmpeg')
    if not ffmpeg:
        p.error('FFmpeg must be available on PATH')
    dest = args.output.resolve(); dest.mkdir(parents=True, exist_ok=True)
    for source, name, size in [(args.artwork, 'PIC1.PNG', (480, 272)), (args.icon, 'ICON0.PNG', (144, 80))]:
        with Image.open(source) as image:
            image.convert('RGB').resize(size, Image.Resampling.LANCZOS).save(dest/name, optimize=True)
    with tempfile.TemporaryDirectory(prefix='pvz-launcher-') as temporary:
        work = Path(temporary); music = args.music
        if args.imported_dir:
            from prepare_music import module_section
            raw = (args.imported_dir/'music/MAINMUSIC.XM').read_bytes()
            selection, _ = module_section(raw, 152)
            music = work/'menu.xm'; music.write_bytes(selection)
        pcm = work/'preview.wav'
        filters = ('lowpass=f=10000,loudnorm=I=-18:TP=-2:LRA=11,'
                   'afade=t=in:st=0:d=0.12,'
                   f'afade=t=out:st={args.duration-.2:g}:d=0.2')
        subprocess.run([ffmpeg, '-y', '-v', 'error', '-i', str(music), '-t', f'{args.duration:g}',
            '-ar', '44100', '-ac', '2', '-af', filters, '-c:a', 'pcm_s16le', str(pcm)], check=True)
        subprocess.run([str(args.pspbuild.resolve()), 'audio', 'snd0', str(pcm), '-o', str(dest/'SND0.AT3')], check=True)
    check = subprocess.run([str(args.pspbuild.resolve()), 'audio', 'inspect', str(dest/'SND0.AT3')],
        check=True, capture_output=True, text=True)
    (dest/'audio-validation.txt').write_text(check.stdout, encoding='utf-8')
    subprocess.run([ffmpeg, '-v', 'error', '-i', str(dest/'SND0.AT3'), '-f', 'null', '-'], check=True)
    facts = validate_media(dest)
    facts['artwork_input_sha256'] = sha(args.artwork.read_bytes())
    facts['icon_input_sha256'] = sha(args.icon.read_bytes())
    facts['audio_encoder'] = 'ItsNoHax/pspbuild 0.1.0; see LAUNCHER-0.11.1.md'
    (dest/'launcher-media.json').write_text(json.dumps(facts, indent=2), encoding='utf-8')
    print(json.dumps(facts, indent=2))

if __name__ == '__main__':
    main()
