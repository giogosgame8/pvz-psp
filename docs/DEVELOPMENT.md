# Developer and manual setup notes

The download ZIP is the supported starting point for players. GitHub's automatic
source archives contain no EBOOT.PBP. Use the top-level README for installation.

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
py -m unittest discover -s tools -p test_setup_assets.py
```

See [performance results](../PERFORMANCE-0.18.1.md),
[port status](../PORTING-STATUS.md) and
[known missing features](../REMAINING-FEATURES-0.18.md) for implementation limits.
The 0.18.2 distribution changes PC setup and documentation; the native game
executable is unchanged from 0.18.1.

## Licensing

No project-wide source-code license has been selected. The importer and third-party
components retain their own licenses; see [notices](../THIRD-PARTY-NOTICES.txt).
Game resources and trademarks belong to their respective owners. This project is
not endorsed by EA, PopCap or Sony; asset-free packaging grants no rights to their content.
