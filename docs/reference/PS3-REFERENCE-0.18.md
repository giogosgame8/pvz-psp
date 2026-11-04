# Evidence for 0.18

Supported source: BLUS30852 compilation containing NPUA_30059.
ISO SHA-256: 50ecf8b9a8ccbba80bbbba83d9456c85f3080c33039a1f7424261235faaf1053.
The owner's extracted/decrypted executable was read locally. It is not bundled.
No RPCS3 session was opened for this update.

* Foley: ELF file offset 0x8538c4; 104 records of 52 bytes, big-endian
  ID/float pitch/ten weighted sound-global pointers/flags. Sound-global and
  resource-name arrays were correlated with the original resource manifest.
  tools/reference/FOLEY-REFERENCE-0.18.json records the recovered numeric facts. The packer can
  reread the table with --reference-elf. All group variant lists were cross-checked
  with the local supporting PvzpFoley source; the actual PS3 flags take precedence.
* The sound archive has 194 actual MPGs. resources.xml has 194 Sound IDs but
  only 193 distinct available paths: tapglass is absent, one alias shares a sample,
  and thunder_4/thunder_4r exist outside those declarations. Foley-index.json in
  the personal data identifies each actual sample and the unavailable declaration.
* Palette: PS3 function around VA 0x259840–0x259aac, colour constructor calls
  0x2598b8–0x2599bc. Eleven colours apply to Marigold_petals only. Its variation
  branches also name anim_idle_noflower, anim_waterplants, anim_zengarden,
  anim_idle_aquarium and anim_bigidle.
* Challenges: ELF file offset 0x85125c, six big-endian words per record. Only
  mode 14 (Pool Endless) is on Survival page 0; other Endless terrains are limbo.
  Puzzles alternate modes 52/53, 54/55, etc., with separate family progress.
  The native UI stores each family contiguously without changing that arithmetic.
* GetChallengeDefinition: VA 0x1bc208. Unlock arithmetic: 0x1bc2b8–0x1bc81c.
  This directly confirms the +3 trophy allowance, 10-trophy Endless requirement,
  first-three/Adventure gates and per-family puzzle counts. Relevant branches:
  0x1bc3ac, 0x1bc3bc, 0x1bc5cc, 0x1bc690, 0x1bc734 and 0x1bc768.
* Tool clips/rates, event placement details and present milestones additionally
  use local supporting ZenGarden, ChallengeScreen, Board, Plant and Zombie
  source as behavioral references. Those details are not all independently traced
  through the stripped PS3 executable. The implementation is independently written.

Emulator results prove the tested native code paths, checksums and memory limits,
not exact PS3 choreography, physical PSP performance or every original event timing.
See TEST-RESULTS.txt and BUILD.json in the delivered package for the exact binary.

## Additional direct executable checks for 0.18.9

- Eating cues: VA 0x2cc3bc-0x2cc4e4. The normalized loop positions are
  0.14/0.68 normally, 0.38/0.80 for Pole Vaulter, 0.42 once for Newspaper
  and Ladder, 0.53 once for Jack-in-the-Box, 0.33/0.83 for Bobsled, and
  0.33/0.79 for Imp. Both tests call the timed-event helper at 0x2d9a48.
  Equal positions lead to one sound through the original short-circuit test.
- Eating playback: VA 0x2bca18-0x2bcacc. Normal playback uses 36 authored
  frames/second, with a 20 frames/second branch for types 3, 16, 24, 17,
  15, 11 and 19 (Pole, Balloon, Imp, Digger, Jack, Snorkel and Yeti).
  These override the reanimation file's default FPS.
- The native code shares its selected clip and playback clock between drawing
  and sound. It handles cue crossings across loop boundaries and skipped
  display frames, rather than counting down a separate 0.6-second timer.
  Existing damaged/customizer clips remain prepared data, with no save-format
  change. Other cinematic, combat and audio differences listed in the audit
  remain outside this focused correction.
