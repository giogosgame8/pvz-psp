# Known limitations — 0.24.1 development

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

The 0.24.1 scene/regression executable passed 1,389 checks on each of the
Street and PSP-1000 emulator profiles, with no failures or cache errors.
See [0.24.1 validation](reference/VALIDATION-0.24.1.md). The longer full-ending
and audio runs are documented in [0.24.0 validation](reference/VALIDATION-0.24-DEVELOPMENT.md).
Emulator results are not physical-console certification or complete PS3 parity.

PSP-1000 support is experimental. It defaults to 30 FPS and smaller caches;
the optional 60 FPS target is not a physical-hardware performance guarantee.
Suspend/resume, battery interruption and extended hardware play still need testing.

0.24.1 requires newly prepared data. Game saves remain version 14. Profile format
remains version 5; version 13 and older supported game saves import.
Back up your existing folder before updating. Builds before 0.24.0 cannot read
version 14 saves; keep your backup if reverting.

[0.24.1 scene corrections](reference/SCENE-0.24.1.md) ·
[Validation results](reference/VALIDATION-0.24.1.md) ·
[Full completion checklist](reference/COMPLETION-WORK.md)
