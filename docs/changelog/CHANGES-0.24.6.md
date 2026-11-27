# PvZ PSP 0.24.6 single-player preview

- Reduced stalls when opening pause, moving the lawn cursor, and showing the
  level reward. Improved animation loading for the Almanac and busy scenes.
- Fixed combat and presentation issues in Adventure, Zombotany, Portal Combat,
  Bobsled Bonanza, and Heavy Weapon.
- Improved music transitions, sound mixing, and ending playback.
- Improved checkpoint validation and recovery from interrupted writes. Existing
  saves still use `PSP/SAVEDATA/PVZPSP`.
- Windows Setup prepares game files more efficiently and can reuse a verified
  installation. Setup runs without a separate Python install.

## Validation

The PSP Street and PSP-1000 profiles in PPSSPP each passed 1,770 checks,
including fixtures for all 50 Adventure levels. The PSP-2000 and PSP-3000
profiles also reached the main menu with an existing save. All 71 Windows
installer and importer tests passed.

This preview covers offline single-player. Performance and suspend/resume on
physical PSP hardware still need testing. See the [known limitations](../KNOWN-LIMITATIONS.md).
