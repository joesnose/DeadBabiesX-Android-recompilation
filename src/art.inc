static int artOverride = -1;
const char *NativeArtLabel(int mode) {
  return mode == 5 ? "CARTOON" : mode == 4 ? "CEL SHADED" : mode == 3 ? "SUPER SUPER HD" : mode == 2 ? "SUPER HD" : mode == 1 ? "HD" : "ORIGINAL";
}
bool NativeHasArtMode(int mode) {
  if (mode == 0 || mode == 4 || mode == 5) return true;
  const char *manifest = mode == 1 ? "game/hd/manifest.json" :
                         mode == 2 ? "game/superhd/manifest.json" :
                         mode == 3 ? "game/supersuperhd/manifest.json" : nullptr;
  return manifest && GetFileAttributesA(manifest) != INVALID_FILE_ATTRIBUTES;
}
bool NativeSetArtMode(int mode) {
  if (!NativeHasArtMode(mode)) return false;
  artOverride = mode;
  return true;
}
bool NativeHasHDArt() { return NativeHasArtMode(1); }
bool NativeSetHDArt(bool enabled) { return NativeSetArtMode(enabled ? 1 : 0); }
const char *NativeArtSkin() {
  if (artOverride == 0) return nullptr;
  if (artOverride == 1) return NativeHasArtMode(1) ? "hd" : nullptr;
  if (artOverride == 2) return NativeHasArtMode(2) ? "superhd" : nullptr;
  if (artOverride == 3) return NativeHasArtMode(3) ? "supersuperhd" : nullptr;
  if (artOverride == 4) return "celshaded";
  if (artOverride == 5) return "cartoon";
  if (getenv("DBX_ORIGINAL_ART")) return nullptr;
  if (getenv("DBX_CARTOON")) return "cartoon";
  if (getenv("DBX_CEL_SHADED")) return "celshaded";
  if (getenv("DBX_SUPER_SUPER_HD") && NativeHasArtMode(3)) return "supersuperhd";
  if (getenv("DBX_SUPER_HD") && NativeHasArtMode(2)) return "superhd";
  if (NativeHasArtMode(1)) return "hd";
  return nullptr;
}
