# Combat and wave polish report — 0.19.0

This is the report for the approved C1–C24 and W1–W16 checklist. It records a
completed audit and a set of fixes, **not complete PS3 parity**. “Retained” means
the existing native behavior was reviewed or exercised; it does not mean every
frame, collision boundary or PS3 level table has been independently reproduced.

## Changes delivered

- Freeze is checked before special zombie handlers. Eligible frozen enemies no
  longer advance their attacks or special actions. Chilling, airborne states,
  vehicles, underground Diggers and friendly zombies retain distinct eligibility.
- Advanced zombie poses have their own saved clock instead of the global scene
  clock. Eating transitions restart the relevant clock. Gargantuar smash rendering
  follows the actual attack timer and its recovered 16-frame/second rate.
- Newspaper shield loss now has a stationary gasp before angry movement and its
  angry sound. Jack has a visible pop and delayed explosion. Garlic pauses
  chewing, reacts, changes to an adjacent valid lane and recovers.
- Gargantuar winds up before releasing one Imp. The Imp travels through the air,
  lands, then walks. The passenger is removed from the parent's subsequent
  throw, walking, attacking and dying artwork.
- Dolphin vaulting selects its own disc animation. Balloon eating imports the
  actual eating clip instead of its walking clip.
- Ordinary bodies can use charred deaths for Cherry Bomb, Jalapeño and Doom
  explosions, or water deaths for Kelp. These override interrupted action poses
  and fade before removal. Specialized charred variants are still incomplete.
- A swallowed ordinary zombie no longer leaves a falling corpse outside the
  Chomper. Gargantuar receives a 40-point bite instead of being swallowed whole.
- Cherry, Jalapeño, Mine, Chomper and Kelp targeting no longer catches friendly
  hypnotized zombies in the corrected paths. Ground-only attacks reject relevant
  airborne or underground targets.
- Lobbed shots hit an eligible zombie at their landing position. They cannot
  damage an old target that changed lanes or a distant replacement reusing its
  array slot. Melon/Cob area damage excludes dead and friendly targets.
- Shared cell destruction clears pending Imitater/attack state and both halves
  of a Cob Cannon. It is used by boss destruction, Jack, Gargantuar and Bungee
  removal paths.
- Full zombie pools preserve remaining wave entries instead of dropping them.
  Bobsled teams reserve four slots together; their health includes all riders.
  Final grave arrivals also wait for space. Queued entries and their progress
  survive saving. The progress meter retains completed waves during a queue.
- Survival intermissions advance dying bodies and effects once per simulation
  step, and new rounds clear previous transition state.
- Saves use version 11, retain version 10 import and existing older migrations,
  validate new state, and keep a previous-save backup. If the primary file is
  missing during an interrupted final rename, loading can recover that backup.
  An existing corrupt primary is still rejected.
- Ending preparation retries short Windows permission/sharing failures around
  temporary chunk publication, reads and cleanup. Retries are bounded; persistent
  failures and invalid frame data still fail rather than producing a usable-looking
  partial file.

## Evidence and validation

The owned BLUS30852 disc was the resource source. Its SHA-256 is
`50ecf8b9a8ccbba80bbbba83d9456c85f3080c33039a1f7424261235faaf1053`.
No RPCS3 session was needed for this pass. No original executable, disassembly,
images, music or prepared game data are included in the public source/release.

Relevant facts checked in the private PS3 executable:

| Behavior | Evidence |
| --- | --- |
| Gargantuar smash and Imp release | Normalized cue tests at `0x2cf944` (.64) and `0x2cf9a8` (.74), calling the previously identified animation-event helper `0x2d9a48`. Smash selects `anim_smash` at 16 authored frames/second; throw selects `anim_throw` at 24. |
| Newspaper gasp | `anim_gasp` selected around `0x2c7008–0x2c7020` with rate 8. |
| Jack pop | `anim_pop` selected around `0x2d060c–0x2d0614` with rate 28. Countdown setup stores 110 at `0x2d0590–0x2d0598`; the surprise threshold compares 80 at `0x2d0454`. |
| Garlic reaction milestones | The handler compares 70, 170 and 270 around `0x2c6754–0x2c6768`. Supporting source was also used to interpret the milestones. |
| Chewing and eating rates | The prior 0.18.9 recovery and its regression suite are retained. See CHANGES-0.18.9.md. |

These facts do not verify all movement curves, blending durations, particles or
target-selection details. Some native durations and movement paths remain
approximations. Supporting PC reconstruction source is not treated as proof of
PS3-specific behavior.

The data recipe is `ps3-0190-1`, containing 391 graphics banks. Compared with the
previous prepared set, 378 banks are byte-identical, Balloon eating changes, and
12 transition banks are added. Existing pose anchors already passed the shared
canvas-origin check; this update does not claim to have changed those 378 banks.
The preparation regression verifies that anchor handling cannot alter compressed
frame pixels or separate the customizer's body and face anchors.

Standalone setup completed a fresh conversion in **153.946 seconds**, then
verified/reused it in **3.708 seconds** on this PC with neither Python nor the
`py` launcher on PATH. All 421 integrity-indexed files passed verification.
The generated launcher contains the owned icon and background. These are local
measurements, not an average setup time or physical-console startup benchmark.

The final executable passed **1,124 checks in the Street profile (60 FPS target)**
and **1,123 in PSP-1000 (30 FPS target)**, with zero failures and zero cache
errors. This includes 62 additional native checks: 48 gameplay/state cases,
13 presentation cases and one additional audio-event case. All 27 preparation
tests and six importer tests passed. The Street run played the entire ending;
the PSP-1000 run checked initial, middle and final decoding plus the UI suite.
These functional profile runs are not performance certification.

Bare executable SHA-256:
`8b60366b9c2509505bd6a0c9abb7302b47beba295428272a8be74a87f5e18a83`.

Native and Python results are recorded with the local release validation files.
Native testing uses PPSSPP profiles, including Street and PSP-1000. Physical
PSP performance, memory-card I/O, sound output and WLAN have not been certified
by this pass. A 60 FPS target is not a guarantee on PSP-1000.

## Complete combat checklist

| ID | Result of this pass | Remaining limit |
| --- | --- | --- |
| C1 | Retained shared pose anchors; saved advanced pose clocks and freeze checks improve walk/eat/action continuity. | Not every specialized pose switch has frame-by-frame PS3 comparison. |
| C2 | Retained Peashooter/Snow Pea recoil blending and delayed Repeater/Gatling bursts; existing shot regressions rerun. | All shooter variants and their exact original blend curves are not certified. |
| C3 | Fixed physical lob landing, lane changes, recycled target slots and friendly splash targets. | Pult windup/launch/recovery choreography still needs closer matching. |
| C4 | Retained Sunflower generation presentation, Sun-shroom growth and sleep/wake rules. | Exact original growth/wake choreography remains partial. |
| C5 | Removed external corpses after swallowing and corrected Gargantuar bite damage. | Chomper's full delayed bite, miss, digest and swallow choreography remains partial. |
| C6 | Retained Squash look/turn/jump/impact phases and impact-time damage; existing regressions rerun. | Original trajectory and fine timing are not certified. |
| C7 | Hardened Mine targeting, ice eligibility and explosive friendly-target filtering; added ordinary charred corpses. | Activation particles and specialized deaths are incomplete. |
| C8 | Corrected Kelp targeting and ordinary water death; retained Magnet, Grave Buster and Blover rules. | Tool/plant-specific transition animation and all equipment-removal variants remain partial. |
| C9 | Destruction clears pending Imitater and two-cell Cob state; migration tests retained. | Every upgrade/support edge case has not been compared with PS3. |
| C10 | Retained nut/Pumpkin damage/support rules; unified heavy destruction cleanup. | Dedicated Tall-nut/Pumpkin damage art and all destruction-order nuances need work. |
| C11 | Retained existing armor, arm/head loss and head-particle checks; advanced poses now use independent clocks. | Advanced enemy attachment and limb-loss coverage is incomplete. |
| C12 | Added ordinary charred/water deaths, action interruption and fades; swallowed enemies have no external corpse. | All specialized burn, crush, drown and vehicle deaths are not reproduced. |
| C13 | Fixed freeze ordering, saved chilled pose clocks, eligibility and Garlic reaction/lane changes. | Exact original facial art, every hypnosis interaction and thaw transition remain partial. |
| C14 | Dolphin now uses its authored jump clip; retained existing vault rules. | Pole/Dolphin/Pogo pre-jump, blocked-jump and landing choreography still differs. |
| C15 | Added Newspaper gasp/recovery with delayed anger sound. Existing Door/Ladder rules retained. | Ladder placement/climb and dedicated shield-loss animation remain partial. |
| C16 | Fixed actual Balloon eating resource and surfaced Digger eat-clock transitions. | Snorkel entry/rise, Digger emergence/walk and Balloon pop/landing need fuller state/art coverage. |
| C17 | Added Jack pop/surprise/explosion sequence and freeze handling. Existing Dancer/backup rules retained. | Dancer summons, synchronized group choreography and original Jack fuse randomness remain partial. |
| C18 | Special handlers respect eligible freeze; Bobsled wave admission keeps complete teams; destruction cleanup covers Bungee. | Zomboni crushing, Catapult shots, Bungee flights and Bobsled crash/choreography need closer matching. |
| C19 | Added timed throw, one Imp, flight/landing and empty-back poses; smash uses recovered rate/cue. | Imp trajectory and every range/target/cancel rule are not exact PS3 reproductions. |
| C20 | Boss destruction uses consistent pending-state cleanup; existing boss regressions retained. | Full original boss action scheduling, RV/Bungee/spawn/stomp choreography remains partial. |
| C21 | Existing bowling hit, bounce, lane and impact-effect regressions retained. | Exact PS3 trajectories and all collision nuances remain unverified. |
| C22 | Existing portal cooldown and Zombotany interaction checks retained. | Original attack variants, attachments and all portal edge cases remain partial. |
| C23 | Existing mower trigger/collision/exit and water/roof checks retained. | Every original collision boundary has not been measured. |
| C24 | New saved action/pose/queue state, version 10 migration, malformed-state rejection and rename-gap recovery checks. Zero-delta updates cannot advance actions. | Power-loss safety is not a filesystem durability guarantee; downgrades cannot read version 11 saves. |

## Complete wave checklist

| ID | Result of this pass | Remaining limit |
| --- | --- | --- |
| W1 | Existing 50-level campaign schedule checks rerun. | No new claim that every native wave table equals the PS3 table. |
| W2 | Retained existing first-spawn/tutorial conditions and their tests. | All original tutorial presentation triggers are not certified. |
| W3 | Retained existing type introduction/unlock rules. | Original second-Adventure and Yeti progression are still missing. |
| W4 | Ordinary native wave ramp/timers retained; entries no longer disappear at capacity. | Exact per-level PS3 composition remains partly unverified. |
| W5 | Preserved ordinary spawn timing and positions; full pools queue entries and reserve Bobsled teams. | Original per-zombie arrival grouping/spacing is not fully reproduced. |
| W6 | Retained lane weighting, active-lane and land/water placement rules. | Complete PS3 lane-selection equivalence is not certified. |
| W7 | Early-wave health excludes friendly hypnotized zombies; queued Bobsled health includes every rider. Existing delay rules retained. | Exact PS3 health accounting for all special enemies remains unverified. |
| W8 | Retained huge-wave warning/countdown and the prior recovered timing. | Complete banner/audio/camera presentation is not frame-exact. |
| W9 | Retained existing flag/final wave planning; full pools cannot silently drop entries. | Original final-wave composition and density still need table-level comparison. |
| W10 | Final grave arrivals now survive pool exhaustion. | All pool/grave/special-event timings are not independently PS3-verified. |
| W11 | Conveyor supply and existing selection/packet tests retained. | Original item weights, drought-prevention rules and all schedules remain partial. |
| W12 | Mini-game gameplay/schedule regression suite rerun. | Especially Heavy Weapon, Bobsled and Zombotany retain fidelity gaps. |
| W13 | Intermission corpse/effect timing corrected; new rounds clear transition state and remain saveable. | Exact original Endless composition, scaling and Giga-gargantuar remain incomplete. |
| W14 | Retained death/reward gating and tests; pending wave entries prevent premature completion. | Original reward/camera timing still has differences. |
| W15 | Queued waves retain completed meter progress; existing continuous progress/final-wave tests retained. | The meter is a handheld reconstruction, not proof of identical original interpolation. |
| W16 | New action and queue persistence tested; fixed-step simulation and prior 30/60/uneven-frame audio tests retained. | No physical-hardware 60 FPS guarantee or complete long-session pacing certification. |

## Installing this update

Use the **0.19.0 executable and prepared data together**. Run the new Setup.exe
against the supported original PS3 ISO, wait for READY, then copy EBOOT.PBP and
data into PSP/GAME/PVZPSP. Keep existing save/profile/customizer files, and back
them up first. Previous data is rebuilt because this version needs new clips.
Version 10 saves import; after saving in version 11, older builds cannot read
that newer save. Keep the pre-update backup for downgrading.

The locally prepared ready-to-copy package contains the owner's disc resources
and is for that owner's installation. Public GitHub downloads remain asset-free.
