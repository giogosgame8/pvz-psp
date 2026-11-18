# Validation — 0.24.2 smoother transitions

17 November 2026. [Changes](HITCHES-0.24.2.md).

Final bare EBOOT SHA-256: `e4ae488c2c39cb7898482555255fb5d69b84221df80e7bc30d00d852446aace2`.

| Emulator profile | Passed checks | Failures | Cache errors | Run time |
| --- | ---: | ---: | ---: | ---: |
| Street scene/regression | 1403 | 0 | 0 | 105.26 s |
| PSP-1000 scene/regression | 1403 | 0 | 0 | 108.25 s |
| Street live day/pool | 1405 | 0 | 0 | 231.39 s |
| PSP-1000 full ending/audio | 1415 | 0 | 0 | 335.55 s |

These are emulator results, not physical-console certification. Scene runs
cover existing logic and 50 synthetic campaign fixtures, visual menus/combat,
cache layout and data integrity, plus the new checkpoint/retention/crow tests.

The 14 new checks exercise immutable snapshots, rapid coalescing, write failures
and recovery, retained frames after cache pressure, controller input while a
checkpoint is pending, and the crow's page-to-page flight and landing.

Measured on this PC's Street emulator:

- CHECKPOINT ENQUEUE: 163 us with 150 ms injected storage delay
- HITCH INPUT TIMES: pause 289 us, cursor 1 us, victory enqueue 280 us (200 ms injected storage delay)

The injected delay sleeps the save worker. It tests scheduling isolation, not
physical Memory Stick bandwidth or an end-to-end PSP frame-time guarantee.
The pause input and victory calls return before the storage operation finishes.
Existing temporary-file verification and backup handling remain enabled.

The XMB-wrapped executable passed normal-startup checks. The wrapper's
executable sections match the tested bare PBP. Normal startup logged 1.112 s
on each profile here; Street defaults to 60 FPS and PSP-1000 to 30 FPS.

The Street live run held 59.83 FPS in its day scene and 59.70 FPS in pool combat
with 60 fixed updates/s. Foley reported no I/O/decode errors or underruns.
These averages do not describe every frame or guarantee physical PSP speed.

On the PSP-1000 emulator profile, the ending reached the final credits with
3,535 displayed frames, audio position 159.266 s and maximum video lag of
0.534 s. Streamed music passed 5/5 checks and adaptive music passed 6/6, with
no reported read errors or buffer underruns.

Prepared data is unchanged from 0.24.1 (`ps3-0241-1`), including the soundtrack,
194 imported effects and ending data. The full private package is integrity
checked; the optional update-only ZIP contains EBOOT.PBP and instructions.
Game/Profile save layouts remain v14/v5. Cache budgets remain 12/24 MiB, with
retained pages capped at 1536 KiB inside those budgets.

Physical PSP Street acceptance of the reported hitches remains outstanding.
