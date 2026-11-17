# Gameplay polish — 0.24.0 development

17 November 2026. This pass addresses requested improvements 2, 3, 4 and 6:
animation transitions, wave pacing, boss/special modes, and rewards/story.
It is a tested development checkpoint, not complete PS3 parity.

## Animation transitions

- Thrown Imps now follow a continuous gravity arc from their release position,
  retain altitude and initial vertical velocity in saves, and play their landing
  recovery before walking. Roof height is included in the trajectory.
- Disco uses the disc's moonwalk, pointing and arm-raise clips. Backup dancers
  rise into view, and the group alternates walking and raising its arms with
  the original 23-beat pattern. Pointing holds its final pose instead of replaying.
- Dancers and backups use the original 0.8 character scale. Each leader owns
  four backup slots through actor generation IDs. Nearby groups cannot share
  another leader's backups, and recycled actor slots cannot adopt an old group.
- New transition banks interpolate source transforms offline between authored
  poses. Image and visibility changes stay discrete. Existing banks retain their
  original format; this is not a global skeletal animation/blending system.
- The classic dancer choice is retained. The disc's classic artwork has an
  arm-raise clip but lacks the replacement Disco moonwalk/point sequences; its
  available poses are used rather than fabricating missing original artwork.

## Waves and arrivals

- Repeat Adventure, later Survival rounds and Last Stand use their own point
  budgets. Last Stand uses its actual stage counter. Removed the artificial
  ten-wave addition previously applied to Endless budgets.
- Corrected mode multipliers, introduced-zombie placements, Column's fixed
  Ladder/Jack/Gargantuar groups and Adventure-only final-wave roster completion.
- Shields contribute one fifth of their remaining health to early-wave release;
  ordinary helmets contribute their full health. Friendly and carried actors
  do not hold up the next wave.
- Short-wave modes use their separate interval. The huge-wave voice follows
  its warning cue; final-wave audio is delayed instead of playing immediately.
- Final night grave, pool and roof arrivals wait two seconds. Admission remains
  queued when the actor pool is full, and winning/advancing waits for the queue.
  Roof reinforcements descend as linked Bungee passengers and detach on landing.
- Ordinary pool zombies can use water lanes after the opening five waves.
  Swimming Coneheads/Bucketheads retain armor, damaged armor artwork and Magnet
  interaction. A rake cannot force an aquatic zombie into a dry lane.

These rules use imported campaign tables, owned animation data, executable
observations and a supporting reconstruction. Native random sequences, bounded
actor storage and coordinate projection remain different; all fifty schedules
have not been certified against PS3 recordings.

## Boss and special modes

- Zomboss has actual idle recovery pauses, attack priority and cue timings tied
  to clip lengths. Stomps target planted row pairs. Bungee attacks retain their
  carriers' identities, wait for their state and play the separate exit clip.
  RV impacts reset the relevant attack delays.
- Boss balls wait through formation time, then follow their source ground-motion
  profile. They crush plants/mowers and leave the screen; reaching the left edge
  no longer causes an immediate loss. Their rolling animation uses its own clock.
- Heavy Weapon's basic projectile is a pea rather than a Starfruit star. Homing
  shots use the displayed board position and validate the target's generation
  before keeping a lock; invalid targets are reacquired.
- Portal travel translates an airborne actor's trajectory as well as its current
  position, preventing a snap back on the following frame. Homing projectiles
  are not mistaken for straight portal-compatible shots.
- Bobsled relationships validate the leader's generation. An old team cannot
  attach to, or dismount, a new team that reuses its leader's actor slot.
- Zombotany's destructive attacks use shared square cleanup, including Cob's
  second square and supporting plant state.

## Rewards and story

- Reward movement/growth is more gradual, with separate position and size curves
  and an endpoint matching the handheld award frame. Input, automatic story
  changes and the ending wait for the pickup transition to finish.
- Pickups use reward-specific sounds. Removed temporary "reward is coming" and
  "wait for your reward to land" text from the lawn.
- Dave's talk clock and voice start after his entrance. Entry, exit and final
  grab use their authored clip lengths rather than one shared hard-coded wait.
  SCREAM2 selects the intended short talking pose.

## Data and saves

Use **EBOOT.PBP and freshly prepared data together**. Recipe: `ps3-0240-1`;
649 PVA banks and 692 integrity-listed files. Setup.exe includes its own Python
runtime; the fresh setup test completed in 203.952 seconds on this PC with no
Python on PATH. This is one machine's measurement, not a universal estimate.

Game saves use version 14. Published version 13 and older supported saves import;
profile format stays at version 5. New saved fields are checked before loading.
Back up your existing folder because old executables cannot read version 14 saves.

## Still unfinished

Full cross-clip skeletal blending, exact Gargantuar/Imp coordinate projection,
every specialized death and status interaction, complete boss effects/cameras,
and full PS3 Heavy Weapon weapon behavior still need comparison. Portal,
Zombotany, Bobsled, late Endless and wave randomness are not certified exact.
Original story camera sequences, every Dave expression/mouth override, complete
menu transitions, reward coin choreography and Gold Sunflower presentation are
still incomplete. The ending remains a reconstructed source-resource movie.
Multiplayer/ad-hoc play is not implemented. Physical PSP testing is still needed.

See [validation results](VALIDATION-0.24-DEVELOPMENT.md).
