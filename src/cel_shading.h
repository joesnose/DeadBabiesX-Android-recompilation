#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

// Apply ink inside existing silhouettes. Never expand the sprite's alpha mask.
inline void NativeApplyCelShading(std::vector<unsigned char> &pixels,
                                 unsigned width, unsigned height) {
  if (!width || !height || pixels.size() != size_t(width) * height * 4) return;
  auto source = pixels;
  // Suppress fine photographic texture before banding so material detail does
  // not turn into speckled ink. Ignore transparent pixels to avoid dark halos.
  for (unsigned y = 0; y < height; ++y) {
    for (unsigned x = 0; x < width; ++x) {
      const size_t at = (size_t(y) * width + x) * 4;
      if (!pixels[at + 3]) continue;
      unsigned total = 0, sum[3]{};
      for (int dy = -1; dy <= 1; ++dy) for (int dx = -1; dx <= 1; ++dx) {
        const int nx = int(x) + dx, ny = int(y) + dy;
        if (nx < 0 || ny < 0 || nx >= int(width) || ny >= int(height)) continue;
        const size_t sampleAt = (size_t(ny) * width + nx) * 4;
        const unsigned weight = (dx == 0 ? 2 : 1) * (dy == 0 ? 2 : 1) * pixels[sampleAt + 3];
        total += weight;
        for (int c = 0; c < 3; ++c) sum[c] += pixels[sampleAt + c] * weight;
      }
      for (int c = 0; c < 3; ++c) source[at + c] = static_cast<unsigned char>((sum[c] + total / 2) / total);
    }
  }
  const int radius = std::clamp(int(std::min(width, height) / 160), 1, 3);
  auto light = [&](size_t at) {
    return (source[at] * 54 + source[at + 1] * 183 + source[at + 2] * 19) / 256;
  };
  for (unsigned y = 0; y < height; ++y) {
    for (unsigned x = 0; x < width; ++x) {
      const size_t at = (size_t(y) * width + x) * 4;
      if (!source[at + 3]) continue;
      bool silhouette = false;
      int contrast = 0;
      const int offsets[4][2] = {{-radius, 0}, {radius, 0}, {0, -radius}, {0, radius}};
      for (const auto &offset : offsets) {
        const int nx = int(x) + offset[0], ny = int(y) + offset[1];
        if (nx < 0 || ny < 0 || nx >= int(width) || ny >= int(height)) {
          silhouette = true;
          continue;
        }
        const size_t neighborAt = (size_t(ny) * width + nx) * 4;
        if (source[neighborAt + 3] < 48) silhouette = true;
        else contrast = std::max(contrast, std::abs(light(at) - light(neighborAt)));
      }
      // Outline the darker side of strong internal features to retain eyes,
      // armor seams and scenery panel lines without doubling bright edges.
      bool ink = silhouette || (contrast > 68 && light(at) < 135);
      if (ink) {
        for (int c = 0; c < 3; ++c) pixels[at + c] = source[at + c] / 5;
      } else {
        const float value = std::max({source[at], source[at + 1], source[at + 2]}) / 255.f;
        const float band = std::round(value * 5.f) / 5.f;
        const float luminance = light(at) / 255.f;
        for (int c = 0; c < 3; ++c) {
          const float saturated = luminance + (source[at + c] / 255.f - luminance) * 1.12f;
          const float result = saturated * band / std::max(value, .001f);
          pixels[at + c] = static_cast<unsigned char>(std::lround(std::clamp(result * 255.f, 0.f, 255.f)));
        }
      }
    }
  }
}
