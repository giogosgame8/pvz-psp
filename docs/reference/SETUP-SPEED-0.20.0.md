# First-time setup measurements — 10 November 2026

This measures creating files from the supported original PS3 ISO, not reusing
previously prepared data. The optimization changes are in commit `5b080d3`.

## What changed

- Reuse identical transformed artwork across animation frames, with a bounded
  32 MiB cache. The cache stores layers, not a previous frame's background.
- Process independent graphics frames concurrently in the remaining menu,
  shooter and Squash preparation stages.
- Decode prepared RGBA4444 portraits with Pillow's compiled channel operations
  instead of a Python loop over individual pixels.
- Keep the existing output format, compression level and integrity checks.
  Python is still bundled inside Setup.exe; players do not install it.

## Controlled stage comparisons

The same extracted inputs were processed old/new/new/old in one process. Every
output file from each stage was compared with the baseline and was identical.

| Graphics stage | Old runs | Optimized runs | Files compared per run |
| --- | --- | --- | --- |
| Level/mini-game graphics (`prepare_features09`) | 10.228 s, 9.686 s | 2.869 s, 2.843 s | 27 |
| Existing menu backgrounds (`prepare_menus011`) | 10.108 s, 9.140 s | 2.133 s, 2.117 s | 16 |

Those stages consistently improved by approximately 3.4–4.8 times in these runs.
They are only part of setup. These comparisons predate the new animated-menu
resources added in 0.20.0, so they compare equal work.

## Complete setup comparisons

Each frozen executable generated the same 0.19.0 content in a fresh destination.
The system PATH contained Windows tools, with no Python or py launcher. The
Windows file cache was warm; this was not a cold-boot experiment. FFmpeg was
already installed. Other user applications were left running.

| Pair | Published 0.19.0 setup | Optimization candidate | Output comparison |
| --- | --- | --- | --- |
| 1 | 114.757 s | 97.945 s | 422 identical files |
| 2 | 100.133 s | 155.365 s | 422 identical files |
| 3 | 188.695 s | 130.284 s | 422 identical files |

The only excluded file was `ending-progress.json`, whose diagnostic timings
naturally change. Animation frames, audio, ending video and integrity records
were compared. There were no other differences in any pair.

**These noisy whole-run results do not establish a dependable overall speedup.**
Unchanged music and ending stages also varied markedly. The stage-specific
results above support retaining the improvements, but do not justify promising
a percentage reduction or an average completion time on other PCs.

Test machine: Intel Core i5-14600K, 20 logical processors, approximately 32 GB RAM.
Disk speed, antivirus scanning, available memory, background load and CPU speed
can affect a user's time. The first FFmpeg download is additional work.

## Compression experiment

All 391 original graphics banks (10,244 frames) were also recompressed at zlib
levels 9/6/6/9. Every result was decompressed and checked against the source pixels.
Level 9 took 3.129/3.114 s; level 6 took 1.552/1.549 s in this isolated parallel
test. Compressed payload grew from 32,048,825 to 32,363,920 bytes at level 6.

The shipped default remains level 9. This modest saving did not justify changing
existing output bytes or increasing storage traffic on the PSP. Reusing a
compatible prepared directory remains much faster than a fresh conversion.

## New-content build check

The final 0.20.0 standalone setup completed a fresh ISO conversion in 88.646 s
on this PC, with no system Python on PATH. Its 489 graphics banks and all 521
integrity-indexed files verified successfully. This includes additional content,
so the time is a build check, not a controlled comparison against 0.19.0.

A subsequent compatible-data verification/reuse run completed in 3.490 s.
It reported READY without rebuilding the assets. This is a separate repeat-run
measurement, not a first-install time.
