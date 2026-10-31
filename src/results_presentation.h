#pragma once
#include <algorithm>
#include <cmath>
#include <vector>

inline std::vector<int> NativeResultOrder(unsigned participants, const int *scores) {
  std::vector<int> order;
  for (int i = 0; i < 4; ++i) if (participants & (1u << i)) order.push_back(i);
  std::stable_sort(order.begin(), order.end(), [&](int a, int b) { return scores[a] > scores[b]; });
  return order;
}
inline int NativeResultRank(const std::vector<int> &order, const int *scores, size_t row) {
  size_t first = row;
  while (first && scores[order[first - 1]] == scores[order[row]]) --first;
  return int(first) + 1;
}
inline float NativeResultRollDuration(size_t count) { return count > 1 ? 5.4f : count == 1 ? 2.8f : 0.f; }
inline float NativeResultCursor(size_t count, float elapsed) {
  if (count < 2 || elapsed >= NativeResultRollDuration(count)) return 0.f;
  float p = std::clamp(elapsed / NativeResultRollDuration(count), 0.f, 1.f);
  float eased = 1.f - std::pow(1.f - p, 3.f);
  float span = float(count - 1);
  float distance = span + 5.f * span * eased;
  float phase = std::fmod(distance, 2.f * span);
  return span - std::abs(phase - span);
}
inline float NativeResultScoreProgress(size_t count, float elapsed) {
  float p = std::clamp((elapsed - NativeResultRollDuration(count)) / 1.2f, 0.f, 1.f);
  return 1.f - (1.f - p) * (1.f - p);
}
inline int NativeResultDisplayedScore(int score, size_t count, float elapsed) {
  if (elapsed >= NativeResultRollDuration(count) + 1.2f) return score;
  return int(std::llround(double(score) * NativeResultScoreProgress(count, elapsed)));
}
inline bool NativeResultsCanContinue(size_t count, float elapsed) {
  return elapsed >= NativeResultRollDuration(count) + 1.6f;
}
