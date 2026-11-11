# Adaptive music — 0.22.0 development

11 November 2026. This extends the 0.21 development checkpoint; it does not
complete the entire [feature backlog](FEATURE-BACKLOG.md). Public 0.20.1 is
unchanged until a separate release is published.

## Implemented

- Grasswalk, Moongrains, Watery Graves, Rigor Mormist and Graze the Roof now
  react to the living hostile zombie count. Day/pool/fog/roof mix the main,
  percussion and hi-hat parts; night changes to its dedicated alternate module.
- Entry at ten zombies, exit below four, a four-second rise and an eight-second
  minimum hold use the owned PS3 controller's 100 Hz counters. Recovery lasts
  eight seconds, or eleven for night. Day-type percussion waits for a musical
  boundary and fades for half a second on exit; night fades its alternate part
  for eight seconds and restores its main part during the last four seconds.
- Huge-wave countdowns can request the intense arrangement. Pause freezes both
  the consumed music samples and the adaptive clock. Game simulation and RNG
  remain separate from the audio clock.
- The existing automatic selection of all thirteen context tracks remains.
  Menus and other songs do not acquire combat layers. ADPCM streams retain
  their source intro/loop distinction instead of always replaying the intro.
- Track loading preserves and refills the currently playing song while the next
  one is opened. Stale requests cannot replace the latest selection. Failed
  reads do not terminate the worker permanently; a later track can still load.
- Nine additional mono IMA ADPCM streams, shared layer normalization and
  integrity coverage are generated from the owner's PS3 music modules.
  FFmpeg conversion is noninteractive even when an output already exists.

## Resource cost

The PCM ring/entry buffers together remain **128 KiB**. The worker stack rises
from 64 to 80 KiB to retain a second reader bank during loading; metadata adds
about 5 KiB. This is not a claim that total audio memory is unchanged.

Prepared music grows from 15,282,700 to **27,526,696 bytes** on the owned input.
All nine layers are compressed; the increase buys independent musical parts.
Recipe `ps3-0220-1` requires new prepared data. Save/profile formats remain 12/4,
as in the 0.21 development build; older public executables cannot read them.

## Evidence and remaining matching work

The owned BLUS30852 executable supplies the channel groups at `0x23d3ac`, burst
entry at `0x23c9d8`, and controller thresholds/counters at
`0x23cad0–0x23d228`. Music comes from MAINMUSIC.XM (30 channels),
MAINMUSIC_DRUMS.XM and MAINMUSIC_HIHATS.XM. The executable and original resources
stay private; source contains only the reimplementation and numeric facts.

The four-/seven-second huge-wave cue interpretation also uses the supporting
PC reconstruction. The PS3's packed order/fraction boundary handling has not
been fully resolved: this implementation currently queues on whole XM order
changes, and preparation derives positions from the module's timing commands.
FFmpeg's tracker clock differs slightly from the analytical clock. All layers
share the same exported timeline, but exact PS3 beat phase, decoder timbre and
seamless loop fidelity still need listening/reference acceptance. Do not
describe this checkpoint as sample-exact original PS3 mixing.

Validation results are recorded separately in
[VALIDATION-0.22-DEVELOPMENT.md](VALIDATION-0.22-DEVELOPMENT.md).
