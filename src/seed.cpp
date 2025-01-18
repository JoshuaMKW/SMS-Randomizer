#include <Dolphin/types.h>

#include <SMS/System/Application.hxx>
#include <SMS/rand.h>

#include "p_settings.hxx"
#include "seed.hxx"
#include "settings.hxx"

static u32 sSeed32    = 0;
static u64 sSeed64    = 0;
static u32 sColorSeed = 0;

extern Settings::IntSetting gGameSeedSetting;

static u32 xorshift32(u32 x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

static u64 xorshift64(u64 x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

#if USE_SIMPLE_DIST == false

static u32 make_uniform_32(u32 seed, u32 min, u32 max) {
    if (min >= max) {
        return min;
    }

    u32 range = max - min;
    u32 limit = 0xFFFF'FFFFUL / range * range;

    do {
        seed = xorshift32(seed);
    } while (seed >= limit);

    return (seed % range) + min;
}

static u64 make_uniform_64(u64 seed, u64 min, u64 max) {
    if (min >= max) {
        return min;
    }

    u64 range = max - min;
    u64 limit = 0xFFFF'FFFF'FFFF'FFFFULL / range * range;

    do {
        seed = xorshift64(seed);
    } while (seed >= limit);

    return (seed % range) + min;
}

#endif

namespace Randomizer {
    u32 diffuse_u32(u32 seed) {
        seed ^= seed >> 16;
        seed *= 0x85ebca6b;
        seed ^= seed >> 13;
        seed *= 0xc2b2ae35;
        seed ^= seed >> 16;
        return seed;
    }

    u64 diffuse_u64(u64 seed) {
        seed ^= seed >> 33;
        seed *= 0xff51afd7ed558ccd;
        seed ^= seed >> 33;
        seed *= 0xc4ceb9fe1a85ec53;
        seed ^= seed >> 33;
        return seed;
    }

    u32 uniform_dist_u32(u32 seed, u32 min, u32 max) {
#if USE_SIMPLE_DIST == true
        return xorshift32(seed) % (max - min) + min;
#else
        return make_uniform_32(seed, min, max);
#endif
    }

    u64 uniform_dist_u64(u64 seed, u64 min, u64 max) {
#if USE_SIMPLE_DIST == true
        return xorshift64(seed) % (max - min) + min;
#else
        return make_uniform_64(seed, min, max);
#endif
    }

    void srand32(u32 seed) {
        sSeed32 = seed;
#if USE_SMS_RAND == true
        srand(seed);
#endif
    }
    void srand64(u64 seed) {
        sSeed64 = seed;
#if USE_SMS_RAND == true
        srand(seed);
#endif
    }

    u32 rand32() {
#if USE_SMS_RAND == true
        sSeed32 = diffuse_u32(((u32)rand() << 16) | (u32)rand());
#else
        sSeed32 = uniform_dist_u32(sSeed32, 0, 0xFFFF'FFFFUL);
#endif
        return sSeed32;
    }

    u64 rand64() {
#if USE_SMS_RAND == true
        sSeed64 = diffuse_u64(((u64)rand() << 32) | ((u64)rand() << 48) | ((u64)rand() << 16) | (u64)rand());
#else
        sSeed64 = uniform_dist_u64(sSeed64, 0, 0xFFFF'FFFF'FFFF'FFFFULL);
#endif
        return sSeed64;
    }

    u32 rand32(u32 min, u32 max) {
#if USE_SMS_RAND == true
        sSeed32     = diffuse_u32(((u32)rand() << 16) | (u32)rand());
        u32 newSeed = sSeed32 % (max - min) + min;
#else
        u32 newSeed = uniform_dist_u32(sSeed32, min, max);
        sSeed32     = uniform_dist_u32(sSeed32, 0, 0xFFFF'FFFFUL);
#endif
        return newSeed;
    }

    u64 rand64(u64 min, u64 max) {
#if USE_SMS_RAND == true
        sSeed64     = diffuse_u64(((u64)rand() << 32) | ((u64)rand() << 48) | ((u64)rand() << 16) |
                                  (u64)rand());
        u64 newSeed = sSeed64 % (max - min) + min;
#else
        u64 newSeed = uniform_dist_u64(sSeed64, min, max);
        sSeed64     = uniform_dist_u64(sSeed64, 0, 0xFFFF'FFFF'FFFF'FFFFULL);
#endif
        return newSeed;
    }

    f32 randLerp32() { return static_cast<f32>(rand32()) / 4294967295.0f; }
    f64 randLerp64() { return static_cast<f64>(rand64()) / 18446744073709551615.0f; }

    bool tryChance32(f32 percent) { return randLerp32() < percent * 0.01f; }
    bool tryChance64(f64 percent) { return randLerp64() < percent * 0.01; }

    u32 levelScramble(u32 value, u32 uid, bool byEpisode) {
        if (byEpisode)
            return diffuse_u32(value ^
                               ((gpMarDirector->mAreaID << 8) | gpMarDirector->mEpisodeID) * uid);
        return diffuse_u32((value ^ (gpMarDirector->mAreaID << 8)) * uid);
    }
}  // namespace Randomizer

void gameSeedChanged(void *old, void *cur, Settings::SingleSetting::ValueKind kind) {
    const u32 gameSeed = *reinterpret_cast<u32 *>(cur);
    sColorSeed         = Randomizer::diffuse_u32(gameSeed);
}

static u32 mix_u16_u32(u16 _16, u32 _32) {
    u32 mixed = _32 ^ ((u32)_16 << 16);
    mixed     = (mixed >> 1) | (mixed << 31);
    mixed ^= _32 + (_16 * MAGIC_DIFFUSE);
    return mixed;
}

void initGameSeed(TApplication *app) {
    u16 seed    = rand();
    OSTick time = OSGetTick();

    int seed32 = (int)mix_u16_u32(seed, time);
    while (seed32 > gGameSeedSetting.getValueRange().mStop) {
        seed32 -= gGameSeedSetting.getValueRange().mStop;
    }

    while (seed32 < gGameSeedSetting.getValueRange().mStart) {
        seed32 += gGameSeedSetting.getValueRange().mStop;
    }

    gGameSeedSetting.setInt(seed32);
}

u32 getColorSeed() { return sColorSeed; }