# 0.18.6 - faster PC preparation

- Packs RGBA4444 texture pixels with compiled Pillow channel operations instead
  of a Python loop for every pixel. Quantization and byte order are unchanged.
- Compresses independent texture frames concurrently, preserving frame order.
- Raises the graphics cap to 16 threads and sound conversion cap to 8 workers.
- Converts up to four soundtrack sections concurrently. Reuses a sample-value
  lookup table during music normalization, retaining the original rounding.
- Removes preparation of obsolete resident PCM effects that were discarded
  after the current streaming sound bank was generated.
- Renders the ending in up to 8 separate processes, with an independent scene
  clock and cache per process. The writer retains the original frame order.
- Limits ending queues to two chunks per worker. Workers run without extra
  console windows, including inside the portable Setup.exe.
- Selects worker counts from CPU capacity and available memory. Low-memory PCs
  use fewer workers; this does not change the generated graphics or audio.
- Keeps the standalone setup and its existing ISO picker and safety backups.
- Shows 100% when ending preparation finishes.

The PSP executable and prepared data format remain those of 0.18.5. Players
already running that version do not need to rebuild their installed game files.
No original disc resources are included in the public download.

Back-to-back full setup tests on an i5-14600K took 235.915 seconds with 0.18.5
and 93.540 seconds with 0.18.6 (60.4% less time). All 388 content and metadata
files matched byte for byte; only the ending progress diagnostic differed.
Both portable runs had Python and py removed from PATH. These are results for
one PC, not an average or a guaranteed time for other users.
