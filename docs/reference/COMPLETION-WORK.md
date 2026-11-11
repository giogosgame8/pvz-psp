# Completion work — development checkpoint

Requested 10 November 2026. Baseline: 0.20.1 / 861e393.
Branch: `completion/full-checklist`. This is **not a completed 40-item release**.

The entire checklist remains in scope. The table distinguishes implemented work,
prototypes and work that has not started. A passing emulator test does not prove
PS3 timing parity or physical PSP acceptance.

| Item | Status | Delivered / remaining |
| --- | --- | --- |
| F1 | Improved; fidelity review pending | Separate MT19937 stream, original seed ordering and stable candidate removal now drive Dave choices. All previous repeated-run functionality retained. PS3 profile-to-level-seed mapping and complete repeat-wave comparisons remain. |
| F2 | Implemented; reference review pending | Second 4-10 introduction, later rare eligibility, 1350 HP, 15–20 s retreat timer, freeze interruption, rightward escape, four diamonds on a kill, saved/recycled-slot checks. Timing/eligibility interpretation still needs the corresponding PS3 instruction audit. |
| F3 | Implemented; reference review pending | Eight red-eyed/empty-back clips; 6000 HP; half-health Imp throw; Endless candidate and saved per-arrival variants. First-allowed wave 15 and weight 6000 are disc-table facts; flag-scaled caps currently use supporting reconstruction. |
| F4 | Implemented; presentation review pending | Forty-eight finite challenge trophies, excluding Endless records; combined Gold requirement from disc text; one-time award pickup and wallet cap; repeated-run counter and migration. Full original award choreography remains. |
| F5 | Paid ownership implemented; offline adaptation | Free/trophy-tier prototype replaced by purchased blueprints, PS3 regular costs and two-upgrade limit, alternate renovation levels, owned previews and five shop pages. The original leaderboard context is an offline local adaptation. See the 0.23 report. |
| F6 | Partially implemented | Seven case-insensitive codes with Tree gates at 100/500/1000; actual dance/mower/face/moustache/candy/daisy artwork; Sukhbir routes ambient groans through the original weighted Foley group. Advanced-head coverage and exact original particles remain incomplete. |
| F7 | Pending | Playable co-op has not been implemented by this pass; remains hidden. |
| F8 | Pending | Playable versus has not been implemented by this pass; remains hidden. |
| F9 | Pending | No working ad-hoc transport is claimed. Requires implementation plus two-console acceptance on Wi-Fi PSP-2000/3000. |
| P1 | Improved; comparison pending | Pults now have saved launch/recovery phases and distinct Kernel butter poses, with source-canvas action anchors. Remaining shooters, blending and exact target/trajectory rules are not fully signed off. |
| P2 | Improved; comparison pending | Coffee has a one-second wake transition and duplicate-bean protection. Sun-shroom growth pauses production and selects its growth clip. Complete production variation remains open. |
| P3 | Transitions implemented; reference acceptance pending | Chomper has bite wind-up, reacquisition, misses, large-target damage, bite completion, digestion and separate swallow recovery. Save/reload and range-loss regressions added. |
| P4 | Improved; visual acceptance pending | Squash reacquires at launch and predicts movement without altering the live target. Exact source trajectory and blending remain open. |
| P5 | Improved; remaining utilities pending | Kelp has grab/drag/pull phases and identity checks. Magnet preserves zombie types and removes equipment with bare-body artwork. Full original effects and all utility plants remain under review. |
| P6 | Improved; remaining acceptance pending | Existing defensive damage layers retained; one-second support placement protection, aquatic placement restrictions and Cob/attack cleanup added. |
| P7 | Improved; specialised coverage incomplete | Advanced health values, headless behaviour, ten advanced damaged-body families, disarmed variants and eight more detached-head images added. Specialised corpses and limbs remain partial. |
| P8 | Improved; comparison pending | Grounded balloon riders regain freeze eligibility; headless hypnotised zombies stop attacking; new transitions respect saved freeze/chill clocks. Full status/cancel parity remains open. |
| P9 | Improved; trajectory acceptance pending | Pole, Dolphin and Pogo now have separate clip durations; older vault saves migrate. Exact vault distances and trajectories remain open. |
| P10 | Improved; variants incomplete | Ladder placement, balloon falling/ground travel, Digger emergence and Snorkel rise/dive added without type conversion. Remaining shield and interruption artwork remains open. |
| P11 | Partially improved | Jack randomized fuse, original 500 HP and disarmed-box behaviour added. Existing dancer formation remains; exact original dance/summoning choreography is incomplete. |
| P12 | Partially improved | Vehicles crush plants; Catapult launches visible basketballs with twenty ammo; Bungee waits, grabs one layer and ascends with identity protection. Bobsled choreography and recovery of a stolen plant from a killed rising Bungee remain open. |
| P13 | Partially improved | Existing Giga/Imp artwork retained. A full actor pool no longer traps the parent in its throw; close throws cannot travel backwards. Adventure and I, Zombie Imp HP are separate. Original ballistics remain open. |
| P14 | Partially improved | Entering/leaving head immunity corrected, including Ice/fireball cancellation. Full original boss AI/choreography remains unfinished. |
| P15 | Partially improved | Giant Nut sweep uses consistent radii; mower target exclusions corrected. Exact original bounce/effect choreography remains open. |
| P16 | Partially improved; PS3 modes incomplete | Column/Portal/Invisighoul conveyor cadence and Column supply corrected. Heavy Weapon full weapon rules and all Zombotany/portal interactions are not complete PS3 reproductions. |
| P17 | Partially improved | Owned PS3 spawn records replace approximate weights/points; recovered first-allowed waves now gate random arrivals. Exact per-level schedules, lane rules and all repeat-run comparisons remain open. |
| P18 | Improved; broader acceptance pending | Original level-40 plant set, overdue full-tray delivery, support/grave-aware supply, boss supply adjustments and Bowling-2 weights implemented. Numeric PS3 evidence is in the 0.23 report. |
| P19 | Partially improved | Previous Survival/Giga work retained. Endless puzzle resets clear delayed attacks, waking, support, ladders and aim state. Exact random layouts and all Endless balance remain unverified. |
| V1 | Partially improved | Original Dave talk poses, five hand-attached props, speech bubble, TIME/DELAY cues and readable dialogue pages added. Complete camera sequences, mouth overrides and scripted timings remain open. |
| V2 | Partially improved | House purchases and shop pagination updated; dialogue fits its original bubble. Full original menu camera transitions remain open. |
| V3 | Partially improved | Gold trophy and money-bag artwork/panels; house/player/code screens captured and checked at 480×272. Full reward/effect choreography remains open. |
| V4 | Partially improved | Existing garden variants/tools retained. Snail coin approach no longer overshoots. Blueprints integrate with the shop. Rare original presentations still need comparison. |
| V5 | Partially improved | Secret voice mode reaches ambient groans and uses the disc Foley group. 0.22 adds pressure-driven main/percussion/hi-hat arrangements for five level themes, plus the dedicated night alternate. Exact PS3 fractional beat-phase matching and the complete event-selection/mixing audit remain open; see [dynamic music](DYNAMIC-MUSIC-0.22.md). |
| V6 | Regression tested | Complete original ending playback was rerun on Street and on the smaller PSP-1000 emulator profile; exact visual comparison and hardware synchronization still need acceptance. |
| Q1 | Improved; hardware unverified | Frame-cache allocation changed from 32 KiB to 4 KiB blocks at the same 12/24 MiB budgets. 512 simultaneous small frames retain exact pixels. PSP-1000 emulator live Day 59.83 FPS / Pool 59.55 FPS; 30 FPS remains the safe default and 60 remains selectable. |
| Q2 | Pending hardware measurement | No physical Memory Stick cold-boot timing is claimed. Diagnostic startup logs include the full test suite and are not normal startup benchmarks. |
| Q3 | Partial regression coverage | Dense combat, mode changes, audio, cache eviction and full ending are exercised in emulator runs. Physical suspend/resume, battery interruption and long-session tests remain. |
| Q4 | Partially improved | Game/Profile transactions validate and close temporary data, retain a previous checkpoint, recover missing/corrupt primaries, preserve good backups during repair and reject overwriting a newer format. Save failures warn and a failed profile commit does not advance the on-disk Adventure. Replay-history/settings recovery, disk-full/power-cut hardware tests remain. |
| Q5 | Regression tested; wider benchmarks pending | Standalone frozen setup successfully builds from the owned ISO with Python absent from PATH; integrity verification and anchor/pixel comparisons pass. New garden sub-preparer restores main animation specs even on failure. Equal-work multi-PC and disk-full/cancel acceptance remain. |
| Q6 | Hardware acceptance pending | Street/1000 emulator suites and model-detection assertions are not a four-model hardware certification. No two-console WLAN acceptance is claimed. |

## Compatibility and private resources

Game saves now write version 13 (version 12 and earlier migrations retained).
Profiles write version 5 (versions 1–4 retained). Back up existing saves before
trying a development build; the older public 0.20.1 executable cannot read these
newer save formats. The 0.23 transition/story work requires new prepared assets: recipe
`ps3-0230-1` (0.22 used `ps3-0220-1`).

Original GOTY creator resources are absent from the supported PS3 disc. The
existing PS3-art customizer is retained. PSN services and system trophies remain
outside the agreed offline scope. Private disc resources and the original
executable must never enter a public source or release package.

Numeric spawn-table evidence is recorded in [SPAWN-FACTS-0.21.json](SPAWN-FACTS-0.21.json).
The source executable and its disassembly stay private. Supporting reconstruction
is explicitly identified where it interprets behavior not yet traced in PS3 code.

Validation and compatibility details: [VALIDATION-0.21-DEVELOPMENT.md](VALIDATION-0.21-DEVELOPMENT.md).
The adaptive-music follow-up is validated in
[VALIDATION-0.22-DEVELOPMENT.md](VALIDATION-0.22-DEVELOPMENT.md).

The requested items 4–34 receive a detailed, qualified status report in
[FIDELITY-0.23-DEVELOPMENT.md](FIDELITY-0.23-DEVELOPMENT.md).
