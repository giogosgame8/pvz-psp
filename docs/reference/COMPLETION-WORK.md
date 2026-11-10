# Completion work — development checkpoint

Requested 10 November 2026. Baseline: 0.20.1 / 861e393.
Branch: `completion/full-checklist`. This is **not a completed 40-item release**.

The entire checklist remains in scope. The table distinguishes implemented work,
prototypes and work that has not started. A passing emulator test does not prove
PS3 timing parity or physical PSP acceptance.

| Item | Status | Delivered / remaining |
| --- | --- | --- |
| F1 | Implemented core; fidelity review pending | Saved completed-run count; all 50 repeated stages; three biome-filtered, locked Dave packets; stable retry/continue; repeated money bags; original repeat dialogue branches; earned shovel retained. Exact PS3 RNG and repeat-wave comparisons remain. |
| F2 | Implemented; reference review pending | Second 4-10 introduction, later rare eligibility, 1350 HP, 15–20 s retreat timer, freeze interruption, rightward escape, four diamonds on a kill, saved/recycled-slot checks. Timing/eligibility interpretation still needs the corresponding PS3 instruction audit. |
| F3 | Implemented; reference review pending | Eight red-eyed/empty-back clips; 6000 HP; half-health Imp throw; Endless candidate and saved per-arrival variants. First-allowed wave 15 and weight 6000 are disc-table facts; flag-scaled caps currently use supporting reconstruction. |
| F4 | Implemented; presentation review pending | Forty-eight finite challenge trophies, excluding Endless records; combined Gold requirement from disc text; one-time award pickup and wallet cap; repeated-run counter and migration. Full original award choreography remains. |
| F5 | Prototype only | Five disc-backed styles × five authored tiers; local selection persists. All styles currently available. The local 12-finite-trophies-per-tier rule is a PSP adaptation, not a recovered PS3 ownership/purchase/unlock rule; not signed off as original behavior. |
| F6 | Partially implemented | Seven case-insensitive codes with Tree gates at 100/500/1000; actual dance/mower/face/moustache/candy/daisy artwork; Sukhbir routes ambient groans through the original weighted Foley group. Advanced-head coverage and exact original particles remain incomplete. |
| F7 | Pending | Playable co-op has not been implemented by this pass; remains hidden. |
| F8 | Pending | Playable versus has not been implemented by this pass; remains hidden. |
| F9 | Pending | No working ad-hoc transport is claimed. Requires implementation plus two-console acceptance on Wi-Fi PSP-2000/3000. |
| P1 | Pending polish | Existing baseline retained and regression-tested; the full original-behavior comparison remains open. |
| P2 | Pending polish | Existing baseline retained and regression-tested; the full original-behavior comparison remains open. |
| P3 | Pending polish | Existing baseline retained and regression-tested; the full original-behavior comparison remains open. |
| P4 | Pending polish | Existing baseline retained and regression-tested; the full original-behavior comparison remains open. |
| P5 | Pending polish | Existing baseline retained and regression-tested; the full original-behavior comparison remains open. |
| P6 | Partially improved | Tall-nut cracked stages and Pumpkin damaged fronts added; Pumpkin rear/front layering surrounds its occupant, including Imitater variants. Other support, upgrade, blink and cleanup acceptance remains open. |
| P7 | Partially improved | Shared source-canvas anchors now applied to 81 clips/components (66 pre-existing banks change only their anchors). Advanced detached parts/death variants still incomplete. |
| P8 | Pending polish | Existing baseline retained and regression-tested; the full original-behavior comparison remains open. |
| P9 | Pending polish | Existing baseline retained and regression-tested; the full original-behavior comparison remains open. |
| P10 | Pending polish | Existing baseline retained and regression-tested; the full original-behavior comparison remains open. |
| P11 | Pending polish | Existing baseline retained and regression-tested; the full original-behavior comparison remains open. |
| P12 | Pending polish | Existing baseline retained and regression-tested; the full original-behavior comparison remains open. |
| P13 | Partially improved | Giga uses its own Imp threshold and correct empty-back artwork. Existing throw/land regressions retained; exact trajectory and cancel rules still need review. |
| P14 | Pending polish | Existing baseline retained and regression-tested; the full original-behavior comparison remains open. |
| P15 | Pending polish | Existing baseline retained and regression-tested; the full original-behavior comparison remains open. |
| P16 | Pending polish | Existing baseline retained and regression-tested; the full original-behavior comparison remains open. |
| P17 | Partially improved | Owned PS3 spawn records replace approximate weights/points; recovered first-allowed waves now gate random arrivals. Exact per-level schedules, lane rules and all repeat-run comparisons remain open. |
| P18 | Pending polish | Existing baseline retained and regression-tested; the full original-behavior comparison remains open. |
| P19 | Partially improved | Saved Giga arrivals, flag-scaled Endless caps/earlier eligibility and plain/cone weight decay; original puzzle and all Endless layouts/limits are not signed off. |
| V1 | Partially improved | Repeat seed-picking/bowling/Bungee branches use the original disc dialogue keys; first-time tutorial scenes are suppressed on repeated Adventure where appropriate. Complete camera, props and timing remain open. |
| V2 | Pending polish | Baseline presentation retained and regression-tested; complete PS3 comparison remains open. |
| V3 | Partially improved | Gold trophy and money-bag artwork/panels; house/player/code screens captured and checked at 480×272. Full reward/effect choreography remains open. |
| V4 | Pending polish | Baseline presentation retained and regression-tested; complete PS3 comparison remains open. |
| V5 | Partially improved | Secret voice mode now reaches ambient groans and uses the disc Foley group. The complete original event-selection/mixing audit remains open. |
| V6 | Regression tested | Complete original ending playback was rerun on Street and on the smaller PSP-1000 emulator profile; exact visual comparison and hardware synchronization still need acceptance. |
| Q1 | Improved; hardware unverified | Frame-cache allocation changed from 32 KiB to 4 KiB blocks at the same 12/24 MiB budgets. 512 simultaneous small frames retain exact pixels. PSP-1000 emulator live Day 59.83 FPS / Pool 59.55 FPS; 30 FPS remains the safe default and 60 remains selectable. |
| Q2 | Pending hardware measurement | No physical Memory Stick cold-boot timing is claimed. Diagnostic startup logs include the full test suite and are not normal startup benchmarks. |
| Q3 | Partial regression coverage | Dense combat, mode changes, audio, cache eviction and full ending are exercised in emulator runs. Physical suspend/resume, battery interruption and long-session tests remain. |
| Q4 | Partially improved | Game/Profile transactions validate and close temporary data, retain a previous checkpoint, recover missing/corrupt primaries, preserve good backups during repair and reject overwriting a newer format. Save failures warn and a failed profile commit does not advance the on-disk Adventure. Replay-history/settings recovery, disk-full/power-cut hardware tests remain. |
| Q5 | Regression tested; wider benchmarks pending | Standalone frozen setup successfully builds from the owned ISO with Python absent from PATH; integrity verification and anchor/pixel comparisons pass. New garden sub-preparer restores main animation specs even on failure. Equal-work multi-PC and disk-full/cancel acceptance remain. |
| Q6 | Hardware acceptance pending | Street/1000 emulator suites and model-detection assertions are not a four-model hardware certification. No two-console WLAN acceptance is claimed. |

## Compatibility and private resources

Game saves now write version 12 (version 11 and earlier migrations retained).
Profiles write version 4 (versions 1–3 retained). Back up existing saves before
trying a development build; the older public 0.20.1 executable cannot read these
newer save formats. New prepared assets are also required: recipe `ps3-0210-2`.

Original GOTY creator resources are absent from the supported PS3 disc. The
existing PS3-art customizer is retained. PSN services and system trophies remain
outside the agreed offline scope. Private disc resources and the original
executable must never enter a public source or release package.

Numeric spawn-table evidence is recorded in [SPAWN-FACTS-0.21.json](SPAWN-FACTS-0.21.json).
The source executable and its disassembly stay private. Supporting reconstruction
is explicitly identified where it interprets behavior not yet traced in PS3 code.

Validation and compatibility details: [VALIDATION-0.21-DEVELOPMENT.md](VALIDATION-0.21-DEVELOPMENT.md).
