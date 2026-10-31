#pragma once
#include <cstdint>
// One explicit generator on every phone; never depend on the system libc RNG.
inline uint32_t dbxRandomState=1;
inline void NativeGameSeed(uint32_t seed){dbxRandomState=seed?seed:1;}
inline int NativeGameRandom(){uint32_t x=dbxRandomState;x^=x<<13;x^=x>>17;x^=x<<5;dbxRandomState=x;return int(x&0x7fffffffu);}
