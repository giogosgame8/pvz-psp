# 0.18.1 — selectable PSP-1000 frame rate and graphics preparation

Adds a saved 30/60 FPS option for known PSP-1000 models. In Options, highlight
Frame rate and use Left for 30, Right for 60, or X to toggle. Changes apply
immediately. 30 remains the conservative default. performance.dat stores this
choice separately, without changing level/profile/health-bar saves.

Unknown model queries remain at 30 FPS; other supported PSP models keep 60 FPS.
Simulation stays fixed at 60 updates per second. Rendering changes do not alter
zombie speed, spawn timing, damage, unlocks or saved progress.

Graphics now use PVA4 textures prepared in PSP GPU layout on the computer. This
removes texture tiling work from cache misses during gameplay. All 356 textures /
8,083 animation frames were converted losslessly and checked against the original
pixels. Native tests also verify old PVA3 loading, PVA4 loading and Imitater gray.
The program still accepts PVA3 data, although it misses the new preparation benefit.

The small-model decoded cache is now 12 MiB, with a 2 MiB compressed-bank limit
and no separately resident static-image cache. Larger models retain 24/8/6 MiB.
The small arena remains allocated across movie playback to avoid a fragmented
large allocation when returning to menus. The audio buffers added in 0.18 stay
bounded at 444,416 bytes. Memory and ending transitions are tested in emulation.

This adds no new levels or modes. All 0.18 sounds, garden variants and challenge
progression are retained. Wireless modes remain unfinished. 60 FPS is a target,
not a promise of locked 60 FPS on real hardware or every crowded scene. Read the
accompanying performance report for measured results, workload and limitations.

New data files are required for the graphics optimization. To downgrade to 0.18,
restore both the old program and its data; its old program cannot read PVA4.
Use the in-game 30 FPS fallback when you only want a lower rendering target.
