# 0.18 — disc Foley, garden presentation and challenge progression

This is a native PSP reimplementation using the owner's BLUS30852 resources.
It remains unfinished; the changes below do not establish exact PS3 parity.

## Sound events

All 194 actual MPG assets from the disc are now in a 4,070,192-byte streaming
IMA ADPCM bank. The PS3 executable supplies 104 Foley groups, their weighted
variants, pitch ranges, loop/instance/pause flags and Dave's no-repeat choices.
The resource manifest declares tapglass, which is absent from this sound archive;
no replacement is invented. Both unlisted thunder-4 files are included.

Gameplay now uses specific sounds for armor/body hits, newspaper tearing/rage,
plant attacks, sun creation/collection, coins/diamonds, mushroom shots, explosions,
vases, bowling, vehicle/tire interactions, jumpers, portals, Jack/Digger/Rain loops,
umbrella blocks, boss actions, garden tools and scene/UI transitions. Groans use a
separate 5–15 second timer rather than playing on each ordinary spawn. Dave has
short, long, extra-long, crazy and scream choices. Playback does not change the
saved gameplay RNG. There are 12 simultaneous streaming voices; buffers occupy
444,416 bytes, replacing approximately 3.7 MiB of resident effects PCM.

All audio files are available to the mixer, including assets for original
features that this port still lacks. The unsupported Sukhbir cheat and missing
bungee ground-marker presentation do not gain their original event paths merely
by importing their sounds. Spatial mixing, exact PS3 gain/voice priorities and
frame-for-frame event timing still need direct console comparison.

## Garden presentation

Adds 34 prepared animation/image assets. Stored marigold variations now select
the eleven recovered PS3 petal colours. Body and petals render separately so the
face and leaves keep their normal colours. New plants also retain random facing.
Adds garden/water pots, plain sprouts, sleeping mushrooms with drifting Zs,
mature Sun-shrooms, aquarium Kelp/Sea-shrooms, stage sizes and chocolate speed.

Watering, gold-can area watering, fertilizer, spray and phonograph use their disc
animations/rates. Care applies when the tool animation completes. Consumables are
reserved once, refunded if the action becomes invalid, and completed before
leaving/saving. Moving plants lift with the glove; the wheelbarrow uses its own
art. Stinky uses turn, wake/retract and idle clips, with normal/chocolate speed.

## Challenge progression

The PS3 challenge table and recovered unlock arithmetic now gate new launches.
The Adventure 3-2 and 4-6 presents unlock Mini-games and Puzzles; completing
Adventure opens Survival and later challenge progression. Mini-games/Survival
advance by distinct trophies; the two puzzle families advance separately.
The first three challenges are the pre-Adventure limit. Pool Endless requires
all ten Normal/Hard Survival trophies. Locks shake, open and fade on new unlocks.

The original Survival page has eleven selections: five Normal, five Hard and
Pool Endless. Old extra-terrain records and valid saved sessions remain intact.
Earned legacy trophies remain playable. Existing profiles migrate from their
Adventure progress and earned records, without removing purchases or plants.
Game save version 10 and profile version 3 remain unchanged. challenge-view.dat
only remembers which lock animations have already been shown; it grants no access.

## Hardware and installation

The same EBOOT detects models: Street/2000/3000 target 60 FPS; PSP-1000/unknown
use the conservative 30 FPS profile. The separate older 60 FPS experiment remains
available. Physical PSP testing is still required. Wireless modes remain unfinished.
Use the complete kit or the 0.17-to-0.18 update: new data is mandatory. Keep saves.
