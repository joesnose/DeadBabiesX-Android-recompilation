#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

struct SoundData { std::vector<float> samples; };
struct NativeAudioVoice {
  std::shared_ptr<SoundData> sound;
  size_t pos = 0;
  bool loop = false;
  uint64_t id = 0;
  float gain = .22f;
  bool paused = false;
  bool playDuringGamePause = false;
  bool announcement = false;
  bool music = false;
};

inline void NativeMixAudio(std::vector<NativeAudioVoice> &voices, short *output,
                           size_t frames, bool paused) {
  static thread_local std::vector<float> mixed;
  mixed.assign(frames * 2, 0.f);
  // Duck only the music while a result announcement is active.
  const bool announcing = std::any_of(voices.begin(), voices.end(), [](const auto &v) {
    return v.announcement && !v.paused && v.sound && v.pos < v.sound->samples.size();
  });
  for (auto &voice : voices) {
    if ((paused && !voice.playDuringGamePause) || voice.paused || !voice.sound) continue;
    const auto &samples = voice.sound->samples;
    const size_t count = samples.size() & ~size_t(1);
    if (!count) continue;
    for (size_t i = 0; i < mixed.size(); i += 2) {
      if (voice.pos >= count) {
        if (!voice.loop) break;
        voice.pos = 0;
      }
      mixed[i] += samples[voice.pos++] * voice.gain * (voice.music && announcing ? .18f : 1.f);
      mixed[i + 1] += samples[voice.pos++] * voice.gain * (voice.music && announcing ? .18f : 1.f);
    }
  }
  voices.erase(std::remove_if(voices.begin(), voices.end(), [](const auto &v) {
    return !v.sound || v.sound->samples.size() < 2 ||
           (!v.loop && v.pos >= (v.sound->samples.size() & ~size_t(1)));
  }), voices.end());
  for (size_t i = 0; i < mixed.size(); ++i)
    output[i] = static_cast<short>(std::clamp(mixed[i], -1.f, 1.f) * 32767.f);
}

// A background track is a singleton, independent of transient channel tokens.
inline uint64_t NativeReplaceMusic(std::vector<NativeAudioVoice> &voices,
                                  uint64_t previous, uint64_t next,
                                  const std::shared_ptr<SoundData> &sound,
                                  float gain) {
  voices.erase(std::remove_if(voices.begin(), voices.end(),
                             [&](const auto &v) { return previous && v.id == previous; }),
               voices.end());
  if (!sound || sound->samples.size() < 2) return 0;
  voices.push_back({sound, 0, true, next, gain, false, true, false, true});
  return next;
}

// Round-owned objects keep their channel tokens until reset; remove voices only.
inline size_t NativeStopEffects(std::vector<NativeAudioVoice> &voices, uint64_t musicId) {
  size_t before=voices.size();
  voices.erase(std::remove_if(voices.begin(),voices.end(),[&](const auto &voice){
    return !musicId || voice.id!=musicId;
  }),voices.end());
  return before-voices.size();
}
