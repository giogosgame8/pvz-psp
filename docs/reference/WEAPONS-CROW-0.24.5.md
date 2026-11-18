# Heavy Weapon and original crow motion — 0.24.5

## Heavy Weapon

The supplied BLUS30852 executable identifies Heavy Weapon as challenge 20.
The seed-bank setup at `0x1a0cd4` lists Repeater, Threepeater, Cattail, Snow
Pea, Torchwood and Doom-shroom in that order. The branch at `0x243f74`
sets their prices to 200/400/200/100/125/200 sun. Purchase code at
`0x1a5d84` keeps the barrel count and Cattail rank separately. Only Repeater
is available initially; it opens the remaining upgrades.

The firing branch at `0x1b3ce8` resets the pea timer to 25 of the original
100 gameplay ticks per second. The projectile construction at `0x1a58d0`
rotates the two-pea burst by −10/+10 degrees and the three-pea burst
by −15/0/+15 degrees. The separate spike branch at `0x1b3b34` yields
100, 75 and 50 ticks at ranks 1, 2 and 3. Thus Cattail never replaces the
primary weapon. The Ice and Fire purchase branches both store 600 ticks,
and clear the other effect. Doom-shroom calls the radius damage routine
with the central cell and a radius of 250 in PS3 coordinates. The PSP
projects this onto its lawn, preserving the existing crater and save format.

The change is verified from disc code rather than a playable PS3 capture.
The PSP maps the dual-stick controls onto D-pad movement, analog aim and X
for firing; with the stick centered, holding X shoots straight up. Shots
originate above the turret. Original controller response, effects, and every
late-wave behavior are outside this focused pass.

## Crow

`tools/prepare_crow_paths.py` reads the local `crow2` track in
`mainmenu3/mainmenu.reanim`. It projects the six authored clips at 12 Hz:
two entrances, Home to More Ways and back, and Home to Extras and back.
Its 328-byte result contains only projected anchors, facing and landing cues.
The PSP interpolates those keys and adjusts the endpoints if the player
interrupts a route. The game still switches menu backgrounds immediately,
so this is not the PS3's full camera pan. It uses the existing
`Crow.reanim` artwork for posing and the previously verified perches.
A missing or invalid optional path file falls back to the earlier motion.

No graphics or audio conversion is needed to update an existing 0.24.1–0.24.4
install. Copy `EBOOT.PBP` and `data/crow-paths.bin`; keep all existing files
and `PSP/SAVEDATA/PVZPSP`. A new setup generates the same file from the ISO.

## Validation

PPSSPP Street at 60 FPS and PSP-1000 at 30 FPS each passed 1,431 regression
and scene checks. Weapon coverage includes the upgrade gate, the two-pea
spread, independent spikes and their target priority, effects and the Doom
radius. A direct cadence test verifies four volleys per second at both frame
rates. Crow tests cover direction-specific paths, the three perches, a
mid-flight reversal and fallback after a damaged path file. The packaged
launcher also passed normal startup, migration and save restoration with
the prior data. Physical PSP testing remains outstanding.
