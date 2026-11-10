# Validation — 0.21.0 development checkpoint

10 November 2026. This is a tested development checkpoint, not completion of the
40-item backlog. See [COMPLETION-WORK.md](COMPLETION-WORK.md) for every item's
actual status and remaining acceptance work.

## Native PSP executable

Bare EBOOT SHA-256: `2f55d8e2d0daf0ab4a5235e5f12da6d375aefe8d91187e066d06714e4f8ebc6e`.

| Emulator profile | Target | Full checks | Failures | Cache errors | Ending audio |
| --- | --- | --- | --- | --- | --- |
| Street | 60 FPS | 1,234 | 0 | 0 | 159.266 s |
| PSP-1000 | 30 FPS default | 1,234 | 0 | 0 | 159.289 s |

Both runs used that exact executable and the same prepared data. Full ending
playback reached the final credits: 3,546 displayed frames on Street and 3,402 on
PSP-1000, maximum measured audio/video lag 0.534 s. Ending/frame counts are
playback diagnostics, not a claim that every original PS3 visual has been matched.

The final executable's additional isolated PSP-1000 60 FPS live run measured:

| Live scene | Measured FPS | Draw CPU time | Simulation | Kills / replacements | Alive at end |
| --- | --- | --- | --- | --- | --- |
| Day | 59.83 | 2.062 ms | 66.984 s / 4,019 fixed updates | 39 / 39 | 30 |
| Pool | 59.55 | 1.633 ms | 66.016 s / 3,961 fixed updates | 37 / 37 | 42 |

The live run includes animated actors, damage/effects and streamed audio. Its
final combined UI/cache run passed **1,235 checks**, with zero failures and zero
cache errors; the private validation JSON records its complete output. No
physical PSP-1000, Memory Stick, suspend/resume or two-console radio acceptance is
claimed. The 1000 retains its 30 FPS default; 60 FPS remains a user-selectable test.

The new allocator keeps the same 12/24 MiB decoded-frame budgets, using 4 KiB
blocks instead of 32 KiB. A regression holds 512 small frames simultaneously and
compares every pixel. PVA3, PVA4 and grayscale layout checks also pass. The old
heavily populated 1000 emulator scene was about 2.6 FPS; this is a local emulator
comparison and is not a physical-console performance guarantee.

## Preparation and resources

- 32 preparation/setup tests and 6 importer tests pass.
- Fresh standalone setup from the owned supported ISO: **157.417 s** on this PC.
- Verify/reuse of the same data: **3.638 s**. Python and the py launcher were absent
  from the child process PATH. These are local timings, not a user-PC average.
- Recipe `ps3-0210-2`: **522 graphics banks, 554 integrity-indexed files**.
- Of 489 baseline banks, 66 change only their anchor bytes; the derived zombie
  gallery is regenerated. Existing compressed frame pixels otherwise remain
  unchanged. New banks supply Giga/secret/defensive/house/award presentation.
- Main animation specifications are restored after garden preparation, including
  exception paths; shared anchors are now applied to 81 clips/components. This
  corrects the production-path coverage overstated in the 0.19.0 report.
- The private XMB PBP has a 144×80 icon, 480×272 cover and AT3 audio. Its executable
  payload is byte-identical to the tested bare PBP.

## Saves and delivery

Game format 12 imports format 11 and the retained older migrations. Profile format
4 imports formats 1–3. Tests cover repeat-run state, locked packets, retry/continue,
Yeti/Giga state, finite trophy counting, one-time award collection, Tree code gates,
missing/damaged primaries, preserved backups, ignored unfinished temporary files,
newer-format protection, and failed-write feedback without advancing Adventure.

Back up the complete existing installation before trying the development build.
The older public executable cannot read the newly written save/profile formats.
The private package includes prepared resources from the owner's disc and must
not be used as the asset-free public release.

Public main/release is unchanged. No RPCS3 session was opened for this pass.
