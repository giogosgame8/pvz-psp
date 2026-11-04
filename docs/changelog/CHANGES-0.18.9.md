# 0.18.9 - animation-driven chewing

- Replaces the independent 0.6-second chewing timer with normalized eating-loop
  cue crossings recovered from the owner's PS3 executable. Several zombie types
  use different cue positions; single-cue types do not play duplicate sounds.
- Restores the PS3 eating playback-rate override (36 authored frames/second,
  or 20 for Pole, Balloon, Imp, Digger, Jack, Snorkel and Yeti). Both the picture
  and sound use the selected animation bank and the same playback clock.
- Later zombie types use their own eating clock. Chilled ordinary eaters advance
  their eating motion at half speed. Frozen, headless, dead and vaulting zombies
  cannot produce a new chew; pausing retains the existing audio pause behavior.
- Bobsled zombies use their eating clip when eating instead of the on-foot walk.
- Preserves the original hard/soft chomp groups and their imported variants.
- Updates the ISO-content audit, distinguishing missing functionality from
  implemented modes that still need closer PS3 matching.

The data recipe remains 0.18.8. No asset reconversion or save migration is needed
when updating a complete 0.18.8 install. The public EBOOT has no original launcher
art; run Setup.exe over the existing PC setup folder to verify/reuse its data
and put the owner's launcher pictures into the new executable before copying it.

Native diagnostics include 50 new checks for the recovered cues/rates, 30/60 FPS
and uneven frame pacing, loop wrap, clip restart/change, old saved positions,
chilled gameplay, correct effect groups and customizer clock compatibility.
Other differences in docs/KNOWN-LIMITATIONS.md remain; this does not establish
complete PS3 parity or physical PSP performance.

The final executable passed 1,061 checks in each Street and PSP-1000 emulator
profile, including all 50 chewing checks, with zero cache errors. The unchanged
portable setup reused the existing data without Python on PATH and embedded
launcher pictures into the new executable without changing its game payload.
