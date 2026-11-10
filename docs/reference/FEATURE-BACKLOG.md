# Feature and test backlog — 10 November 2026

This is the known backlog after 0.20.1, based on the current native code and the
owned BLUS30852 resource audit. Some original behaviors still need reference
measurements before an exact implementation can be specified. This list is a
plan, not a claim that the features below have been added.

## Missing functionality

| ID | Feature to add | Acceptance checks |
| --- | --- | --- |
| F1 | Original second Adventure run | Completed-run counter; Dave's original seed restrictions and repeat-run progression; continue/replay/new-game separation; save/reload mid-run; migration of existing completion saves. |
| F2 | Yeti encounter and retreat | Correct second-run encounter conditions, spawn/escape behavior, damage and loot; prevent premature first-run encounters; save before and during an encounter. |
| F3 | Giga-gargantuar | Red-eyed artwork, original health/behavior and valid Survival/Endless selection; damage thresholds, Imp throw, death and save compatibility. |
| F4 | Gold Sunflower and complete trophy progression | Exact combined completion requirements, trophy accounting, award pickup/presentation and persistence; no early or duplicate completion award. |
| F5 | Local house customization | Disc-backed house choices, ownership/purchases, selection and rendering; locked/owned choices and saved appearance. Online house sharing is outside the offline scope. |
| F6 | Tree-unlocked secret cosmetic effects | Original code entry and applicable cosmetic effects; correct Tree-height requirements, enabling/disabling, visual attachments and save/reset behavior. |
| F7 | Playable co-op | Two-player resources, planting/selection, original co-op rules and progression; synchronized wins/losses, pause/restart and save policy. Requires a multiplayer transport. |
| F8 | Playable versus | Plant/zombie player roles, costs, attacks, scoring and original match rules; both role assignments, round resets and completion. Requires a multiplayer transport. |
| F9 | PSP-to-PSP ad-hoc transport for F7/F8 | Lobby, host/join, discovery, compatible-version checks, state/event synchronization, late packets, host/client disconnects and reconnection policy. Test on two Wi-Fi PSP-2000/3000 consoles. Street must hide these modes. |

## Existing gameplay to finish matching

| ID | Area to improve | Acceptance checks |
| --- | --- | --- |
| P1 | Shooter and pult animation transitions | Idle/fire/recoil/recovery continuity for all variants, burst spacing and projectile launch cues; compare original blend curves at 30/60 FPS. |
| P2 | Sun production, Sun-shroom growth and wake-up | Original timing, growth/wake poses, generated-sun position, nighttime behavior and interruption; test day/night and save/reload. |
| P3 | Chomper | Delayed bite, miss/recovery, swallow and digest choreography; target disappears at the correct cue; oversized enemies take the correct bite damage. |
| P4 | Squash | Original look/turn/jump path, impact timing and target loss/escape behavior; damage only at the impact cue. |
| P5 | Mine, explosives and special utility plants | Arming/activation particles, explosion ranges, Kelp pulls, Magnet equipment removal, Blover and Grave Buster transitions; friendly, airborne and underground eligibility. |
| P6 | Supports, upgrades and defensive damage art | Tall-nut/Pumpkin damage stages, Lily Pad/Flower Pot/upgrade interactions, Imitater and Cob cleanup, damage/destruction order and save/reload. |
| P7 | Zombie armor, detached parts and specialized deaths | Correct equipment/limb/head loss for advanced types; burn/crush/drown/vehicle death variants; bodies/parts disappear at the proper time. |
| P8 | Freeze, chill, hypnosis and Garlic transitions | Correct eligibility, interrupted actions, thaw/recovery, friendly targeting, Garlic facial pose and lane-change continuity. |
| P9 | Pole Vaulter, Dolphin and Pogo | Pre-jump, blocked jumps, airborne state, landing and recovery; original trajectories, collision, roof/water cases and save/reload. |
| P10 | Ladder, Screen Door, Snorkel, Digger and Balloon | Shield-loss poses, ladder placement/climbing, water rise/submerge, digging/emergence and balloon pop/landing; transitions during freeze, damage and target loss. |
| P11 | Dancing Zombie group and Jack-in-the-Box | Summon cues, backup formation and synchronized dance phases; Jack's original fuse randomness, pop/explosion cues and freeze/death interruption. |
| P12 | Zomboni, Catapult, Bungee and Bobsled | Crush/shot/flight/drop/crash sequences, team movement/collision, attachments, mower/plant interactions and original timing. |
| P13 | Gargantuar and Imp | Exact throw trajectory, release cue, landing, target/range checks, smash and cancel/interruption rules; preserve one Imp per parent. |
| P14 | Dr. Zomboss | Original scheduling, head/RV/Bungee/spawn/stomp sequences, attachments, fire/ice-ball counters and damage phases; complete fight without softlocks. |
| P15 | Wall-nut Bowling and mowers | Original bounce trajectories, collision boundaries, lane changes, repeated hits, explosive nuts, water/roof cleaners and exits. |
| P16 | Heavy Weapon, Zombotany and Portal Combat | Weapon durations/aiming, plant-head attachments and special attacks, portal routing/cooldown and edge cases; compare each mode with the PS3 reference. |
| P17 | Adventure waves, flags and tutorial pacing | Per-level enemy tables, first spawns, arrival spacing, lane weighting, health-triggered advances, huge/final waves, grave/pool events and progress-bar timing. |
| P18 | Conveyor supply | Original packet weights, drought prevention, supply schedule and full-tray behavior; ensure no invalid/unavailable packet or lost queued arrival. |
| P19 | Puzzle and Survival accuracy | Original layouts/inventories, difficulty variants, enemy composition, long Endless scaling, intermissions and resume; stress native entity/packet limits. |

## Presentation to finish matching

| ID | Area to improve | Acceptance checks |
| --- | --- | --- |
| V1 | Story, Dave and tutorials | Exact dialogue branches/order, expressions, held props, camera direction, tutorial triggers, skip/continue and correct return screen. |
| V2 | Menu scenery and transitions | Original sky/cloud/lightning motion, page/camera transitions and crow choreography; selection remains readable, unlock-aware and responsive. |
| V3 | Rewards and general effects | Seed/trophy/note arrival, bounce/flight/pickup and award panels; particles, shadows, flashes and small effects; bounds and readable text at 480x272. |
| V4 | Gardens and shop choreography | Original tool-use/plant reactions, cosmetic variations and facing, sleep/aquarium presentation, Stinky, coins, moving/selling and shop presentation. |
| V5 | Original audio-event choices and mixing | Correct event variants/cues, positional level, gain and voice priorities; source/30/60 FPS synchronization, clipping, dense combat and streaming stalls. The 194 samples are already imported. |
| V6 | Ending fidelity | Compare full song/video, lyrics, speech bubbles, dancer motion, layers and transitions with the PS3 reference; first/last-frame handling and audio synchronization. Ending playback already exists. |

## PSP and setup acceptance work

| ID | Improvement or verification | Acceptance checks |
| --- | --- | --- |
| Q1 | PSP-1000 dense-scene performance | Reduce cache churn/decoding/storage stalls, use smaller assets or loading budgets where needed; measure live combat and overloaded scenes at both 30/60 targets with sound and unchanged simulation. |
| Q2 | Real PSP startup and menu loading | Time cold boot and first entry/page change on real Memory Sticks; compare storage speeds; verify XMB cover and startup failure messages. |
| Q3 | Suspend/resume and long-session stability | Sleep during gameplay/music/gardening, depleted battery/restart, repeated mode switches and long Endless runs; check audio recovery, handles and memory use. |
| Q4 | Save/profile resilience | Interrupted writes, missing/corrupt primary and backup files, full/read-only card, migration, repeat-run/challenge/garden state and downgrade instructions; avoid silent progression loss. |
| Q5 | First-time setup speed and recovery | Equal-work benchmarks on several PCs, hash/pixel equivalence, bounded RAM, progress estimates, missing FFmpeg, disk-full/cancel/interrupted conversion and Unicode paths. |
| Q6 | Full hardware matrix | Street, 1000, 2000 and 3000 with the same package; model/FPS defaults, fonts/HUD bounds, physical controls, audio and dense gameplay. F7–F9 need two real Wi-Fi consoles. |

The complete GOTY Zombatar creator is a separate conditional expansion: its
original creator resources are not on the supported PS3 disc. Reproducing that
asset set would need another supported, user-owned source. The current PSP
customizer using PS3 artwork already exists.

PSN leaderboards and PlayStation system trophies remain outside the agreed
offline scope. Locally earned trophies are covered by F4.

Suggested order: **F1/F2**, then **F3/F4**, while prioritizing **Q1–Q4** for an
early-access build. After that, work through P/V reference comparisons. Treat
F7–F9 as a larger linked project with separate two-console acceptance testing.

Evidence: src/profile.*, src/platform.h, src/chapter_game.inc, src/extra_rules.inc,
src/game.h, [0.20.0 audit](COMPLETION-0.20.0.md),
[combat/wave checklist](COMBAT-WAVES-0.19.0.md) and
[known limitations](../KNOWN-LIMITATIONS.md).
