# PSP reimplementation status - 0.18.1

One native EBOOT targets PSP Street, PSP-2000 and PSP-3000, with an experimental
PSP-1000 profile. The PS3 executable
is not being run or converted on PSP. Private disc resources are prepared
separately. This remains an unfinished reimplementation.

Available: 50 Adventure stages, 20 main mini-games, 11 original Survival selections,
20 Puzzle selections, Zen Garden and its additional gardens, Tree of Wisdom,
49 plants, 28 visible shop entries, the 75-entry Almanac, five chapter notes,
original music/effects, level replay, rewards and saved progression.

0.10 adds the new mode families, garden economy/care, Dave presentation and
an animated ending reconstructed from the disc's layered animation. It also
improves Heavy Weapon, Zombotany, Bobsled, portal and boss behavior and fixes
timing, persistence, memory and UI defects.

0.11 uses the original menu panorama, stone widgets, challenge frames, shop
and Almanac art at handheld size. Challenge modes, collection screens and
Adventure replay now have separate hubs. Ordinary/giant bowling hits no longer
trigger the generic Cherry Bomb animation; red nuts retain one blast.

0.12 fixes shooter idle transitions, staged Squash attacks, seed-bank spacing,
Vasebreaker packet stacking, seed/trophy pickup presentation and wave-meter
progress. Health bars are optional and independently saved. The 0.11.1 XMB
cover and music are retained.

0.13 adds a saved player name/controller keyboard, makes the gear flower open
Options, removes routine menu control bars and the dialogue-library entry,
skips locked plants in the chooser, clips trophy effects inside the grass
window, and enforces automatic context music.

0.14 adds D-pad/X navigation to Credits and Pause, a paginated gallery of all
49 plants and 32 zombies (including six Zombotany variants), and a mode guard
that prevents permanent production glow on I, Zombie sunflowers.

0.15 replaces geometric yellow selection outlines with shared rounded sage
highlights and muted amber action markers. No game data or save layout changes.
PSP-1000 remains outside the validated release targets; the low-memory source
configuration is experimental.

0.16 fixes the garden care atlas split, uses the separate water droplet and
original speech bubble, and draws care overlays after plants so they remain
complete. New program and garden data are included.

0.17 adds automatic heap sizing and model-dependent cache/frame pacing. Model
0 and unknown queries use 30 FPS and 8/2/0 MiB frame/bank/static budgets;
Street/2000/3000 keep 60 FPS and 24/8/6 MiB budgets. The simulation stays at
60 updates/s. Data and player save formats are unchanged. Physical PSP-1000
validation remains needed; see the exact final model reports.

0.18 adds all 194 available disc sound files with 104 recovered Foley groups,
streaming character/tool effects, original garden variants and animated care,
and progressive challenge locks. Existing saves/records remain intact.

0.18.1 adds a saved PSP-1000 30/60 FPS option, lossless GPU-ready PVA4 graphics
and a 12 MiB small-model frame cache. 30 FPS remains the default for PSP-1000;
see PERFORMANCE-0.18.1.md for workload-specific results and hardware limits.

Remaining work includes complete PS3 cinematic direction/branching, second
Adventure, exact challenge/Survival wave tables, late Endless balance and
various combat, attachment and particle details. PSP ad-hoc co-op and versus
are unfinished and hidden. See REMAINING-FEATURES-0.18.md and PS3-REFERENCE-0.18.md for evidence and limits.

A 24 MiB decoded frame cache and 8 MiB compressed bank limit page the roughly
339 MiB virtual animation data within the automatically sized larger-model heap. The ending streams through
three frame buffers after releasing cached graphics. Resident single-frame
textures have a separate 6 MiB cap; additional images use the paged cache. Music uses IMA ADPCM.

Save format 10 imports supported older saves including version 9; profile
format 3 imports versions 1 and 2. Packages exclude player files. Back up your
installation before updating, particularly if you may downgrade later.

The final model reports belong to the exact executable and data hashes in the
kit. In 0.17, PSP-1000 repeats full synchronized ending playback after every
gallery page. Street/2000/3000 repeat the game/menu/audio review, first/middle/
final decoder checks and cache checks. Rule fixtures and the 50 preplanted
Adventure combat fixtures do not establish physical PSP performance or exact
PS3 parity. Real-console tests and long-session playtesting remain needed.

The personal kit contains privately prepared resources from the user's copy.
The separate asset-free source package excludes extracted game assets.
