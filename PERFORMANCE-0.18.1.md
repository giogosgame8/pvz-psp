# PSP-1000 60 FPS mode — 0.18.1

The opt-in target works in the tested PSP-1000 emulator configuration. It is
not a locked-60-FPS result, and no physical PSP-1000 was tested.

| Sustained live workload | Average FPS | Draw time | Frames |
| --- | ---: | ---: | ---: |
| Day, 30 plants and 30 maintained zombies | 58.67 | 9.676 ms | 3843 |
| Pool, 36 plants plus lily supports, 42 zombies, rain/thunder audio | 56.16 | 10.130 ms | 3714 |

Each sample runs for approximately 66 seconds, with real simulation, shots,
collisions, damage, dying zombies, sun/coins, music and effects. The synthetic
fixture replenishes zombies and plant health to sustain the load. It is a stress
workload rather than a measurement of every campaign or Endless situation.
Individual frames dip below the averages. Both samples exceeded 3,900 fixed
simulation updates; the game clock still advances at its normal speed.

The initial 0.18.1 live experiment with old PVA3 data / 8 MiB decoded cache
averaged 40.91 FPS by day and 39.77 FPS at the pool. GPU-ready graphics at 8 MiB
improved these to 44.76 / 44.42; 10 MiB to 53.41 / 51.70. Those development runs
are diagnostic comparisons on the same emulator settings, not physical-console
benchmarks under a controlled host workload. The table above is the final binary.
The older animated-only 0.18 experiment averaged 56.35 FPS; that workload did not
simulate combat and must not be mistaken for a live gameplay measurement.

Final configuration: PPSSPP 1.20.4, PSP model 0 / nominal 32 MiB, CPU 333 MHz,
480x272 game framebuffer, InternalResolution=0 in the recorded emulator config,
no frame skipping, bad-memory ignoring off, fast-memory access off. The final timed combat samples ran without another emulator benchmark.
Later UI-only validation overlapped ending playback, after the timed samples.

The 12 MiB decoded / 2 MiB compressed / 0 MiB static profile passed with no
texture cache allocation or decode errors. Concurrent sound playback had no I/O,
decode, underrun or voice-limit errors. All 976 native checks
passed, including all 50 Adventure combat fixtures, save compatibility, menu
navigation, frame-rate persistence, 194 sound checks and three texture layouts.

The full ending reached the final credits and returned to the menus:
3545 displayed frames, 0.534 seconds maximum reported frame lag,
159.266 seconds audio position. Displayed count is not every source frame;
the 24 FPS ending may drop frames to keep audio synchronized.

The 30 FPS fallback and other-model controls passed their own native checks.
Unknown model queries ignore a saved 60 preference and remain at 30 FPS.
Street/3000 ignore the 1000-only preference and retain 60 FPS / larger caches.
A physical PSP-1000, real Memory Stick stalls, battery use and long Endless
sessions remain unverified. Use Options > Frame rate > 30 FPS if needed.

Delivered EBOOT SHA-256: 7597eaecbe620ff459b5f7efb18ea6758a66ea2659a88726f007c1b8c9f0d313
All 356 converted PVA4 assets / 8,083 frames retain the original prepared pixels.
Source/profile/game formats remain compatible; the old 0.18 program cannot read
PVA4 graphics, so downgrading requires restoring its old data as well.
