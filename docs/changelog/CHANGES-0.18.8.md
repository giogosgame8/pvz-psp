# 0.18.8 - customizer face and gallery loading fixes

- Restores the separately animated jaw to every customizer face, including
  glasses styles and the walking/eating clips. Existing saved presets still work.
- Gallery animation banks load through PSP asynchronous file I/O. Small portraits
  appear while the full clips load, and all eight cards animate once ready.
  Changing pages, tabs or poses does not wait for the previous bank to finish.
- Pending reads count against the existing 2 MiB PSP-1000 or 8 MiB standard bank
  budget. Leaving a scene, freeing an animation or suspending the cache safely
  drains any read before its buffer can be freed.
- Adds 81 prepared gallery portraits, including the six Zombotany variants.

Run the new Setup.exe and install both EBOOT.PBP and the new data folder. Keep
existing campaign saves and custom-zombie.dat. The prepared-data recipe changes
to 0.18.8; old data is rebuilt instead of accepted as a verified current cache.

Validation includes all 11 gallery pages, rapid page/tab/pose changes with an
injected 250 ms read delay, byte-for-byte comparisons of asynchronous and normal
reads, and cleanup while reads are in flight. Python tests cover the independent
jaw motion in every face style. Only the two face banks change among existing
game-content files; 403 other files remain byte-identical to 0.18.7.

The performance checks run in PPSSPP with Street and PSP-1000 profiles. They do
not establish timings on physical PSP hardware or every memory card.

The Street run passed 1,012 checks, including full ending playback; the PSP-1000
UI run passed 1,011. Both passed all 16 gallery I/O checks with zero cache errors.
The worst measured gallery drawing pass was 49.689 ms, including cold resources;
there were no synchronous bank reads or waits during page navigation. These
are emulator diagnostics, not a guarantee of 60 FPS on every physical PSP.
