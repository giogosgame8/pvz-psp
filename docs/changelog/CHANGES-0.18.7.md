# 0.18.7 - startup, launcher artwork and zombie customization

- Normal launches no longer run the developer self-tests. Diagnostic checks
  remain available through benchmark mode.
- A CRC-checked animation metadata index replaces hundreds of startup file
  opens. Invalid/missing indexes fall back to individually validated headers.
  Compressed frames are still decoded and checked when used.
- Setup hashes every prepared file before installation or reuse. A repeat run
  with the same supported ISO can reuse verified current data. Corrupt or old
  data requires a rebuild; existing game data and saves retain backup protection.
- Setup creates an icon and background from the owner's imported main-menu art
  and embeds them in EBOOT.PBP without changing its game-code sections.
- Adds Player → Zombie customizer: five face/glasses styles, hair/cone/bucket/
  helmet choices, three headgear conditions, three mustaches, and an in-game
  toggle. Presets persist separately from campaign saves and replace the flag
  zombie's living appearance, including walking and eating animations.
- This is a PSP customizer using available PS3 artwork, not the GOTY Zombatar
  asset library, which is absent from the supported PS3 disc.
- More Ways Left/Right navigation follows the stones' horizontal positions and
  wraps at either end. Triangle and Circle no longer open/launch Adventure from
  the home screen. Adventure remains accessible through its selected button.

The data version is 0.18.7. Install both the new executable and prepared data.
The native Game save layout is unchanged. All 387 prior content files (excluding
the updated manifest and progress diagnostic) remained byte-identical.

The portable first preparation took 189.541 seconds on the reference i5-14600K;
verified reuse took 3.528 seconds, with Python/py absent from PATH. Normal Street
profile emulation reached its first frame in 1.666 seconds (0.694 seconds from
the game's main function). These are PC/emulator measurements, not physical PSP
timings or an average for other computers and memory cards.
