#pragma once

#include <Dolphin/types.h>

#include <SMS/System/Application.hxx>

#define MAGIC_DIFFUSE   0x5BD1E995
#define USE_SMS_RAND    false
#define USE_SIMPLE_DIST false

#if USE_SMS_RAND == true
#undef USE_SIMPLE_DIST
#define USE_SIMPLE_DIST true
#endif

namespace Randomizer {
    u32 diffuse_u32(u32 seed);
    u64 diffuse_u64(u64 seed);

    u32 uniform_dist_u32(u32 seed, u32 min, u32 max);
    u64 uniform_dist_u64(u64 seed, u64 min, u64 max);

    void srand32(u32 seed);
    void srand64(u64 seed);
    u32 rand32();
    u64 rand64();
    u32 rand32(u32 min, u32 max);
    u64 rand64(u64 min, u64 max);

    f32 randLerp32();
    f64 randLerp64();

    bool tryChance32(f32 percent);
    bool tryChance64(f64 percent);

    u32 levelScramble(u32 value, u32 uid, bool byEpisode);

}  // namespace Randomizer
