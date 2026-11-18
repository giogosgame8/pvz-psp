# PvZ PSP 0.24.4 development

- After Pool Endless unlocks, open **More Ways to Play → Survival → Endless**
  and press **Square** for Bonus Endless: Day, Night, Fog and Roof. Each has
  its own flag record. These are playable PSP adaptations of hidden PS3
  entries and do not count toward the Gold Sunflower.
- Replay history and audio settings validate new writes and can recover from
  the previous valid copy. Player files still live in `PSP/SAVEDATA/PVZPSP`.
- Repositioning one portal no longer resets the exit protection of other
  portals. Zombotany peas strike Pumpkin first, and headless shooters stop
  firing. Late Survival budget math cannot wrap into a negative number.
- Dave's dialogue bubble appears when his talk animation starts. The ending
  decoder reads one frame ahead where it can, keeps displayed frames in
  order, and gives the final frame time to display before closing.
- The crow now perches on the Home gravestone, Mini-games gravestone and
  Extras glass orb, based on the PS3's authored menu poses.

Replace only `EBOOT.PBP` when updating from 0.24.1–0.24.3. Existing prepared
data and save formats remain compatible.
