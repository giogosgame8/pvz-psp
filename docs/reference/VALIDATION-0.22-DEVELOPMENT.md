# Validation — 0.22.0 development checkpoint

11 November 2026. Scope and original-game limits:
[adaptive music](DYNAMIC-MUSIC-0.22.md),
[full completion status](COMPLETION-WORK.md).

## Native executable

Bare EBOOT SHA-256:
`df7be5ee7584596295e6cf978c2809245b7717f672a14acee10616afde95dbb8`.

| Emulator profile | Target | Passed checks | Failures | Cache errors |
| --- | --- | --- | --- | --- |
| Street, full ending and adaptive playback | 60 FPS | 1,269 | 0 | 0 |
| PSP-1000, live Day/Pool and adaptive playback | 60 FPS | 1,270 | 0 | 0 |

Both use that exact executable and byte-identical prepared resources. The
PSP-1000 live tests measured Day **59.83 FPS**, 2.127 ms draw CPU, and Pool
**59.53 FPS**, 1.655 ms draw CPU. Simulation remained 4,019 updates / 66.984 s
for Day and 3,961 updates / 66.016 s for Pool. These are emulator measurements,
not physical-console results; PSP-1000 retains its 30 FPS default.

Each run includes seventeen controller checks, three added seek/intro-loop
checks, thirteen base-track and nine layer fingerprints against FFmpeg, and
live calm/rise/hold/recovery/pause checks for all five themes. Adaptive playback
completed with no additional read errors or underflows. The rapid-selection
test makes 61 changes with a 250 ms injected storage delay: 5/5 checks pass;
menu 59.94 FPS, maximum frame 17.907 ms Street / 18.178 ms PSP-1000.

An earlier candidate failed that rapid-switch underflow check. The final
worker retains a separate active reader bank and refills the old song before
opening a candidate. Both final runs above include this correction.

Street's complete ending reached its final credits with **3,540 displayed
frames**, maximum reported lag **0.534 s**, audio position **159.266 s**. This
is a regression check, not sign-off of the remaining exact PS3 visual matching.

Normal startup, with diagnostics absent, reached the first frame in **1.062 s**
in both Street and PSP-1000 emulator fixtures. Detected targets were 60 and
30 FPS respectively. The approximately 49-second startup in diagnostic runs
includes the test suite and must not be presented as normal boot time. Real
Memory Stick cold boot and suspend/resume remain unmeasured.

## Setup and private delivery

- **43 preparation/setup tests and 6 importer tests pass.**
- Fresh frozen Setup.exe built all data from the owned supported ISO with Python
  and `py` absent from PATH: **106.981 s** on this PC. This is not an average
  user-PC time or a controlled comparison against earlier releases.
- Recipe `ps3-0220-1`: 522 graphics banks, **565 integrity-indexed files**.
  Every indexed fresh-setup file matches the development candidate byte for
  byte, including all layer files and their decoder fingerprints.
- Prepared music: **27,526,696 bytes**, including nine new compressed layers.
  Joint normalization bounds the intended musical combinations before the
  normal mixer gain. Corrupt or omitted layers fail setup integrity checks.
- The private EBOOT embeds the already validated owned-disc XMB icon, cover and
  ATRAC3 sound. Its executable payload is byte-identical to the tested bare PBP.
  Private PBP SHA-256:
  `dae83c0cd94f40363fa49ef37eaed581c759668ef8b1392cbda12ba4bf69498c`.

Save/profile formats remain 12/4 from 0.21. Existing 0.21 data must be replaced
with the new prepared set together with the executable. Back up saves before
testing; the public 0.20.1 executable cannot read those newer save formats.

No physical PSP, two-console wireless test or RPCS3 session was used this pass.
Co-op, versus and ad-hoc remain unimplemented and hidden. Exact PS3 fractional
musical-boundary handling, listening acceptance and the wider combat/story/
garden polish backlog remain open. Public main/release has not been changed.
