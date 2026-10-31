# Dead Babies X Native — Windows build v9 — Graphics Toggle and Mega Weapons

A native Windows port of Chris Friesen's Dead Babies Xtreme source. Gameplay,
weapon constants, collisions, animation and art come from the original code.
The Xbox emulation/recompilation runtime is not linked into this executable.

## Run

1. Extract the download into a new folder.
2. This preview includes the supplied `game/data` folder and a separate
   `game/hd` skin. No additional game files are needed. It does not need
   `default.xbe`. Keep the game folder beside `DeadBabiesXNative.exe`.
3. Keep `libwinpthread-1.dll` and `music3.ogg` beside the EXE.
4. Run `DeadBabiesXNative.exe`, or add that EXE to Steam. On Linux, select a
   Proton compatibility tool and enable Steam Input.

The normal Windows build requires Windows 10 or later and an OpenGL driver.
The game opens in borderless fullscreen at your desktop resolution and targets
60 FPS. On a 1920×1080 display, the 4:3 game area is **1440×1080**, centered
with 240-pixel black bars on each side. Original artwork is scaled; it does
not gain new detail. The 640×480 logical playfield and gameplay are retained.
F11 or Alt+Enter switches to/from a 960×720 window. Escape opens the pause menu.
Resizing preserves the aspect ratio.

## Controls

Controller: left stick moves, right stick aims, right trigger fires, left
trigger uses alternate fire, Start opens the pause menu for an active player (or joins for a new player). Shoulder buttons map to the
original black/white special-weapon buttons. Supports up to four XInput pads.

Keyboard: A/D moves, arrows aim, Space or left mouse fires, Shift or right mouse
uses alternate fire, Enter joins/opens the pause menu, Q/E use black/white specials.

After choosing Start Game, the selected number of players starts immediately.
Keyboard controls player one; additional players use their own controllers.
The original drop-in joining behavior remains available during play. Weapons are normally random
and change through pickups, just as in the original.

## Title and pause menus

The title screen has **Start Game**, **Players**, **Graphics**, **Mega Weapons**
and **Exit Game**. Use arrows, D-pad or stick to select. Left/Right adjusts an
option; Enter/A/Start also cycles it. Players selects 1-4 starting players.
Escape/B selects Exit; confirmation is required.

**Graphics: Original / HD** switches immediately. It is also on the pause menu:
**Resume / Graphics / Title Screen / Exit Game**. Pausing freezes gameplay/audio.
You can compare the same scene in Original and HD without restarting or losing
scores, positions, weapons, active effects or the selected weapon mode. The
original and HD textures stay cached, so repeated switches reuse them. If HD
files are absent, Original remains available and the menu explains the missing
HD files. The packaged build includes both art choices.

**Mega Weapons** is Off by default. Choose it before starting a round:
- Off: maximum weapon level **3**, preserving the original weapon progression.
- On: maximum weapon level **5**. Repeated pickups of the same weapon upgrade
  from level 1 through 2, 3, 4 and 5, and further duplicates stay at 5.

Collect weapon crates with controller A as in the original game. Selecting a
different weapon or respawning still starts that weapon at level 1. Level 4/5
machine-gun bullets deal 4/5 damage with increased impulse; plasma adds a
symmetric 4/5-shot primary fan; rockets launch 4/5 primary or 12/15 alternate
rockets; grenade fragments rise to 44/56 normally or 80/100 when remotely
triggered; railgun cooldown falls to 7.5/6 ticks; flame lifetime/area continues
the original scaling. The charged plasma burst retains its original bound.
The HUD shows **Lx/3** or **Lx/5**, with the power meter fitting either cap.

Player count, art selection and Mega Weapons are retained between rounds and
when returning to the title within one launch. Mega Weapons resets to Off on
relaunch. HD starts selected when installed; the legacy `DBX_ORIGINAL_ART=1`
option starts with Original selected, and the menu can still change it.

Score text remains centered and shrinks to fit large scores. All prior results,
saved solo high score, pause/exit and 4:3 fullscreen features remain available.

## Native implementation

- OpenGL handles textured sprites, tracers, alpha and additive blending.
- WIC decodes the original PNG files. The original opaque-black colour key is
  retained; otherwise the layered background and HUD would become black boxes.
- XInput handles pads; keyboard controls are also provided.
- A bounded PCM mixer plays original WAV effects. `music3.ogg` is a conversion
  of the existing game music, decoded with stb_vorbis, so Proton does not need
  a WMA codec. Game speed changes preserve gameplay timing; original Xbox DSP
  effects and audio pitch processing are not reproduced.
- Simulation uses the original 60 Hz tick units, including slow-time effects.
  It advances one simulation tick per presented frame; sustained rendering
  below 60 FPS will slow simulation. This first build is capped at 60 FPS.
- Unsafe list-removal loops are replaced with saved-next iteration. Numeric
  members are initialized. Weapons/players remain owned for the round so
  active projectiles cannot refer to freed owners; round reset releases them.
- Machine-gun/flame projectiles originate from the original moving muzzle
  positions. Flames retain their originating player independently of pickups.

First-build differences: a generated Windows font atlas replaces Xbox bitmap
fonts. The absent intro movies and inactive legacy menu are omitted. Exact
pixel/audio equivalence and every multiplayer/special-weapon combination have
not been verified. This is a test build, not a finished remaster.

## Logs

`native.log` is written beside the EXE and includes renderer and FPS reports.
For a problem, send that log. The previous port's `recomp.log` is not used.

## Build from source

No Xbox SDK is required. Use CMake and a Windows C/C++ compiler with the
Windows SDK, or MinGW-w64. On Windows with Visual Studio:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

The project links Windows system libraries plus OpenGL. MinGW builds also need
its `libwinpthread-1.dll` alongside the EXE. The asset folder is not compiled in.

Run `DeadBabiesXNative.exe --self-test` to check original collision behavior
and machine-gun/flame muzzle coordinates without opening the game window.
Results go to `native.log`.

Optional automated testing environment variables (unset for normal play):
`DBX_STRESS=1` supplies movement/aim/fire and keeps players invincible;
`DBX_TEST_WEAPON=1..6` fixes spawn/pickup weapon type;
`DBX_TEST_SECONDS=N` exits after N seconds;
`DBX_TEST_RESTART_AT=N` tests a round reset after N simulated seconds;
`DBX_UNCAPPED=1` disables pacing (also speeds simulation; testing only).
`DBX_TEST_SCORE=N` and `DBX_TEST_PLAYERS=1..4` preview HUD values and slots.
`DBX_STRESS` skips the title screen for automated gameplay tests.

## Credits and source provenance

Original game/code/art: Chris Friesen (parlance), with the contributors named
in the original game's Readme.txt. The supplied original source's permission
and attribution request are retained in `original/readme.txt`. Its code is
reused here; the native platform layer and portability fixes are new.

Vorbis decoder: Sean Barrett and stb contributors, v1.22, vendored from
nothings/stb commit f0569113c93ad095470c54bf34a17b36646bbbb5. Its MIT/public-domain
license is included at the end of `vendor/stb_vorbis.c`.

Set `DBX_WINDOWED=1` before launch to start windowed instead of fullscreen.

## Game over results

Balloon babies float across the game over screen. Multiplayer shows the
highest-scoring player as WINNER in their cursor colour (red, yellow, blue,
green); ties list all joint winners in their respective colours. Eliminated
players remain eligible. Solo shows LAST GAME and HIGH SCORE instead.
The solo record is saved in `%LOCALAPPDATA%\DeadBabiesXNative\highscore.txt`
(inside the game's Proton prefix on Linux), and survives EXE upgrades when
using the same prefix. Only completed solo rounds update this record.
After two seconds, Enter/A/Start returns to the title screen. Escape still
opens the pause menu with Exit Game.

For results testing only, `DBX_TEST_GAMEOVER_AT=N` eliminates the players after
N simulation seconds; `DBX_TEST_RESULTS=a,b,c,d` supplies fixture scores when
that end-of-round test is enabled. Leave both unset for normal play.

The multiplayer results scoreboard lists every participating player in descending
score order, including eliminated players. Equal scores share a rank; tied rows
keep player-number order. Winner rows are marked with a star and each player
uses their own cursor colour. Solo last-game and saved high-score displays remain.

## Original artwork HD preview

This version returns to the supplied original artwork. It enhances 81 texture
exports: the original human players in all four colours, babies and animation
poses, pickup crates, aiming cursors, selected weapon sprites, the brown/gold
industrial wall, rock floor and green foreground trim. It keeps the original
designs and palette. The original score panel, soft effects and remaining
smaller/legacy assets are retained. It is not the smooth-cartoon redesign.

Sprites export at 4x the original canvas resolution; the main wall exports
at 2x. Original sprite alpha silhouettes constrain the enhanced art, preserving
transparent gaps in cursor/crate textures and the original canvas alignment.
Human skin and hair are retained when exporting colour variants; clothing
colours follow the corresponding original player textures. Enhanced sheets
were generated from the supplied original sheets, then exported into individual
textures; some fine details are reconstructed and are not historically exact.

Original files remain in `game/data`. The HD skin is in `game/hd/data`.
This skin takes priority when installed, including when updating over an older
cartoon preview. Missing HD textures use original art, so old cartoon files
cannot leak into this version. The original graphics, gameplay physics,
collision sizes, weapon constants, spawns and controller behavior are preserved.
The renderer still skips empty transparent margins to avoid unnecessary work.

To compare original artwork on Windows, run `Launch Original Art.cmd`.
On Steam/Proton, use launch options `DBX_ORIGINAL_ART=1 %command%`.
Remove that option to return to the enhanced original art. Keep your existing
Steam entry and Proton prefix when updating to retain its saved high score.

Source restoration sheets and export metadata are in `source/hd_art`.
With Python 3 and Pillow installed, run from the package root:
`python source/export_hd_assets.py --game-data game/data --output game/hd/data`.

This is an HD preview, not a claim of recovered original high-resolution assets.
Original artwork and source credits remain as documented above.

## v9 validation fixtures

`DBX_TEST_MEGA=1` starts the menu with Mega Weapons selected for automated runs.
`DBX_TEST_PICKUPS=1` spawns eight matching crates at player one and supplies
pickup input through the normal collection path. Combined with `DBX_STRESS=1`
and `DBX_TEST_WEAPON=1..6`, this verifies the real pickup cap and firing behavior.
Leave these testing options unset for normal play.
