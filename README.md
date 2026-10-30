# DeadBabiesX-Android-recompilation
This is a recompiled original Xbox homebrew game called DeadBabies xtreme. I was able to find the source code and get AI to recompile it for android. This is 100% AI generated. Its a great retro vibed game that i always loved on Xbox, a few option have been added but it remains basically the same.

Native Android port of Dead Babies Xtreme, with touch controls, controller support, selectable graphics themes, and music. Based on the original game by Chris Friesen and contributors.

**[Download Android v1.2 APK](downloads/DeadBabiesX-Android-v1.2.apk?raw=true)** · [Checksums](downloads/SHA256SUMS)

Requires Android 8.0 or later on a 64-bit ARM phone or tablet. Download the APK, open it, and allow installation from your browser or file manager if Android asks. Version 1.2 installs over the previous Android preview builds and keeps the high score.

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

Version 1.2 assigns the controller that starts the round to **player one**, even when Android enumerates a passive input device first. A solo round started by touch adopts the first controller used. Additional controllers remain available for local players. This port does not add internet multiplayer.

## Build on Linux

Install JDK 21, Python 3, CMake 3.20+, Ninja, Android NDK **r27c**, Android build-tools **35.0.0**, and Android platform **35**. SDL **2.32.10** source is bundled in `third_party/` with a checksum, so no SDL download is required during the build.

```bash
export JAVA_HOME=/path/to/jdk-21
export ANDROID_NDK_HOME=/path/to/android-ndk-r27c
export BUILD_TOOLS_DIR=/path/to/android-sdk/build-tools/35.0.0
export ANDROID_JAR=/path/to/android-sdk/platforms/android-35/android.jar
./build-android.sh
```

The signed APK is written to `build-apk/DeadBabiesX-Android-v1.2.apk`. The script builds the C++ game and SDL, compiles the Android activity, bundles the assets and ARM64 libraries, aligns the package, signs it, and verifies the signature.

A local development signing key is generated under `.local/signing/` unless `SIGNING_KEYSTORE` and `SIGNING_ALIAS` are provided. Passwords can be supplied with `DBX_STORE_PASSWORD` and `DBX_KEY_PASSWORD`; development defaults are `android`. Signing keys are excluded from Git. A self-built APK signed with your own key cannot update an installed preview signed by the maintainer without first uninstalling it, which removes local saves.

## Source and validation

- `src/`: C++ gameplay, Android platform layer, touch/controller input, renderer, and audio mixer.
- `android/java/`: Android activity and SDL Java bindings.
- `android/assets/`: packaged game assets, music, controller mappings, and dependency notices.
- `android/AndroidManifest.xml`: Android package and version settings.
- `tests/`: recorded checks from the v1.2 build.

Native ARM64 compilation, APK v2/v3 signing, 16 KB native-library alignment, touch hit targets at three aspect ratios, virtual-controller input, and gameplay self-tests passed. An integration test exercised the real gameplay update with a passive first device and an active second controller, including movement, aiming, projectiles, pause/resume, and touch-start controller adoption.

Physical Android/controller testing remains necessary. Offscreen EGL rendering could not be verified in the cloud environment. This is a preview build.

## Credits and permissions

Original game/code/art: **Chris Friesen** and the contributors credited in [the original readme](src/original/readme.txt). Original source and attribution are retained under `src/original/`.

This repository contains components with their own terms; no new blanket license is applied to the game or its artwork. See the original readme, the licenses embedded in the stb source files, and the SDL, controller mapping, and font notices in `android/assets/`. Graphics and music additions are inherited from the existing public-game build.
