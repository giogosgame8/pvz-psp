# 0.23.0 — Early access

Includes the work developed in the 0.21, 0.22 and 0.23 checkpoints since the
previous public 0.20.1 release.

- Repeat Adventure, locked Dave seed choices, Yeti encounters/retreat, Giga
  variants, Gold Sunflower progression, purchased house blueprints and secret effects.
- Dynamic arrangements for the five level music themes, with continuous playback
  while loading tracks.
- Delayed plant attacks and recovery, equipment removal that preserves zombie
  types, advanced damage stages, Snorkel surfacing, Bungee theft phases and visible
  Catapult basketballs.
- Corrected conveyor plant selections, delivery timing, support/grave-aware supply,
  bowling collision calculations, several health values and boss immunity windows.
- Dave's original talking poses, held props and speech bubble; long dialogue pages
  fit the frame.
- Better save recovery, Endless puzzle cleanup and bounded rendering caches.

## Updating

Back up your current PSP game folder. Extract the new game ZIP into a new PC
folder, run Setup.exe with your original PS3 ISO, then replace **EBOOT.PBP and
data together**, keeping your existing save/settings files. No Python installation
is needed. Older saves are imported; older executables cannot read saves updated
by this release.

## Validation and remaining work

1,336 Street and 1,337 PSP-1000 emulator checks passed, with no failures or cache
errors. Street completed the full ending playback. These results do not certify
physical-console performance or exact PS3 behaviour.

Heavy Weapon details, boss/dancer choreography, some story cameras, effects,
reward timing and other fidelity work remain unfinished. Multiplayer is unavailable.

[Known limitations](../KNOWN-LIMITATIONS.md) ·
[Detailed implementation report](../reference/FIDELITY-0.23-DEVELOPMENT.md) ·
[Validation](../reference/VALIDATION-0.23-DEVELOPMENT.md)
