# Scene corrections — 0.24.1 development

17 November 2026. Addresses the reported cut-off robot, plants offset from
their Flower Pots, and zombies walking above the pool instead of swimming.

- **Zombot:** its artwork contains negative source coordinates. The old bake
  clipped those pixels before making PSP textures. The source canvas now covers
  authored artwork and its original driver attachment. All 22 banks share one
  origin and scale. The upright robot towers above the camera, as in the original;
  its head lowers into view for attacks. An earlier attempt to fit all parts on
  screen made the retracted pose look like a small incomplete torso; that framing
  was removed. Drawing respects the HUD/footer boundaries, while canvas-edge
  checks reject lost source pixels before encoding.

  Visual reference: [PS3 level 5-10 recording](https://www.youtube.com/watch?v=1sD6nQPA8f0),
  including upright views at 4:51 and 5:49. This is a framing comparison, not a
  claim that every attack/camera movement matches exactly.
- **Flower Pots:** plant crops no longer determine their attachment point.
  Imported placement metadata retains the original 80×80 cell origin and the
  pot/plant height offsets. All 49 plants and the separate unarmed Mine crop
  have placement entries. Occupied pots stop their idle animation. Pumpkin
  layers, shooter parts and Squash takeoff retain the same placement correction.
- **Pool zombies:** Ducky, Conehead and Buckethead banks use the original
  42-frame swimming sequence, in-water tube artwork and ripple tracks.
  Underwater legs are hidden in swimming/eating/death poses, and the drawing
  retains a shared waterline, including damaged armor variants.
- **Headless screenshot:** `review-fidelity-transitions` deliberately sets Jack,
  Ladder, Digger and Yeti to 100/100/80/400 health to exercise head-loss artwork.
  It is now marked as a damage/disarm test. A separately labeled full-health
  scene shows the intact versions. Tests verify heads on fresh spawns and after
  reusing a previously headless actor slot. Normal head-loss behavior is retained.

These are presentation corrections. Combat rules and the version 14 Game /
version 5 Profile save layouts remain those of 0.24.0. Use the new executable
and freshly prepared `ps3-0241-1` data together. The ready-to-copy private ZIP
already contains that prepared data.

This does not finish all original boss effects, attack choreography, specialized
mode interactions or camera sequences. See the [broader status](POLISH-0.24-DEVELOPMENT.md)
and [0.24.1 validation](VALIDATION-0.24.1.md).
