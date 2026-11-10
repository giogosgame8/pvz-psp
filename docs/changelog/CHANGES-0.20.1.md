# 0.20.1 — simpler Credits menu

Removed Animation Gallery from Options > Credits and extras. The smaller panel
contains only Watch the ending and Back, with D-pad selection and X activation.
The Almanac remains available through Extras.

The 0.20.0 prepared data and Setup.exe remain compatible. When updating from
0.20.0, only EBOOT.PBP needs replacement. Gameplay saves and preferences are
unchanged. Older installations can use the full download and Setup.exe normally.

[Known feature and test backlog](../reference/FEATURE-BACKLOG.md)

Validation: 1,151 native checks passed in each of the Street (60 FPS target)
and PSP-1000 (30 FPS target) emulator profiles, with no cache errors. Existing
menu tests now verify two-entry wrapping, Back activation and ending activation.
The compact panel was checked in a native screenshot. Full ending playback was
not repeated for this menu-only change; decoder samples and ending activation
passed. Physical hardware remains unverified.

Bare EBOOT SHA-256:
`f3ead66f2754ce501302eb74a2ff2938f1f314acd9b031dff1b90768009a3cd0`.
