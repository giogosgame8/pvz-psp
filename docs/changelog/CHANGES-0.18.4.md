# 0.18.4 - clearer preparation progress

- Shows the current preparation stage, including music, menus, the ending and sounds.
- Stops counting music tracks and sound effects as graphics assets.
- Checks the real child-process progress display in the installer integration test.

The slowdown around the old 200-item counter included boss animations followed
by audio conversion and menu graphics. The old counter was not a percentage.
This update changes progress messages, not conversion speed or generated content.
The native PSP executable is unchanged from 0.18.1. Working installations do not
need to regenerate their files.
