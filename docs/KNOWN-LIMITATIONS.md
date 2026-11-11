# Known limitations — 0.23.0

This is an unfinished native PSP reimplementation, not an exact PS3 reproduction.
It imports resources from the original BLUS30852 version 01.00 PS3 ISO. It does
not run the PS3 executable on PSP.

## Missing or unfinished

- **Multiplayer:** playable co-op, versus and PSP ad-hoc transport are not implemented.
- **PS3-specific challenge details:** Heavy Weapon weapon behaviour, some Zombotany
  attacks, Bobsled movement and portal interactions still need work.
- **Original combat and pacing:** boss and dancer choreography, some target and
  status rules, projectiles, specialised deaths, effects and late Endless balance
  are not exact reproductions.
- **Presentation:** some story camera sequences, dialogue expressions, menu
  transitions, garden variants and reward choreography remain unfinished.
- **Ending:** the song and reconstructed 480×272/24 FPS animation are included;
  exact original compositing and physical-console A/V timing are not certified.
- **Zombatar:** the customizer uses available PS3 zombie artwork. The supported
  disc does not contain the complete original GOTY creator resources.

PSN leaderboards and PlayStation system trophies are outside the offline scope.
Local collectible trophies are separate from PlayStation system trophies.

## Testing and compatibility

The final 0.23.0 executable passed 1,336 checks on the Street emulator profile
and 1,337 on PSP-1000, with no failures or cache errors. These are emulator
results, not physical-console certification or proof of complete PS3 parity.

PSP-1000 support is experimental. It defaults to 30 FPS and smaller caches;
the optional 60 FPS target is not a physical-hardware performance guarantee.
Suspend/resume, battery interruption and extended hardware play still need testing.

0.23.0 requires newly prepared data and updates the game/profile save formats.
Back up your existing folder before updating. Older builds cannot read the new
saves; keep the backup if you want to revert.

[Detailed status for items 4–34](reference/FIDELITY-0.23-DEVELOPMENT.md) ·
[Validation results](reference/VALIDATION-0.23-DEVELOPMENT.md) ·
[Full completion checklist](reference/COMPLETION-WORK.md)
