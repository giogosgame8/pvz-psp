# Validation — 0.24.1 scene corrections

17 November 2026. [Changes](SCENE-0.24.1.md).

Final bare EBOOT SHA-256: `bab97dc303ccfd48fa2f6d5ad9233f537d0ab89f8493b4cbea428175629fed6c`.

| Emulator profile | Passed checks | Failures | Cache errors | Run time |
| --- | ---: | ---: | ---: | ---: |
| Street, 60 FPS preference | 1389 | 0 | 0 | 106.14 s |
| PSP-1000, smaller caches, 60 FPS preference | 1389 | 0 | 0 | 109.16 s |

These are targeted scene/regression runs. They include the existing logic,
fifty synthetic campaign combat fixtures, menus, damaged artwork and decoded
movie samples, plus all captured review scenes. The long live-audio suite and
full ending playback were not repeated for this presentation-only patch.
All 24 prepared soundtrack/layer, Foley-bank and ending files are
byte-identical to the fully tested 0.24.0 resources.

New checks cover negative-coordinate rasterization, original pot placement,
shared boss origins and towering camera framing, source swimming clips, waterline anchors, full-health
heads and reuse of a headless actor slot. The two marked damage/full-health
scenes were reviewed alongside boss entry/exit, potted variants and armored
swimmers. Upright framing was compared with the PS3 recording linked in the
change report; lowered-head scenes retain the driver.

- Python preparation tests: **56 passed**; importer tests: **6 passed**.
- Fresh frozen Setup.exe completed in **256.139 seconds** on this PC
  with no Python on PATH. No general timing estimate is implied.
- Recipe `ps3-0241-1`: **649 PVA banks, 693 integrity-listed files**.
- The private XMB wrapper contains the tested executable payload unchanged.
- Normal startup reached the menu on both profiles; Street defaults to 60 FPS
  and PSP-1000 to 30 FPS. Both logged a 1.095-second first frame on this PC
  with diagnostics disabled; this is not a PSP boot-time measurement. The
  smaller 1000 cache limits remain enabled.

Physical-console acceptance, extended play and the broader PS3 fidelity work
remain outstanding. The scene-only benchmark flag skips unrelated long audio
playback after all visual reviews; it has no effect on normal gameplay.
