# Known limitations — 0.24.5 development

This is an unfinished native PSP reimplementation, not an exact PS3 reproduction.
It imports resources from the original BLUS30852 version 01.00 PS3 ISO. It does
not run the PS3 executable on PSP.

## Missing or unfinished

- **Multiplayer:** playable co-op, versus and PSP ad-hoc transport are not implemented.
- **PS3-specific challenge details:** Heavy Weapon's primary upgrade and firing
  rules now use the PS3 values. Exact controller feel and additional effects,
  along with Bobsled movement and some Zombotany and portal edge cases, remain open.
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

The 0.24.3 scene/regression executable passed 1,408 checks on each of the
Street and PSP-1000 emulator profiles. Normal Street startup verified
save migration and backup restoration. The 0.24.2 live-combat and full
ending/audio results are linked separately.
Emulator results are not physical-console certification or complete PS3 parity.

PSP-1000 support is experimental. It defaults to 30 FPS and smaller caches;
the optional 60 FPS target is not a physical-hardware performance guarantee.
Suspend/resume, battery interruption and extended hardware play still need testing.

0.24.3 uses the same prepared data as 0.24.1 and 0.24.2. Replace only
EBOOT.PBP when updating from either of those builds. Player data now lives
in `PSP/SAVEDATA/PVZPSP`; legacy files are copied from the installation
on the first successful launch. Game saves remain version 14 and profiles
remain version 5. Back up both folders if reverting.

[0.24.3 migration and validation](reference/SAVEDATA-0.24.3.md) ·
[0.24.4 polish and Bonus Endless](reference/POLISH-0.24.4.md) ·
[0.24.5 Heavy Weapon and crow](reference/WEAPONS-CROW-0.24.5.md) ·
[0.24.2 extended validation](reference/VALIDATION-0.24.2.md) ·
[Remaining work](reference/REMAINING-0.24.2.md)
