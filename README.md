# Plants vs. Zombies PSP

An unofficial, experimental native PSP reimplementation. This project does not
run the PS3 executable on PSP and is not affiliated with or endorsed by EA,
PopCap or Sony.

**Current preview: 0.18.1.** It is unfinished and does not reproduce every
feature, animation, wave or interaction of the original game.

## Download and requirements

Download the **asset-free ZIP** from [Releases](https://github.com/giogosgame8/pvz-psp/releases). It contains the
PSP executable, source and asset-preparation tools, but no original game art,
music, sound banks, ending frames, PS3 executables, firmware or disc image.

You need:

- A PSP configured to run homebrew.
- Your own supported disc dump: **BLUS30852**, containing PvZ **NPUA_30059**.
  Other editions are not currently supported by the importer.
- A computer with Python 3.10+, Pillow, and FFmpeg with XM/libopenmpt decoding.
- Sufficient free space on the PSP for the locally prepared resources.

Full instructions: [ASSET-FREE-SETUP.txt](ASSET-FREE-SETUP.txt).

## Quick installation

Extract the release ZIP on your computer. With FFmpeg available on PATH, open
PowerShell in the extracted folder and run:

```powershell
py -m pip install Pillow
py .\tools\prepare_assets.py --iso "C:\path\to\your-owned-copy.iso" --output ".\prepared-data"
```

Replace the ISO path with your own file. The output folder must not already
exist. Preparation can take time. Keep prepared resources private.

Copy `PSP/GAME/PVZPSP` from the ZIP onto the PSP's memory card. Copy the resulting
`prepared-data` folder into `PSP/GAME/PVZPSP` and rename it `data`:

```text
PSP/GAME/PVZPSP/EBOOT.PBP
PSP/GAME/PVZPSP/data/...
```

Start the game from the PSP Game menu. Back up existing installations and saves
before updating. See [INSTALL-0.18.1.txt](INSTALL-0.18.1.txt) for model settings.

## Implemented content

- 50 first-run Adventure stages and level replay.
- 20 main mini-game selections, 11 Survival selections and 20 Puzzle selections.
- 49 plants, Zen Garden and additional gardens, Tree of Wisdom, shop and Almanac.
- Chapter notes, rewards, player names and saved progression.
- Level-context music, imported sound-event support and a reconstructed animated
  ending, generated locally from the owner's resources.
- Compact PSP menus, animation gallery and optional health bars.

These counts describe available implementations, not exact PS3 parity.

## Models and performance

One executable detects the PSP model. Street, PSP-2000 and PSP-3000 target 60 FPS.
PSP-1000 defaults to 30 FPS, with a saved experimental 60 FPS option in Options.
Unknown model queries use the conservative 30 FPS profile. Simulation runs at
60 updates per second regardless of the display target.

The latest validation used PPSSPP 1.20.4. Sustained PSP-1000-mode stress scenes
averaged 58.67 FPS by day and 56.16 FPS at the pool. These are emulator results,
not physical-console benchmarks or a guarantee of locked 60 FPS. The 60 FPS
live configuration passed 976 native checks; other tested configurations
passed 973 each. Real-console testing and long Endless sessions remain needed.

See [PERFORMANCE-0.18.1.md](PERFORMANCE-0.18.1.md) for workload and test limits.

## Known limitations

- Co-op, versus and PSP ad-hoc multiplayer are not implemented and are hidden.
- The original second Adventure run, full Yeti progression, Giga-gargantuar,
  Gold Sunflower completion, house customization and secret-code cosmetics are
  incomplete or missing.
- Exact challenge layouts, waves, late Endless balance, boss choreography,
  several special mini-game mechanics and presentation still need work.
- The ending is reconstructed from layered resources at 480x272/24 FPS, not a
  frame-exact reproduction of the PS3 compositor.
- PSN services and PlayStation system trophies are outside this PSP project.

Read [REMAINING-FEATURES-0.18.md](REMAINING-FEATURES-0.18.md) for details.

## Building

With PSPDEV installed, including libjpeg-turbo and zlib:

```powershell
py .\tools\build.py --pspdev "C:\path\to\pspdev" --no-launcher-media
```

This creates `build/EBOOT.PBP`. The ordinary build includes model detection;
`--low-memory` forces the conservative profile for diagnostics.

## Reporting problems

Open an issue with your PSP model (or emulator version/settings), firmware,
release version, mode/level, and the exact steps needed to reproduce the bug.
Attach screenshots or a short clip if useful. Remove personal information from
logs and never upload an ISO, extracted game resources, keys or firmware.

## Notices and licensing

Original game content and trademarks belong to their respective owners. Users
must provide their own resources; no ownership of that content is claimed.
Asset-free packaging is not a grant of permission from the original rights
holders. The download is not an official EA, PopCap or Sony release.

A project-wide source-code license has not yet been selected. Do not assume a
dependency's license applies to the whole project. The importer and third-party
components retain their own licenses and notices; see
[THIRD-PARTY-NOTICES.txt](THIRD-PARTY-NOTICES.txt), `importer/` and `third_party/`.
