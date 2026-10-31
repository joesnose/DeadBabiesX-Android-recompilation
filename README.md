# Dead Babies X — PS Vita native port preview

**[Download the Vita VPK](downloads/DeadBabiesX-Vita-v0.1.vpk?raw=true)**

Built from the native C++ game source using VitaSDK and SDL 2.32.8's native GXM renderer. This is a **first hardware-test preview**, not a confirmed-working Vita release. The ARM executable, SELF and VPK have been built and package integrity checked; no physical Vita was available for testing.

## Install

Use VitaShell on a homebrew-enabled PS Vita or PS TV to install `DeadBabiesX-Vita-v0.1.vpk`. Launch the Dead Babies X bubble. All game assets are included; no Android APK, browser, Windows emulator, OpenGL plugin or external shader compiler is required.

Title ID: `DBXV00001`. Saves, controller mappings, last host IP and `native.log` use `ux0:data/DeadBabiesX/` and are separate from the installed app.

## Controls

| Vita control | Action |
|---|---|
| Left stick / D-pad | Move |
| Right stick | Aim |
| R | Fire |
| L | Alternate fire |
| Cross | Collect pickup / confirm |
| Square / Triangle | Specials 1 / 2 |
| Start | Pause / resume |
| Circle / Select | Back |

Controller Mapping remains available in the main menu. Android-style virtual stick/button overlays are hidden on Vita. Menu touch targets from the shared game code remain where supported; use physical controls for the Vita LAN setup screen.

## Graphics and audio

The 4:3 playfield is centered on Vita's 960×544 display. Original graphics are selected initially. Other graphics choices remain available; the previous theme's textures are released when changing style to avoid accumulating all packs in memory. Actual frame rate and memory behavior need device testing.

Music is streamed from Ogg rather than decoded into large permanent PCM buffers. The original music was resampled to 48 kHz for the Vita streaming mixer; other tracks already use 48 kHz. Sound effects and result announcements remain included. The app uses the standard 444 MHz CPU setting while running, restoring the previous clock on normal exit.

## LAN preview

Host and Join are available from LAN Multiplayer. Use the same Wi-Fi. The host displays its IP. For Join, select an IP octet with Left/Right, adjust it with Up/Down, use Square/Triangle for +/-10, then Cross/Start to connect. The host uses Cross/Start to begin after guests join.

The shared v1.5 protocol includes pause menus and Play Again without rejoining. Vita-to-Vita and Vita-to-Android interoperability have **not** been verified on hardware. Connection failures are reported on screen and in the save directory's logs.

## Build

Install VitaSDK (tested with the Linux `master-linux-v2.540` toolchain) and the VitaSDK `sdl2` package (2.32.8). Then:

```sh
export VITASDK=/path/to/vitasdk
export PATH="$VITASDK/bin:$PATH"
cmake -S . -B build -G Ninja
cmake --build build
```

The output is `build/DeadBabiesX-Vita-v0.1.vpk`. Assets and LiveArea resources are packaged automatically. Obsolete PSP art-source files, WMA music and thumbnail databases are excluded from the VPK.

## Credits

Original game/code/art: Chris Friesen and credited contributors; see `notices/ORIGINAL-GAME.txt` and `src/original/`. Native port/platform adaptations were developed with AI assistance. Component licences remain applicable; see the notices bundled in `assets/` and the embedded stb licences. No blanket relicensing of original game assets is intended.
