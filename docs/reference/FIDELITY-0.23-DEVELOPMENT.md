# Fidelity work — 0.23.0 development

11 November 2026. Requested scope: numbered items **4–34**, corresponding to
F1–F6, P1–P19 and V1–V6 in [the completion checklist](COMPLETION-WORK.md).
This is a development checkpoint, **not a declaration that every item now
matches the PS3 game exactly**. Existing implemented features are distinguished
from changes made in this pass below. Multiplayer was outside this request.

## Changes by requested item

| Item | Area | This pass and remaining acceptance |
| --- | --- | --- |
| 4 | Repeat Adventure | Dave's choices use a separate MT19937 stream, original seed ordering and removal without reshuffling the remaining candidates. Retries retain the level seed. The mapping from a PS3 profile to that seed is not yet reproduced. |
| 5 | Yeti | Existing introduction, retreat and diamond rewards retained; headless Yetis no longer start their escape. Added damaged-body artwork. Exact rare-arrival and movement comparisons remain. |
| 6 | Giga | Endless ordinary-wave limits count previous flag-wave arrivals as well. Existing 6000 HP, empty-back poses and red-eye banks retained. All original Endless distributions are not certified. |
| 7 | Gold Sunflower | Existing 48 finite-trophy requirement, one-time collection and save migration retained. Original pickup choreography is still incomplete. |
| 8 | Houses | Free styles and trophy-based tiers replaced with purchased blueprints. Regular house starts at authored tier 3 and has two upgrades; alternate styles have paid renovation levels. House previews, five shop pages, ownership and purchase rollback updated. This remains a local offline use of an originally leaderboard-related feature. |
| 9 | Secrets | Existing seven codes, Tree gates and original artwork retained. Headless bodies no longer continue ordinary attacks. Advanced cosmetic overlays and exact particle choreography remain partial. |
| 10 | Shooters/pults | Cabbage, Kernel, Melon and Winter Melon now wind up, release at their launch cue and finish their shooting clip. Kernel has distinct butter/kernel poses. Crop bounds retain the whole action pose, with the idle source origin as anchor. Full interpolation and every shooter variation still need comparison. |
| 11 | Production/growth/waking | Coffee takes one second to wake its original mushroom and rejects duplicate beans. Sun-shroom plays its growth clip and pauses production during growth. Full original production variation is not signed off. |
| 12 | Chomper | Bite wind-up, cue-time target reacquisition, misses, 40 damage against oversized targets, bite completion, 40-second digestion and separate swallow recovery. Digestion starts after the bite finishes. Swallowed victims leave no external corpse. Saved transitions survive reload. |
| 13 | Squash | Reacquires a valid target at launch and leads moving targets without mutating their live movement state. Prevents execution of a newly recycled zombie slot. Original trajectory/blending still needs visual acceptance. |
| 14 | Utility plants | Kelp grabs, starts dragging at 0.5 s, splashes and finishes at 1 s without an above-water corpse. Magnet disarms Jack, Digger, Pogo, Ladder and armoured targets while retaining their zombie type. Disarmed artwork is separate. Every utility's original effects remain under review. |
| 15 | Supports/upgrades | Lily Pad/Flower Pot placement protection lasts one second. Invalid aquatic placements are rejected. Destruction/shovelling clears pending attack state and cancels aiming for a destroyed Cob Cannon. Existing Pumpkin/Tall-nut layering retained. |
| 16 | Damage/deaths | Corrected advanced health values and added body-damage banks for ten advanced families and disarmed bodies. Added eight detached-head images. Headless humanoids lose health and stop ordinary attacks. All specialised corpses and detached-limb variants are not complete. |
| 17 | Status transitions | Grounded balloon riders can freeze; airborne/falling riders cannot. Headless hypnotised zombies stop biting. New actions share the saved freeze/chill clocks. Complete original status/cancel rules remain open. |
| 18 | Vaults | Pole, Dolphin and Pogo have separate authored durations instead of all using the Pole duration. Old saves migrate remaining vault time. Exact range and trajectory acceptance remains. |
| 19 | Special equipment | Ladder retains its ladder during placement; Digger visibly drills/lands; balloon riders pop/fall/continue on foot; Snorkel rises before biting and dives when its target disappears. All keep their identities. Screen-door damage artwork and every interruption variant remain partial. |
| 20 | Dancer/Jack | Jack has the original 500 body HP, randomized movement-scaled fuse, early-fuse branch, chilled fuse and fixed pop countdown. A stolen box disables it. Existing dancer formation retained; exact original summoning/dance choreography remains incomplete. |
| 21 | Vehicles/Bungee/Bobsled | Vehicles crush defenders. Catapult has 850 HP, twenty basketballs, visible launch/flight and impact-time Umbrella checks. Bungee has descent, three-second wait, grab and ascent; steals one layer, preserves Pumpkin/support and rejects replacement plants using plant identities. Existing Bobsled phases retained. Original carried-plant recovery after killing a rising Bungee and exact Bobsled entry still need work. |
| 22 | Gargantuar/Imp | A full actor pool consumes the throw release and lets its parent recover. A close throw cannot send its Imp backwards toward the zombie entrance. Adventure Imp remains 270 HP; only I, Zombie uses 70. Exact original ballistic path remains open. |
| 23 | Boss | Entering/leaving head is immune to ordinary head-window attacks; Ice can cancel a fireball without damaging/freezing an entering head. Campaign fixture uses real Jalapeño packets during vulnerability. Full original AI/choreography is not complete. |
| 24 | Bowling/mowers | Giant Nut sweep calculations use the same radii throughout. Mowers exclude airborne balloons, underground Diggers and Bungees. Exact original bounce/effect choreography remains open. |
| 25 | PS3 challenges | Corrected Column/Portal/Invisighoul conveyor cadence and Column Flower Pot weight. Existing Heavy Weapon, Zombotany, portals and Bobsled rules remain playable, but **Heavy Weapon's complete PS3 weapon behaviour and exact portal/Zombotany interactions are not finished**. |
| 26 | Waves | Original disc spawn table retained; early Endless Giga cap accounting corrected. No claim that all fifty schedules, lane histories and wave transitions are exact. |
| 27 | Conveyors | Full trays retain an overdue delivery. Storm 4-10 gets the original eight plant types. Grave Busters stop when all graves are covered; Lily/Flower Pot supply responds to planted/tray totals. Boss pots/pults respond to available pots and RV phase. Bowling 2 uses 85:15:15 weights. Original duplicate suppression retained. |
| 28 | Puzzles/Survival | Endless puzzle reset clears delayed attacks, waking, ladders, support state and aim state. Existing finite boards, Survival transitions and saved Endless progression retained. Full PS3 random layouts and late Endless balance remain unverified. |
| 29 | Story | Original short/medium/long/crazy Dave clips, five hand-attached props and shared source anchors; original speech-bubble artwork; TIME/DELAY tokens and word-boundary paging. Full camera sequences, every mouth override and every scripted pause are still incomplete. |
| 30 | Menus | House purchase UI and shop pagination updated; long dialogue stays within its bubble. Existing compact menus retained. Exact original menu camera transitions remain incomplete. |
| 31 | Rewards | Existing original seed/note/trophy types, grass award panels and pickup animations retained. No new claim of exact original collection choreography. |
| 32 | Garden/shop | Blueprints integrated into the shop. Chocolate-fed snail stops at a coin instead of overshooting it. Existing care variants, tools and progression retained. Rare original garden presentations still need comparison. |
| 33 | Audio | New plant release, grab, swallow, ladder, emergence, basketball, stealing and landing cues follow their actions. Existing 194 sample library and dynamic music retained. Complete original event selection/mixing is not certified. |
| 34 | Ending | Rebuilt original-resource movie/song retained and included in full playback tests. This pass does not claim new camera/compositing parity or physical-console A/V certification. |

## Reference evidence

Owned reference is BLUS30852 v01.00, embedding NPUA30059. Original resources,
the executable and disassembly remain private. Numeric facts below were checked
in the supplied executable; addresses are PS3 virtual addresses.

- `0x2d1968`: default body health 270. `0x2d1fb0`: Catapult 850 and its twenty
  projectiles. `0x2d23f4`: Dolphin 500. `0x2d27a0`: Jack 500 and randomized fuse
  initialization. `0x2d2060`: Yeti 1350. `0x2d25d8`: Zomboni 1350.
- `0x2d2d3c–0x2d2d78`: Imp's reduced 70 HP is conditional on I, Zombie,
  rather than the Adventure Imp released by a Gargantuar.
- `0x2a0bf4`: blueprint costs. Regular: 2400 then 3200 wallet units;
  alternates: 800 × (purchase count + 1). One wallet unit displays as $10.
  `0x2a29d4`: regular blueprint sells out after two purchases;
  `0x2a27f4–0x2a282c`: alternate blueprints sell out after five purchases.
  `0x2a457c`: regular-house presentation starts two authored tiers higher.
- `0x1b0d18`: conveyor selection. `0x1b0ea8–0x1b0f18`: level 40's eight
  entries and weights. `0x1b17a0`: Column Flower Pot weight 155.
  `0x1b1230–0x1b12d0`: support-count supply adjustment.
  `0x1b11d0`: Grave Buster supply gate. `0x1b1548`: Bowling 2's 85:15:15.
  Cadence constants are 400/425/500/1000 ticks; single-player multipliers
  match the corresponding challenge branches.
- Clip ranges and attachment tracks come directly from the disc reanimations.
  Dialogue/prop tags come from the disc's UTF-16 LawnStrings resource.

Supporting reconstruction in PvZ-Portable was used to interpret plant action
timelines, equipment transitions, Bungee phases, Dave attachment offsets and
some supply rules. Those interpretations are **not independently certified PS3
behaviour merely because an animation or supporting implementation exists**.
No original game executable or imported artwork is part of the public source.

## Saves and installation

Game writes **version 13**, preserving the complete version-12 prefix for
migration. Profile writes **version 5**; versions 1–4 remain readable. Old
prototype free house selections migrate to ownership derived from purchases.
The current public executable cannot read the new formats. Back up the existing
game folder before testing. Replace EBOOT.PBP and data together and preserve
player saves/settings. Data recipe is **ps3-0230-1**.

The prepared private package contains owned-disc resources. Do not publish it.
Public distribution must remain asset-free. Physical PSP acceptance and exact
PS3 parity are separate from passing automated emulator checks.

Validation: [0.23 test results](VALIDATION-0.23-DEVELOPMENT.md).
