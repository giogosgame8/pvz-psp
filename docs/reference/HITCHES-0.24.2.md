# Smoother transitions — 0.24.2 development

17 November 2026.

- **Pause and victory:** their checkpoints now copy the current game/profile
  and queue a background writer. Start can display the pause menu immediately,
  and the final death/reward animation can continue while storage is busy.
  Coin banking uses the same queue instead of writing the profile on the drawing
  thread during play. Collecting a reward also queues its checkpoint.
- **Save safety:** the writer owns two separate snapshots, coalesces only
  unstarted updates for the same files, and keeps the existing checked temporary
  file/backup replacement for Game and Profile. A failed profile write does not
  overwrite the game save. Direct save/load operations, mode changes, normal
  exit and the suspend callback drain pending work. Errors still warn the player.
- **Shared textures:** the font, pause controls, small reward artwork and all 49
  packet frames remain cached across ordinary combat-frame eviction. The four
  pause-menu frames are prepared before accepting input. Other shared art is
  read asynchronously and decoded one frame at a time. Retained frame pages
  have a 1536 KiB ceiling inside the existing 12/24 MiB frame-cache budgets.
- **Cache work:** frame occupancy is counted incrementally and free-entry
  searches keep a hint; decoding a new frame no longer recounts every cache page.
- **Crow:** it flies from its current position to a perch on Home, More Ways to
  Play and Extras, then lands and resumes its idle/caw sequence. The previous
  implementation drew it only on Home. This is menu-page following; it does not
  chase the lawn cursor or every highlighted button.

Game saves stay version 14 and Profiles version 5. Prepared data remains
`ps3-0241-1`: if 0.24.1 is already installed, replace only EBOOT.PBP. There is
no asset-conversion step for this patch. The full private package also includes
that same verified data for installations coming from older builds.

These changes remove identified blocking work; they do not guarantee that every
physical PSP frame will meet its target. Memory-card speed and cold combat
animation reads can still affect pacing. Street defaults to 60 FPS; PSP-1000
retains smaller caches and its 30 FPS default, with 60 FPS optional.

[Validation](VALIDATION-0.24.2.md) · [Broader limitations](../KNOWN-LIMITATIONS.md)
