# PvZ PSP 0.24.3 development

- Player data now lives in `PSP/SAVEDATA/PVZPSP` on the Memory Stick.
  Back up or restore that one folder while the game is closed.
- On the first successful launch, the game copies progress, challenge saves,
  the profile, preferences, the player name, Zombatar and existing backups
  from the install folder. The originals are left intact. A restored save
  folder takes precedence over old copies in the install folder.
- Clearing a wave before a scheduled flag no longer cuts a 45-second
  countdown to two seconds. Ordinary waves still advance early when the
  remaining health falls below the original threshold.

0.24.1 and 0.24.2 use the same prepared data. Replace just `EBOOT.PBP` if
updating from one of those builds. Keep the legacy files in the install
folder through the first launch so the game can migrate them.

[Migration details and fidelity work](../reference/SAVEDATA-0.24.3.md)
