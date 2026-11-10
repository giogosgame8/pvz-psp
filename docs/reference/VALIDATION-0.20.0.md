# 0.20.0 validation — 10 November 2026

Final bare EBOOT.PBP SHA-256:
`a3a5584b2f0cb29b80e4e5f1b5f5d4ba8d1cf4f809c2c23c4ba94b52e37330f2`

Both emulator profiles tested this same executable with the final 0.20.0 data.
These are PPSSPP 1.20.4 checks, not physical-console certification.

| Check | Result |
| --- | --- |
| Python preparation/launcher/setup tests | 31 passed |
| Disc importer tests | 6 passed |
| Street profile, 60 FPS target | 1,152 native assertions passed, no failures |
| PSP-1000 profile, 30 FPS default | 1,151 native assertions passed, no failures |
| New menu/dancer assertions | 28 passed in each native run |
| Prepared resources | All 521 integrity-indexed files verified; 489 graphics banks |
| Frozen first setup, no system Python on PATH | Completed fresh disc conversion in 88.646 seconds |
| Compatible-data repeat setup | Verified/reused files in 3.490 seconds |
| Texture caches | No allocation or decode errors in either complete native run |
| Ending on Street profile | Reached final credits; 3,548 displayed frames, 159.266 s audio position, maximum reported lag 0.534 s |
| Ending on 1000 profile | Initial, middle and final decoder samples; full playback was tested on Street |
| Music streaming UI under injected 250 ms reads | 59.94 FPS on Street, 29.97 FPS on 1000; no stream errors |

The assertion suite includes campaign fixtures, progression, combat/save
regressions, settings persistence, old UI preference import, dancer bank choice,
head-loss art, original menu metadata, Extras visibility/navigation, rejected
truncated metadata, crow clip availability, audio and cache lifecycle checks.

Native screenshots were visually reviewed for the home/crow, three Extras
progress states plus a fresh profile, More Ways to Play, both dancers and the
Options layout. The first review caught a cropped crow; its source canvas was
enlarged and its PSP display scale reduced before the final conversion. It also
caught attached envelopes/packets/bonsai left visible without their parent
entry; those now share the unlock gate.

The adversarial dense scene on the PSP-1000 profile averaged only **2.59 FPS**
with its small caches. This is not a representative live-combat measurement,
and passing the assertions does not make that scene performant. It reinforces
the existing experimental-support warning. No new claim of physical PSP-1000
60 FPS compatibility is made. Further dense-scene/storage work is required.

The private prepared package embeds the owner's generated XMB icon/cover and
existing menu preview audio. Its executable payload was compared with the
tested bare EBOOT and is identical. The public archive contains only the bare
executable, Setup.exe, README and third-party notices/licenses (16 files), with
no original game artwork, audio, prepared banks, ISO or reference executable.

[First-setup benchmark details](SETUP-SPEED-0.20.0.md) ·
[Current completion audit](COMPLETION-0.20.0.md)
