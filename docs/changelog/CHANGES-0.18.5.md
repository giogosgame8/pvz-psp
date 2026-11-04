# 0.18.5 - presentation fixes and setup without installing Python

- Adds a portable 64-bit Windows Setup.exe with Python, Pillow, the ISO importer
  and all asset-preparation modules embedded.
- Keeps the ISO picker, stage progress, backup protection and final READY message.
- Uses the same executable for the preparation worker; no system Python or py
  launcher is needed. FFmpeg remains a separate detected/assisted dependency.
- Uses the executable's folder for user output/logs and bundled resources for
  the Foley reference table. Background workers do not open extra consoles.
- Simplifies the player ZIP to Setup.exe, README, PSP and third-party notices.
  Developer source remains available in the repository and source archives.
- Correctly untile PVA4 pixels before composing packet portraits and plant awards.
- Keeps nested ending dancers' clocks running across static container loops.
- Places ending text at its authored top position so dialogue fits its bubbles.
- Shares the idle pose between Peashooter-family heads and stems and aligns
  firing recoil to that pose instead of resetting the idle head's position.
- Adds original detached-head artwork, falling/bouncing motion and final fade
  for the ordinary zombie family. Cosmetic effects do not consume gameplay RNG
  or change the saved Game structure. Flag zombies also stop eating headless.
- Moves the main menu's Player button to the top right.

Both the PSP executable and game data are updated. Run the new setup and copy
the new EBOOT.PBP and data folder together; keep existing player saves. The new
data manifest version is 0.18.5 and includes firing-alignment data and five head
textures. Textures still use PVA4, music IMA ADPCM and the ending indexed JPEG.
