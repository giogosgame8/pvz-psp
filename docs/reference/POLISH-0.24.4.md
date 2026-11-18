# Polish and hidden Survival boards — 0.24.4

The disc's challenge table puts Pool Endless (mode 14) on the Survival page.
Day, Night, Fog and Roof Endless (12, 13, 15 and 16) are on the hidden page.
0.24.4 offers those four as an extra page after the normal Pool Endless
unlock. They reuse the terrain-specific boards, round reselection, late
Survival waves and previously reserved profile record slots. Resume uses the
existing saved challenge slot. The normal eleven Survival selections, unlocks
and finite trophies are unchanged. Other unused challenge entries, including
the named prototypes, are outside this patch.

Replay history and audio settings retain the previous valid file and recover
if the primary is corrupt. The file layouts did not change. This is separate
from the existing Game and Profile transaction checks. The live pause,
tile navigation and victory checkpoint fixtures remain in the test suite.
Actual Memory Stick timing and power-loss testing require a PSP.

Late Endless budgets use a wider intermediate calculation. The regression
exercises late waves on all five terrain types without claiming exact PS3 RNG
or difficulty curves. The new Portal Combat and Zombotany checks reproduce
and cover specific interaction bugs. Heavy Weapon weapon rules, Bobsled
movement and full Zomboss and dancer choreography still need comparison.

Dave now shows the dialogue bubble at the same time as the talk animation
and voiced line. Ending playback keeps audio as the primary clock, requests
the next frame early when a buffer is free, and delays closing if the final
frame still needs time to decode. No ending asset reconversion was required.
Original note placement (after 1-9 through 5-9) and the existing seed/trophy
reward behavior remain. A full side-by-side story and reward comparison and
hardware ending sync are still outstanding.

## Crow verification

The original disc's `reanim/mainmenu3/mainmenu.reanim` contains a 12 FPS
`crow2` track using separate menu fly, landing, idle and takeoff sprite
frames. Its Home, More Ways and Extras idle clips hold distinct stationary
poses. Its four directional pan clips have distinct flight keyframes.
Rendering the original idle frames places the crow on the Home gravestone,
the Mini-games gravestone and the Extras glass orb. 0.24.4 uses these objects
for the perches and faces the bird right when it lands. It still uses the
disc's separate `Crow.reanim` artwork and a shared flight curve, so the
takeoff poses and flight timing are not exact. These placements were verified
from the disc animation; the RPCS3 reference run did not reach a visible menu.

## Validation

The final PSP Street and PSP-1000 PPSSPP scene and regression runs each
passed 1,420 checks, with no failures or animation-cache errors. Captures
were compared against all three authored crow idle poses.
The separate full PSP-1000 pass before the perch-only change passed 1,432
checks, including playback through the final credits. It displayed 3,551
frames with a maximum video lag of 0.511 seconds on that run. This measures
the emulator and does not establish a physical PSP gain.

A normal Street emulator launch migrated 13 legacy files, restored the
savedata folder on its own, and recovered the two volume settings from a
backup after deliberately interrupting the test primary. Replay-history
tests also covered a corrupt primary and repair without replacing the
last valid backup. The published save layouts and data recipe are unchanged.
