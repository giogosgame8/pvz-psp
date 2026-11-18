# 0.24.2 — Smoother transitions

- Queue pause, victory, coin-banking and reward-pickup checkpoints off the drawing thread.
- Keep frequently used pause/reward/packet textures ready within existing cache limits.
- Reduce per-frame animation-cache bookkeeping.
- Make the crow travel between Home, More Ways to Play and Extras.

Uses the same prepared data and save formats as 0.24.1.
[Details and test limits](../reference/HITCHES-0.24.2.md).
