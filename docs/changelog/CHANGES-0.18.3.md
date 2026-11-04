# 0.18.3 - faster PC preparation

- Avoids transforming empty parts of oversized animation canvases.
- Uses up to eight rendering workers and four concurrent sound conversions.
- Keeps the original pixel coordinates, interpolation, compression and sample order.
- Adds pixel-comparison tests against the original compositor.

This update speeds up preparation on the computer. Existing prepared game files
remain compatible. The native PSP executable is unchanged from 0.18.1.
