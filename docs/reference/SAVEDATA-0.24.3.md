# Saved data and PS3 wave timing — 0.24.3

The game still lives at `PSP/GAME/PVZPSP`. Starting with 0.24.3, its player
data and settings live at `PSP/SAVEDATA/PVZPSP` on the same storage device.
For a manual backup, close the game and copy the second folder to your PC.
To restore, close the game and put the backup folder back at that path.
Use USB or a Memory Stick reader. This build does not register an entry in
the PSP's XMB Saved Data Utility.

## Upgrading

Back up the old game folder and leave its `.dat` and `.bak` files in place
for the first launch. The port copies known files into a temporary folder,
then renames it into place once the copy is complete. It never deletes the
originals. It skips old `.tmp` files and leaves an existing destination
folder alone. If the migration fails, the game warns you and uses the
old location for that run. Check that the new folder exists, that your
profile and progress load, and that a backup can be restored before
archiving your original copy. Save formats are still Game v14/Profile v5.

## Fidelity priorities

**Wave pacing first.** In BLUS30852's wave update, the countdown code
compares remaining health strictly against an integer threshold
(`0x1985d8–0x1985f8`); the long countdown before a flag has a zero
threshold (`0x198588–0x198598`). The PSP was using less-than-or-equal,
so clearing the lawn could jump straight to its two-second acceleration.
The port now floors the computed threshold and uses the strict comparison.
Its established tutorial, ordinary, and warning countdown lengths are
unchanged. The health scan runs only while advancing the wave is possible.

**Next, compare representative levels.** Start with 1-1 and 1-2 tutorial
triggers, a standard day flag, final-wave reinforcements in Pool/Roof, and a
late Endless round. Record wave arrival and warning times on a low-damage
run and a fast-clear run. Then check shield breakage, detached heads, and
zombie falls with a controlled set of plants.

**Then, compare special mechanics and story.** Measure Zomboss's recovery
windows; capture how dancer backup summons, Heavy Weapon shots, Bobsled
dismounts and portal crossings behave. Verify the chapter-end notes and
the pickup sequence for a plant, a trophy and the final reward. Compare
ending music/video sync on hardware. This patch does not change those
scenes or claim complete PS3 timing parity.

## Validation

The PSP Street and PSP-1000 PPSSPP profiles each passed 1,408 checks, with
no failures or animation cache errors. Five new checks cover the flag
countdown and strict health comparison. Normal startup on the Street profile
also verified a fresh install, migration of 12 files, retry after a blocked
destination and incomplete staging folder, and restoration without the old
copies. An audio settings write went to the new folder. Physical PSP
Memory Stick testing is still outstanding.
