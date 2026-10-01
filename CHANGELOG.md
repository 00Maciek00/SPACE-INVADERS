# Changelog

All notable changes to this project are documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project follows [Semantic Versioning](https://semver.org/).

## [1.1.0] - 2026-10-01

### Added
- Bilingual PL / EN interface (auto-detected from the Windows UI language, switchable in the main menu).
- Invaders now destroy the bunker blocks they touch.
- "NEW HIGH SCORE!" message on the Game Over screen.
- Game version and compiler shown in the main menu.
- `build.bat`: automatic MSVC / MinGW-w64 detection (vswhere), compiler and version info,
  `debug`, `clean`, `msvc`, `mingw` options, proper exit codes.
- GitHub Actions workflow, `LICENSE`, `NOTICE`, `README` (bilingual), `.gitignore`, `.gitattributes`.

### Changed
- Fixed 60 Hz time step based on `QueryPerformanceCounter` (replaces `WM_TIMER`,
  which is quantized to ~15.6 ms and gave 32 or 64 FPS instead of 60).
- Save files moved to `%LOCALAPPDATA%\SpaceInvadersExtended\` (works from read-only locations).
- Save file has a header (magic + version) and is validated on load.
- Enemy fire rate now scales with the wave from wave 2 (previously only from wave 9).
- Text is rendered through Unicode APIs (correct Polish diacritics).
- Key presses are latched, so a quick tap shorter than one frame is never lost.
- Menu rows use tighter spacing to fit the new "Language" row.

### Fixed
- Background stars were drawn over the HUD and missing at the bottom of the play field.
- Holding `P` / `ESC` toggled the pause repeatedly (key auto-repeat).
- Keys stayed "pressed" after the window lost focus; the game now pauses on focus loss.
- High score did not include the +500 wave bonus and was not saved when leaving via the menu / closing the window.
- Corrupted save file could cause an out-of-range difficulty index.
- After "Delete save" the menu cursor jumped to a random row.
- Screen shake left "ghost" borders of the previous frame.
- Help bar claimed `ESC` opens the menu (it pauses the game).
- Run state (UFO index, shake, flash message) was not reset when starting a new game.
- Missing Polish diacritics (`STRZAl`, `DOl`, ...).
- Duplicated high-score code consolidated into `commitHiScore()`.
- MSVC warning C4005 (`_CRT_SECURE_NO_WARNINGS` defined twice).

## [1.0.0] - 2026-09-30

### Added
- Initial release: classic Space Invaders gameplay, 5 enemy types, bunkers, UFO,
  power-ups, particles, screen shake, procedural sound, high score and save game, menu and pause.
