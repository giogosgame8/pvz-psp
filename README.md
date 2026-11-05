# Plants vs. Zombies for PSP

An unfinished, unofficial PSP reimplementation. Original game files are not included.

**[Download the game ZIP](https://github.com/giogosgame8/pvz-psp/releases/download/v0.19.0/PvZ-PSP-0.19.0.zip)**

## What you need

- A PSP that can run homebrew: Street, PSP-2000, PSP-3000 or experimental PSP-1000 support.
- Your own **original PS3 game ISO: BLUS30852, version 01.00**. An existing PSP ISO will not work.
- A 64-bit Windows computer. **No Python installation needed.**

## Install

1. Download the game ZIP above and **extract all files**.
2. Double-click **Setup.exe** on your computer.
3. Select your **original PS3 ISO**. Setup helps install FFmpeg if needed.
4. Wait until the window says **READY**. Preparing the game can take several minutes.
5. Copy **PVZPSP** from the extracted **PSP/GAME** folder into **PSP/GAME** on your PSP memory card.
6. Start the game from the PSP's **Game** menu.

Back up existing saves before updating. Download the game ZIP above; GitHub's
**Source code** ZIP does not include Setup.exe or EBOOT.PBP.

Updating? Extract this ZIP over your existing PC setup folder and run Setup.exe.
Version 0.19.0 rebuilds the game files for new animations. Copy **EBOOT.PBP and
data together** to your PSP, keeping your save files. Back up saves first:
once saved by 0.19.0, they cannot be loaded by older builds.
Repeat setup runs verify and reuse compatible files instead of rebuilding them.

## Good to know

- This is early access: some modes and original-game behavior still need work.
- Multiplayer is not available yet.
- Open **Player → Zombie customizer** to create a saved flag-zombie appearance using PS3 artwork.
- PSP-1000 defaults to 30 FPS; its Options menu offers an experimental 60 FPS target.

**[Report a bug](https://github.com/giogosgame8/pvz-psp/issues/new/choose)**

[Known limitations](docs/KNOWN-LIMITATIONS.md) · [Manual setup / building](docs/DEVELOPMENT.md) · [Credits and notices](third_party/NOTICE.txt)
