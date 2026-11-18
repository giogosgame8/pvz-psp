# PvZ PSP 0.24.5 development

- Heavy Weapon uses the disc's upgrade order, unlocks, prices, firing
  intervals, and pea spreads. Cattail is a separate weapon that can fire
  alongside Repeater or Threepeater. Snow Pea and Torchwood are exclusive
  six-second effects, and Doom-shroom has a central blast radius.
- The PSP uses the D-pad to move, the analog stick to aim, and X to fire.
  The top tray shows all six upgrades, and the elemental timer is visible.
- The menu crow follows the disc's arrival paths and the four directional
  routes between Home, More Ways to Play, and Extras. Paths are projected
  onto the PSP's menu layout and use the existing crow sprites.

Update from 0.24.1–0.24.4 by copying `EBOOT.PBP` and the included
`data/crow-paths.bin` into the existing game folder. Keep the rest of your
data and saves. Setup generates the path file from the disc on new installs.
