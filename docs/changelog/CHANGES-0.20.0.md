# 0.20.0 — animated menus and dancer choice

- Faster graphics preparation stages with bounded layer reuse and compiled
  portrait decoding. See [first-setup measurements](../reference/SETUP-SPEED-0.20.0.md).
- Saved Disco / Michael Jackson lead Dancing Zombie choice in Options, using
  artwork already present on the supported original PS3 disc.
- Original ambient animations on the three main menu pages, plus an animated
  crow with flight, landing, idle and call sounds.
- Extras artwork and D-pad targets appear only after their Adventure rewards.
- Existing gameplay save format remains version 11. Older UI preferences import
  with Disco selected. Update **EBOOT.PBP and data together**, preserving saves.

This version requires freshly prepared 0.20.0 data. Public downloads remain
asset-free; run Setup.exe with the supported original PS3 ISO.

[Completion review and remaining work](../reference/COMPLETION-0.20.0.md)

[Validation results](../reference/VALIDATION-0.20.0.md): 31 preparation tests,
6 importer tests, 1,152 Street and 1,151 PSP-1000 native assertions passed.
Physical hardware remains unverified; dense PSP-1000 scenes still perform poorly.
