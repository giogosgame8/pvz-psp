# Developer and manual setup notes

The download ZIP is the supported starting point for players. GitHub's automatic
source archives contain no EBOOT.PBP. Use the top-level README for installation.

## Portable Windows setup

The player ZIP includes Setup.exe, which embeds Python and Pillow. Players do
not install Python or use these development commands. FFmpeg remains external;
setup detects it or offers installation through Windows Package Manager.
It also accepts ffmpeg.exe beside Setup.exe or in its tools subdirectory.

To build the portable executable on 64-bit Windows with Python 3.12:

```powershell
py -m pip install Pillow==12.3.0 PyInstaller==6.22.3
py .\tools\build_windows_setup.py
```

The result is build-pc/dist/Setup.exe. Its runtime licenses are written beside
it in windows-setup-licenses; ship those under third_party/windows-setup.
The packaged program launches itself with --prepare-worker instead of looking
for a system Python. User data/logs live beside the executable; bundled Foley
facts are read from its temporary runtime directory. The portable worker does
not generate developer C headers.

Version 0.18.6 uses compiled channel operations for RGBA4444 packing and parallel
frame compression. Graphics use up to 16 threads, audio up to 8 workers, and the
ending up to 8 hidden processes. CPU capacity and available memory can reduce
those counts. Each ending process retains its own scene clock and caches; at
most two chunks per worker wait for the ordered writer.

For developer benchmarks, PVZ_PREP_GRAPHICS_WORKERS, PVZ_PREP_AUDIO_WORKERS and
PVZ_PREP_ENDING_WORKERS override the requested worker counts (1-32); memory caps
still apply. The ending and sound command-line tools also accept --jobs.
The ordinary player flow selects counts automatically. Music uses up to four
threads (PVZ_PREP_MUSIC_WORKERS for benchmarks) and a bit-identical normalization
lookup table. Only the current streaming Foley bank is generated; obsolete
resident PCM effects are no longer converted and discarded.

Version 0.18.7 adds 16 private avatar layer banks, owned-disc launcher pictures,
a CRC-checked assets.idx and integrity.json. Normal native startup reads the
metadata index in one operation and skips developer tests; benchmark.flag still
enables the full diagnostic suite. The main-thread first-frame time is written
to runtime.log. Individual PVA loading remains a fallback when the index is
absent or invalid.

Setup verifies SHA-256 and size for all indexed content before installation and
reuse, and checks the source ISO identity and preparation recipe. --rebuild
requests a fresh conversion. Ordinary repeat runs reuse valid 0.18.7 data and
repack the launcher pictures into the supplied executable. User saves are not
part of the prepared-data cache. Zombie presets live in custom-zombie.dat.

Full portable tests on an i5-14600K (14 cores/20 logical threads), with Python
and py absent from PATH, took 235.915 seconds for 0.18.5 and 93.540 seconds for
0.18.6. All 388 content/metadata files were byte-identical; only the diagnostic
ending-progress.json differed. The PSP executable and 0.18.5 data format remain
unchanged. Setup timing depends on hardware and background workload.

## Manual preparation

See [MANUAL-SETUP.txt](MANUAL-SETUP.txt). Commands run from the extracted package root:

```powershell
py -m pip install Pillow
py .\tools\prepare_assets.py --iso "C:\path\to\your-owned-copy.iso" --output ".\prepared-data"
```

Python 3.10+ and FFmpeg with libopenmpt are required. Copy the resulting
prepared-data folder into PSP/GAME/PVZPSP and rename it data.

The supported source is PS3 BLUS30852 version 01.00 containing NPUA_30059.
PSP images and EBOOTs cannot supply those resources. The importer reads the ISO
without modifying it. Original resources must stay private.

## Build

With PSPDEV, libjpeg-turbo and zlib installed:

```powershell
py .\tools\build.py --pspdev "C:\path\to\pspdev" --no-launcher-media
```

The program is written to build/EBOOT.PBP. Its model detection is automatic.
The optional --low-memory switch forces the conservative profile for diagnostics.

## Tests

```powershell
py -m unittest discover -s importer -p test_disc_input.py
py -m unittest discover -s tools -p "test_*.py"
```

See [performance results](PERFORMANCE.md),
[port status](reference/PORTING-STATUS.md) and
[known missing features](KNOWN-LIMITATIONS.md) for implementation limits.
The 0.18.3 setup limits affine rendering to relevant image regions, uses up to
eight rendering threads, and converts up to four sound files concurrently.
Generated game data was checked against the validated 0.18.2 output. Version
0.18.5 also changes presentation, the native executable and prepared content;
install its new program and data together. Existing save layouts are preserved.

A full conversion on an i5-14600K with dependencies already installed took
7 minutes, compared with approximately 20 minutes for the previous setup run
on the same PC. All 382 generated content/metadata files were byte-identical;
only the ending progress log differed in its elapsed time. Other PCs will vary.

## Licensing

No project-wide source-code license has been selected. The importer and third-party
components retain their own licenses; see [notices](../third_party/NOTICE.txt).
Game resources and trademarks belong to their respective owners. This project is
not endorsed by EA, PopCap or Sony; asset-free packaging grants no rights to their content.
