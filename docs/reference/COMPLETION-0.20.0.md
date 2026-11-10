# Completion review — 0.20.0, 10 November 2026

This is a source and owned-disc resource audit, not a claim of full PS3 parity.
The port implements its own PSP engine and imports artwork/audio from the
supported BLUS30852 v01.00 ISO. Imported files and selectable modes alone do not
prove equivalent gameplay.

## This update

1. First-run setup reuses transformed layers, parallelizes more graphics work
   and decodes portraits with compiled lookups. Equal-work output comparisons
   remained byte-identical. See the [measurements](SETUP-SPEED-0.20.0.md), including
   the noisy total timings and their limits.
2. Options now saves **Dancing zombie: Disco / Michael Jackson**. Both lead-zombie
   art sets were retained in the supplied disc. Walking, eating, head-loss body,
   detached head, death and gallery poses follow the choice. Gameplay rules and
   campaign saves are unchanged. Backup dancers retain the available Disco-era
   artwork; this is not a complete recreation of the older game's dancer group.
3. The home, More Ways to Play and Extras pages now animate original authored
   grass, tree, flower and butterfly layers using interpolated GPU transforms.
   A crow flies in, lands, idles and calls with disc artwork and sound effects.
   Its entry/perch schedule is adapted for the PSP; exact original cloud,
   lightning, camera-transition and crow choreography remain unverified.
4. Extras hides both artwork and associated selection targets until earned:
   notes after 1-9, Almanac after 2-4, shop after 3-4 and Zen Garden after 5-4.
   Attached envelopes, packets and bonsai follow their parent entry's lock.
   D-pad navigation skips locked entries and wraps through the available ones.
5. No multiplayer or missing campaign modes were added by this update.

The menu scene uses 83 single-frame texture layers and motion metadata rather
than full-screen movie frames. Scene textures plus motion metadata total about
436 KiB. This keeps the added menu storage small; actual PSP performance still
needs physical-hardware testing.

## Existing playable scope

- 50 first Adventure stages and replay.
- 20 main mini-game selections, 11 Survival selections and 20 Puzzle selections,
  including the implemented Endless variants.
- 49 plant types and 26 native zombie types; 75 Almanac entries.
- Zen Garden, additional gardens, Tree of Wisdom, shop, five zombie notes,
  campaign/profile persistence and a PSP custom zombie appearance editor.
- 13 context-selected music tracks, 194 imported sound samples and 104 recovered
  Foley group definitions. This does not establish exact original sound mixing.
- A 158.75-second reconstructed animated ending with its song.

## Work required before describing it as complete

| Priority | Work still missing or incomplete |
| --- | --- |
| Missing modes | Playable co-op and versus, plus PSP ad-hoc transport and its progression. Wi-Fi hardware detection exists; playable wireless multiplayer does not. |
| Adventure progression | Original second Adventure run, its restrictions and Yeti encounter/retreat rules. Replay and starting another first campaign do not replace these. |
| Late completion | Giga-gargantuar, complete combined trophy progression and the Gold Sunflower award. |
| Optional features | House customization and secret-code cosmetic effects mentioned by the Tree. The PSP customizer is not the complete GOTY Zombatar creator. |
| Combat fidelity | Chomper windup/miss/swallow, special jump/emergence and vehicle transitions, boss attacks/attachments/choreography, remaining interactions and effects. See the [combat checklist](COMBAT-WAVES-0.19.0.md). |
| Mode fidelity | Exact PS3 wave tables, puzzle layouts/inventories, long Endless balance, Heavy Weapon, Zombotany, Bobsled and portal details. Native limits remain 96 zombies, 256 shots and ten tray packets. |
| Story/presentation | Exact dialogue branching, Dave props/emotions, tutorial/camera timing, remaining menu transitions, rewards, particles, garden choreography and audio priorities. |
| Ending fidelity | The reconstructed ending is not verified frame-exact. Recovering optional executable lyric timing requires a separate private reference; normal setup is disc-only. |
| Hardware acceptance | Long sessions, dense late-game/Endless scenes, Memory Stick startup and 30/60 FPS behavior on physical Street/1000/2000/3000 consoles. Emulator checks cannot certify those. |

The agreed scope excludes PSN leaderboards and PlayStation system trophies.
There is no defensible completion percentage: the available content count is
high, but missing modes and fidelity work vary greatly in size.

Suggested next implementation order: second Adventure/Yeti, Giga-gargantuar and
completion rewards, then the remaining combat/mode comparison work. Multiplayer
needs its own protocol, synchronization and two-console testing effort.

[Validation and hardware limits](VALIDATION-0.20.0.md): both native emulator
profiles passed, including the new menu and dancer checks. PSP-1000 dense-scene
performance remains a material limitation despite functional correctness checks.
