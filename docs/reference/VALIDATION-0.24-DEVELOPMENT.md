# Validation — 0.24.0 development

17 November 2026. [Changes and limitations](POLISH-0.24-DEVELOPMENT.md).

## Final executable

Bare EBOOT SHA-256: `8d4f654394276acc71f271b985c52f1d68fc3efc1df8a36e077db6e6c3f035a4`.

| Emulator configuration | Passed checks | Failures | Cache errors |
| --- | ---: | ---: | ---: |
| Street, 60 FPS target, full ending/audio suite | 1384 | 0 | 0 |
| PSP-1000, 60 FPS preference, live Day/Pool/audio suite | 1385 | 0 | 0 |

The Street run took 382.55 seconds; PSP-1000 took
351.55 seconds. Both used the same executable and newly prepared
resources. Emulator timing is not physical-console performance certification.
PSP-1000 still defaults to 30 FPS with smaller caches.

The suites include fifty combat fixtures that use real projectiles, damage,
terrain and wave transitions. These are well-defended synthetic fixtures,
not human playthroughs or proof of original balance. The boss fixture uses
the actual vulnerable-head window and available attacks.

New checks cover queued final arrivals, water-lane eligibility, independent
dancer groups, recycled leaders/targets, ballistic save state, boss attack
priority and exits, portal trajectory continuity, Last Stand stage budgets,
version 13 import and malformed version 14 fields. Seventeen additional UI
checks cover new clip loading/rates, armor damage, dancer poses, Dave clocks
and the reward transition. Earlier chapter/reward/ending tests were updated
to wait for the full pickup rather than its former short duration.

Live PSP-1000 fixture log:

```text
STARTUP FIRST FRAME: 53.464 seconds; diagnostics enabled
LIVE DAY BENCHMARK: 59.82 FPS, draw 2.149 ms, GPU wait 0.012 ms, 3918 frames
LIVE DAY: simulated 66.984 seconds, 4019 fixed updates, 39 kills, 39 replacements, 30 zombies alive
LIVE POOL BENCHMARK: 59.58 FPS, draw 1.661 ms, GPU wait 0.013 ms, 3951 frames
LIVE POOL: simulated 66.016 seconds, 3961 fixed updates, 37 kills, 37 replacements, 42 zombies alive
BENCHMARK: 59.58 FPS, draw 1.661 ms, GPU wait 0.013 ms, 3951 frames
```

## Preparation and startup

- Python preparation tests: **51 passed**; importer tests: **6 passed**.
- Fresh frozen Setup.exe run: **203.952 seconds**, no Python on PATH.
- Setup SHA-256: `5e2e0e4bdf5b9cdc62c74ad3b89db50ebfcaf48d17bd28c20adb434fb38e6cdc`.
- Recipe `ps3-0240-1`: **649 PVA banks, 692 integrity-listed files**.
- Private XMB wrapper retains the owned icon/background/music; its executable
  payload is byte-identical to the tested bare EBOOT.
- Both normal-startup profiles reached their first frame with diagnostics
  skipped and no cache error. The Street default was 60 FPS; PSP-1000 was 30.

## Remaining acceptance

Physical Street/1000/2000/3000 testing, Memory Stick performance, long Endless
sessions, suspend/resume and complete PS3 side-by-side timing remain outstanding.
These checks do not establish exact boss/camera/effect parity or add multiplayer.
