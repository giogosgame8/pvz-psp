# Validation — 0.23.0 development

11 November 2026. Scope, source evidence and unfinished parity work:
[item-by-item report](FIDELITY-0.23-DEVELOPMENT.md).

## Native executable

The final bare EBOOT SHA-256 is
`8a0a0f6d9493656a7f4c1ac44fc066696c0f268ec3cc53a87f53c9901e0ad620`.

| Emulator configuration | Passed checks | Failures | Cache errors |
| --- | ---: | ---: | ---: |
| Street, 60 FPS target, full ending and audio suite | 1,336 | 0 | 0 |
| PSP-1000, 60 FPS preference, live Day/Pool and audio suite | 1,337 | 0 | 0 |

Both tested that executable and the same prepared resources. The Street run
took 378.34 seconds; PSP-1000 took 347.34 seconds. PSP-1000's live Day fixture
measured **59.83 FPS**, 2.133 ms draw CPU; Pool measured **59.53 FPS**, 1.683 ms.
These are emulator measurements, not physical PSP performance certification.
PSP-1000 still defaults to 30 FPS with smaller caches; 60 FPS is selectable.

The fast native logic run separately passed **678 checks**. Fifty campaign
combat fixtures reached their win conditions using real projectiles, damage,
terrain, waves and challenge actions. These are deliberately preplanted,
well-defended synthetic fixtures, not human playthroughs or balance proof.
The boss fixture uses its actual Jalapeño packets during the vulnerable head
window. It does not force boss damage, death or the win state.

New checks cover delayed attacks, target departure and recycled slots, digestion
reload, equipment identity, Snorkel surfacing, Bungee layer/plant identity,
support protection, Coffee, separate vault durations, health distinctions,
full actor pools, full conveyors, original storm packets, grave supply, growth,
Endless reset and malformed save state. Presentation checks cover all new
banks, damaged disarmed bodies, props, word-boundary dialogue pages and timing
tokens. Existing menu, profile, unlock, reward, music and cache tests remain.

The Street ending test played through **159.266 seconds**, displayed 3,526
frames and reported maximum lag **0.534 seconds**, with no movie checksum/decode
or cache error. This is a playback regression result, not a claim of perfect
physical-console A/V synchronization. PSP-1000's run used the shorter UI-only
ending path; it tested indexed movie samples, not a second full ending playback.

All five adaptive themes completed their rise/layer/hold/recovery/pause checks
without read errors or underflows. Rapid menu track changes stayed responsive
under the existing injected 250 ms storage-delay test.

## Preparation and packaging

- **46** preparation/setup Python tests and **6** importer tests passed.
- A freshly frozen Setup.exe prepared the supported owned ISO in **112.497 s**
  with Python absent from PATH. This is one measurement on this host, not a
  multi-PC average or a controlled speed comparison with the smaller old recipe.
- Recipe **ps3-0230-1** produced **631 PVA banks**. All **674 integrity-listed
  files** were verified before and after copying into the private game folder.
- The original ending and 194 sound samples were included in the fresh build.
- The complete decoded texture sum is 461,274,368 bytes; it is streamed through
  bounded caches rather than resident simultaneously. Game state is 51,968 bytes.
- The private XMB wrapper has original owned pictures/music. Its program and
  DATA.PSAR sections are byte-identical to those in the tested bare EBOOT.
- Wrapper SHA-256:
  `fec9cf90cab0016b9c3663081dee30b91813ea00abb23a1973087ff9a4132213`.
- Program payload SHA-256:
  `862bd008f943708c4bb3d312036ee6b6eee5f3d434ad1cf9c29f8fa1578181fd`.
- That wrapped executable booted with no diagnostics or saved settings in both
  emulator profiles: first frame **1.062 s** each. Street selected 60 FPS;
  PSP-1000 selected 30 FPS. Diagnostic startup includes the test suite and is
  intentionally much longer; it is not the normal startup measurement.

The private archive is CRC-tested and each archived game file is SHA-256
compared to the validated folder. It contains no player saves, test flags,
original ISO or PS3 executable. Archive hashes and exact byte counts are in its
adjacent `package.json`. Imported resources stay out of the source repository.

### Public player ZIP

`PvZ-PSP-0.23.0.zip` contains sixteen files: the tested bare executable,
standalone Setup.exe, installation README and required third-party notices.
It contains no imported game resources. Archive SHA-256:
`7d41cb48f88a2a38fabba727e951383937d1c07fa94517d544b6d11fa3647046`.

Setup was run from a fresh extraction of that exact ZIP with system Python
absent from PATH. It completed in **197.871 s** and all **674** integrity-listed
files matched the validated development data. Launcher preparation preserved
the tested native program payload. The timing difference from the earlier
112.497 s run is another reason not to present one run as a general setup-time
guarantee or controlled speed comparison.

## Limits

No physical Street/1000/2000/3000 acceptance, two-console ad-hoc session,
suspend/resume, battery interruption or multi-hour soak test was performed.
The report explicitly leaves original Heavy Weapon, boss/dancer choreography,
several specialised transitions/effects, story cameras, rare garden presentation
and exact reward choreography unfinished. Passing regression tests does not
make those features exact PS3 reproductions.
