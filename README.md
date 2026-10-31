# DeadBabiesX-Android-recompilation
This is a recompiled original Xbox homebrew game called DeadBabies xtreme. I was able to find the source code and get AI to recompile it for android. This is 100% AI generated. Its a great retro vibed game that i always loved on Xbox, a few option have been added but it remains basically the same. 4 player was great but have not tested it on android yet.

Native Android port of Dead Babies Xtreme, with touch controls, controller support, selectable graphics themes, and music. Based on the original game by Chris Friesen and contributors.

**[Download Android v1.4 LAN APK](downloads/DeadBabiesX-Android-v1.4-LAN.apk?raw=true)** · [Checksums](downloads/SHA256SUMS)

Requires Android 8.0 or later on a 64-bit ARM phone or tablet. Download the APK, open it, and allow installation from your browser or file manager if Android asks. Version 1.4 installs over the previous Android preview builds and keeps the high score.

## Controls

| Input | Action |
|---|---|
| Left touch pad / left controller stick / D-pad | Move |
| Right touch pad | Aim and fire |
| Right controller stick | Aim |
| RT / LT | Fire / alternate fire |
| A | Collect pickups / confirm menu |
| RB / LB | Special 1 / Special 2 |
| Start | Pause / join |
| B | Back in menus |

Touch menus by tapping a row. Pickup and SP1 are on the left side; Alt and SP2 are on the right. Pause is at the top right. Pair Bluetooth controllers in Android settings or connect a compatible USB controller before use.

Version 1.2 assigns the controller that starts the round to **player one**, even when Android enumerates a passive input device first. A solo round started by touch adopts the first controller used. Additional controllers remain available for local players. Ordinary local controller play remains available. LAN multiplayer connects phones on the same network; it does not use an internet matchmaking server.

## v1.4 LAN stability and controller mapping

LAN simulation now uses explicit deterministic trigonometry, vector normalization, and square root rather than device-dependent math-library results. ARM64 floating-point rounding, denormal handling, and NaN behavior are standardized before simulation updates. This addresses a source of exact-state mismatches across different Android devices; strict sync checks remain enabled. All phones must install v1.4 because its LAN protocol is incompatible with earlier previews. Physical-device confirmation is still needed.

### Physical controller mapping

Choose **Controller Mapping** on the main menu. Tap an action (or use the D-pad/stick and A), release held buttons, then press the physical button or trigger to assign. Actions include Fire, Alt Fire, Pickup/Confirm, both specials, Pause/Start, and Back/Select. Assigning an already-used control swaps the existing assignment. **Sticks** swaps movement and aiming; **Reset Defaults** restores the original layout; **Done** returns to the title.

Mappings are saved on that phone and apply to its connected physical controllers. Touch controls are unchanged. The editor itself always uses the standard physical A/B/D-pad controls or touch so remapping does not lock you out.

### Validation

- LAN matches: 14,400 synchronized frames (about four minutes at 60 Hz) with two processes and 7,200 frames with four processes, with matching state hashes and active gameplay throughout.
- Deterministic math: one million square-root cases match the correctly rounded reference; trigonometry/atan accuracy checks and GCC/Clang output checksums match.
- Controller mapping: persistence, button capture/release guarding, applied inputs, stick swap, reset, and invalid-preference fallback tested with a virtual controller.
- Relaunch, input, controller assignment, disconnect/desync handling, malformed network messages, and results regression checks retained.

These are native automated checks, not physical Android-to-Android Wi-Fi validation. An error now includes the failing frame on both peers; local diagnostics are written to the app's private `lan-diagnostics.txt` file.

## v1.3.1 relaunch fix

Fixed Exit Game preventing a second launch while Android keeps the app process alive. Every native launch now resets the exit/background flags, input, round ownership, texture cache and audio channels. Shutdown also releases round resources. A regression test exits and starts gameplay three times in the same process. Install this update over v1.3; no uninstall or data clearing is needed. Physical-device relaunch confirmation remains necessary.

## LAN multiplayer (v1.3)

1. Install **v1.4 on every phone** and connect them to the same Wi-Fi.
2. On one phone choose **LAN Multiplayer → Host**. Share the displayed Wi-Fi IPv4 address.
3. On the other phones choose **LAN Multiplayer → Join** and enter that address.
4. The host taps **Start match** once everyone has joined. Two to four phones are supported, one player per phone. The host is player one; the other phones get players two to four. A label identifies your player.

The host's Mega Weapons setting applies to the room. Each phone can keep its own graphics and music choices. Existing touch controls, side action buttons, and controllers work for that phone's assigned player.

Start or the touch Pause button pauses/resumes the shared match. Android Back or the controller Back/Select button leaves the room, ending the match for everyone. Keep every phone in the foreground: backgrounding a phone cancels its LAN session. After results or disconnection, host/join a new room for another round.

The room uses direct TCP on port **39828**. No account, external server, or port forwarding is needed. Guest Wi-Fi/client isolation can block direct connections; use a network that allows devices to communicate. If several private IP addresses are listed (for example, with a VPN), use the phone's actual Wi-Fi address from Android network details.

Inputs and random seeds are shared for every simulation step, with state checks detecting divergence. A slow phone slows the shared simulation; a timed-out or desynchronized match returns to the title rather than continuing with different game states. Ordinary local play is available through Start Game.

### LAN validation

Automated tests use independent native processes and real TCP sockets with the same gameplay update code as the APK:

- Two-player and four-player matches with matching state hashes, movement and projectiles.
- Shared pause/resume, game-over/winner flow, and return to title.
- Disconnect and deliberate desynchronization handling.
- Malformed packet sizes, incompatible versions, invalid slots, and truncated messages.
- Quick touch Pause/results taps retained while the network waits.
- Existing touch, controller assignment, and gameplay regressions.

Physical Android-to-Android Wi-Fi testing has **not** been performed. LAN play is a preview. All peers must use this same APK. See `tests/lan-integration.log` and `tests/run_lan_integration.py` for the reproducible native checks.

## Build on Linux

Install JDK 21, Python 3, CMake 3.20+, Ninja, Android NDK **r27c**, Android build-tools **35.0.0**, and Android platform **35**. SDL **2.32.10** source is bundled in `third_party/` with a checksum, so no SDL download is required during the build.

```bash
export JAVA_HOME=/path/to/jdk-21
export ANDROID_NDK_HOME=/path/to/android-ndk-r27c
export BUILD_TOOLS_DIR=/path/to/android-sdk/build-tools/35.0.0
export ANDROID_JAR=/path/to/android-sdk/platforms/android-35/android.jar
./build-android.sh
```

The signed APK is written to `build-apk/DeadBabiesX-Android-v1.4-LAN.apk`. The script builds the C++ game and SDL, compiles the Android activity, bundles the assets and ARM64 libraries, aligns the package, signs it, and verifies the signature.

A local development signing key is generated under `.local/signing/` unless `SIGNING_KEYSTORE` and `SIGNING_ALIAS` are provided. Passwords can be supplied with `DBX_STORE_PASSWORD` and `DBX_KEY_PASSWORD`; development defaults are `android`. Signing keys are excluded from Git. A self-built APK signed with your own key cannot update an installed preview signed by the maintainer without first uninstalling it, which removes local saves.

## Source and validation

- `src/`: C++ gameplay, Android platform layer, touch/controller input, renderer, and audio mixer.
- `android/java/`: Android activity and SDL Java bindings.
- `android/assets/`: packaged game assets, music, controller mappings, and dependency notices.
- `android/AndroidManifest.xml`: Android package and version settings.
- `tests/`: recorded checks from the current build.

Native ARM64 compilation, APK v2/v3 signing, 16 KB native-library alignment, touch hit targets at three aspect ratios, virtual-controller input, and gameplay self-tests passed. An integration test exercised the real gameplay update with a passive first device and an active second controller, including movement, aiming, projectiles, pause/resume, and touch-start controller adoption.

Physical Android/controller testing remains necessary. Offscreen EGL rendering could not be verified in the cloud environment. This is a preview build.

## Credits and permissions

Original game/code/art: **Chris Friesen** and the contributors credited in [the original readme](src/original/readme.txt). Original source and attribution are retained under `src/original/`.

This repository contains components with their own terms; no new blanket license is applied to the game or its artwork. See the original readme, the licenses embedded in the stb source files, and the SDL, controller mapping, and font notices in `android/assets/`. Graphics and music additions are inherited from the existing public-game build.
