# 0.19.0 — combat transitions and wave reliability

- Adds Newspaper gasp/recovery, Jack pop/explosion, Gargantuar throw and Imp
  flight/landing, with saved action state and original disc animation banks.
- Gargantuar strike/release markers, smash/throw rates, Newspaper and Jack rates
  were checked in the owner's PS3 executable. Garlic reaction milestones were
  also checked. Original trajectory, blending and complete choreography remain partial.
- Advanced animation clocks pause correctly with eligible freeze states.
  Balloon uses its real eating clip; Dolphin uses its own jump animation.
- Adds ordinary charred and water deaths, fixes external corpses after Chomper
  swallowing, and prevents Chomper swallowing a Gargantuar whole.
- Corrects several friendly/airborne/underground target filters, lobbed-shot
  landing hits, and pending Imitater/two-cell Cob destruction cleanup.
- Queues wave/grave arrivals when the zombie pool is full. Bobsled teams need
  four slots together and include all riders in wave health. Ordinary wave
  timers are retained. The meter preserves completed progress during a queue.
- Fixes Survival intermission corpse/effect updates and new-round state reset.
- Save version 11 imports version 10 and the existing older layouts. The prior
  save is retained as a backup; missing-primary rename-gap recovery is supported.
- Retries transient Windows permission failures in ending preparation without
  accepting incomplete output. Persistent errors still stop preparation.

**Update both EBOOT.PBP and data.** Run the new Setup.exe against the supported
original PS3 ISO and wait for READY. This version rebuilds previous data to add
12 animation banks. Keep and back up your saves. Older executables cannot read
a save after it has been written in the new version 11 format.

See the [complete C1–C24 / W1–W16 report](../reference/COMBAT-WAVES-0.19.0.md)
for changes, reference evidence, tests and explicitly unfinished fidelity work.
No original game assets are included in the public download.
