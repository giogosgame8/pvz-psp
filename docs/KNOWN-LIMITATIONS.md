# ISO content audit, 5 November 2026 (0.19.0)

This audits the current native PSP implementation against locally extracted
resources and the project's PS3 comparison records. It is not an exhaustive
frame-by-frame PS3 playthrough. A menu entry, imported image or dialogue string
does not establish that its original behavior is implemented.

The supplied restored disc is BLUS30852, containing PvZ NPUA_30059.
ISO SHA-256 was checked again for this audit:
50ecf8b9a8ccbba80bbbba83d9456c85f3080c33039a1f7424261235faaf1053.
The game is a native reimplementation using resources from that copy; it does
not run the PS3 executable on PSP. 0.18 adds the sound bank, garden presentation and challenge progression.

## Available already

50 first-run Adventure stages, 20 main mini-game selections, 11 original Survival
selections (legacy extra records are retained), 20 Puzzle selections (Vasebreaker and I, Zombie with Endless),
49 plants, Zen Garden/additional gardens, Tree of Wisdom, shop purchases,
75 Almanac entries, five zombie chapter notes, saved progression/replay,
13 context-selected music tracks and all 194 available disc sound files
with 104 recovered Foley group definitions are present. The ending song
and a 158.75-second animated ending reconstructed from disc layers are present.
These counts describe implemented content, not exact PS3 parity.

0.18.7 adds a PSP zombie customizer using the disc's available heads, glasses,
headgear and mustaches. It saves a flag-zombie appearance separately from the
campaign. 0.18.8 restores the separate jaw track and loads gallery clips asynchronously.
0.18.9 replaces the fixed chewing timer with recovered PS3 loop cues and eating
playback rates, shared by drawing and sound. These improve specific behaviors;
they do not establish complete PS3 parity.

0.19.0 adds saved combat transitions, timed Newspaper/Jack/Gargantuar actions,
Imp flight and landing, ordinary charred/water deaths, and capacity-safe wave
queues. The [full combat and wave report](reference/COMBAT-WAVES-0.19.0.md)
covers every approved checklist item and its remaining differences. This does
not complete the missing modes or certify all original combat behavior.

The supported PS3 disc does not contain the original GOTY Zombatar
creator resources; this addition does not reproduce that complete asset set.

## Confirmed missing functionality

| Feature | Current gap and evidence |
| --- | --- |
| Co-op, its challenge progression and versus | No playable multiplayer or PSP ad-hoc transport. Both mode bits remain excluded in src/platform.h and hidden by the menus, even on Wi-Fi models. The ISO's LawnStrings contains its co-op mode/selection strings. |
| Second Adventure run | No completed-run counter, second-run seed restrictions or original repeat-run progression. src/main.c new_game() starts the first campaign again; src/profile.h stores a completion boolean only. Replay is not the original second Adventure. |
| Yeti encounter/progression | Its art and base type exist, but the campaign has no second-run encounter trigger and chapter_zombie() has no retreat behavior. It should not be described as a fully implemented original encounter. |
| Giga-gargantuar | The disc has REDEYED_GARGANTUAR/Giga-gargantuar text. No corresponding variant, health rules or selection logic exist in the native zombie types and Survival rules. Ordinary Gargantuar exists. |
| Gold Sunflower and complete trophy progression | Individual mode wins and the Silver Sunflower are tracked, but there is no combined Gold Sunflower completion award or Adventure completion-count presentation. The disc's SILVER_SUNFLOWER_TOOLTIP describes this reward. |
| House customization | The ISO has Customize/Choose House text and named Funhouse, Scary House, Space House and Truck House options. No house-selection screen, ownership state or house rendering is implemented in the port. Online house-sharing services are a separate excluded PS3 feature. |
| Secret-code cosmetic effects | Tree text mentions effects such as future/trickedout and later unlocks, but no code-entry/effect system applies those unlocks to zombies, mowers or particles. Growing/feeding the Tree itself works. |

## Present but still needing matching or verification

| Area | Remaining work |
| --- | --- |
| Story/cutscenes | Precise PS3 camera direction, dialogue branching, Dave emotion/held-prop choreography, tutorial pacing and some transitions. Imported dialogue and major introductions already play. |
| Survival, puzzles and Endless | Exact PS3 wave tables, all placements/inventories, enemy composition and long-run balance. Some puzzle facts come from supporting PC source and are not verified PS3 layouts. Native limits are 96 zombies, 256 shots and ten tray packets. |
| Mini-games | Heavy Weapon durations/targeting, Zombotany attachments/special attacks, Bobsled collision/movement and portal transitions. All 20 selections being playable does not establish faithful PS3 reproductions. |
| Boss and combat | Attack scheduling, attachments, stomp/spawn choreography, damage interactions, collision nuances and some plant/zombie particles or death details. |
| Presentation | Handheld menu proportions, animation transitions, particles, reward timings, exact original garden choreography and spatial audio/gain/voice priorities. UI and controls have deliberate PSP adaptations. |
| Ending | Song and reconstructed animation are included at 480x272/24 FPS. Recovered PS3 lyric timing is optional when preparing from a private executable; normal disc-only setup does not recover it from the encrypted executable. The result is not a verified frame-exact reproduction of the original live compositor. |

## Verification limits

The 0.18.8 standalone Windows setup completed a full disc conversion without
Python or the py launcher on PATH. The source counts above include the 3,810-frame
reconstructed ending. Native checks use PPSSPP profiles; these checks are not
physical-console certification. Version 0.19.0 requires its new prepared data;
the standalone setup has also completed a fresh conversion without system Python.

PSP-1000 support remains experimental. Its 30/60 FPS target is not a performance
guarantee. Dense scenes can exceed its small graphics cache and need further
performance work; see docs/PERFORMANCE.md for the workload-specific results.

PSN leaderboards, PlayStation system trophies and other PS3 platform services
remain excluded from the agreed offline scope. Local collectible trophies are
implemented separately and should not be confused with PSN system trophies.

Supporting evidence: src/platform.*, src/main.c, src/profile.*, src/game.h,
src/chapter_game.inc, src/extra_rules.inc, src/mini_ui.inc, src/extra_ui.inc,
src/garden.*, tools/prepare_assets.py and the supplied disc's LawnStrings.txt.
0.18 now uses stored marigold colours, facing, sleep/aquarium/pot variants,
animated tools and progressive challenge locks. See docs/changelog/CHANGES-0.18.md and
docs/reference/PS3-REFERENCE-0.18.md for direct ISO evidence and remaining matching limits.
No RPCS3 session was opened for this audit.
