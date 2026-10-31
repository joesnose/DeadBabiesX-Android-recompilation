#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
: "${ANDROID_NDK_HOME:?Set ANDROID_NDK_HOME to Android NDK r27c}"
: "${ANDROID_JAR:?Set ANDROID_JAR to API 35 android.jar}"
: "${BUILD_TOOLS_DIR:?Set BUILD_TOOLS_DIR to Android build-tools 35.0.0}"
if [ -n "${JAVA_HOME:-}" ]; then export PATH="$JAVA_HOME/bin:$PATH"; fi
for command in javac java cmake ninja python3 keytool tar sha256sum; do command -v "$command" >/dev/null; done
(cd third_party && sha256sum -c SHA256SUMS)
if [ ! -d third_party/SDL2-2.32.10 ]; then tar -xzf third_party/SDL2-2.32.10.tar.gz -C third_party; fi
cmake -S . -B build-arm64 -G Ninja \
 -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
 -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26 \
 -DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-arm64 --parallel "${BUILD_JOBS:-4}"
mkdir -p build-apk/classes build-apk/dex build-apk/package/lib/arm64-v8a
find android/java -name '*.java' > build-apk/sources.txt
javac -source 8 -target 8 -classpath "$ANDROID_JAR" -d build-apk/classes @build-apk/sources.txt
find build-apk/classes -name '*.class' > build-apk/classfiles.txt
"$BUILD_TOOLS_DIR/d8" --min-api 26 --lib "$ANDROID_JAR" --output build-apk/dex @build-apk/classfiles.txt
NDK_LLVM="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64"
cp build-arm64/libmain.so build-arm64/sdl/libSDL2.so build-apk/package/lib/arm64-v8a/
cp "$NDK_LLVM/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so" build-apk/package/lib/arm64-v8a/
for lib in build-apk/package/lib/arm64-v8a/*.so; do "$NDK_LLVM/bin/llvm-strip" --strip-unneeded "$lib"; done
"$BUILD_TOOLS_DIR/aapt2" link -o build-apk/unsigned.apk --manifest android/AndroidManifest.xml -I "$ANDROID_JAR" -A android/assets --min-sdk-version 26 --target-sdk-version 35
python3 - <<'PY'
from pathlib import Path
import zipfile
with zipfile.ZipFile('build-apk/unsigned.apk','a',zipfile.ZIP_DEFLATED) as z:
 z.write('build-apk/dex/classes.dex','classes.dex')
 for p in Path('build-apk/package').rglob('*.so'): z.write(p,str(p.relative_to('build-apk/package')))
PY
"$BUILD_TOOLS_DIR/zipalign" -f -P 16 4 build-apk/unsigned.apk build-apk/aligned.apk
SIGNING_KEYSTORE="${SIGNING_KEYSTORE:-$PWD/.local/signing/development.jks}"
export DBX_STORE_PASSWORD="${DBX_STORE_PASSWORD:-android}"
export DBX_KEY_PASSWORD="${DBX_KEY_PASSWORD:-$DBX_STORE_PASSWORD}"
SIGNING_ALIAS="${SIGNING_ALIAS:-dbx}"
if [ ! -f "$SIGNING_KEYSTORE" ]; then
 mkdir -p "$(dirname "$SIGNING_KEYSTORE")"
 keytool -genkeypair -keystore "$SIGNING_KEYSTORE" -storepass:env DBX_STORE_PASSWORD -keypass:env DBX_KEY_PASSWORD -alias "$SIGNING_ALIAS" -keyalg RSA -keysize 3072 -validity 10000 -dname 'CN=DeadBabiesX Android Development'
fi
OUTPUT_APK=build-apk/DeadBabiesX-Android-v1.4-LAN.apk
"$BUILD_TOOLS_DIR/apksigner" sign --ks "$SIGNING_KEYSTORE" --ks-key-alias "$SIGNING_ALIAS" --ks-pass env:DBX_STORE_PASSWORD --key-pass env:DBX_KEY_PASSWORD --out "$OUTPUT_APK" build-apk/aligned.apk
"$BUILD_TOOLS_DIR/apksigner" verify --verbose "$OUTPUT_APK"
"$BUILD_TOOLS_DIR/zipalign" -c -P 16 4 "$OUTPUT_APK"
sha256sum "$OUTPUT_APK"
echo "Built $OUTPUT_APK"
