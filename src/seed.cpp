#include <Dolphin/types.h>

#include <SMS/System/Application.hxx>
#include <SMS/rand.h>

#include "p_settings.hxx"
#include "settings.hxx"

static u32 sSeed      = 0;
static u32 sColorSeed = 0;

extern Settings::IntSetting gGameSeedSetting;

namespace Randomizer {
    void srand32(u32 seed) { sSeed = seed; }

    u32 rand32() {
        sSeed = sSeed * 0x41C64E6D + 0x3039;
        return sSeed;
    }

    f32 randLerp() { return static_cast<f32>(rand32()) / 4294967295.0f; }

    bool tryChance(f32 percent) { return randLerp() < percent * 0.01; }

    u32 levelScramble(u32 value, u32 uid, bool byEpisode) {
        if (byEpisode)
            return (value ^ ((gpMarDirector->mAreaID << 8) | gpMarDirector->mEpisodeID)) * uid;
        return (value ^ (gpMarDirector->mAreaID << 8)) * uid;
    }
}  // namespace Randomizer

void gameSeedChanged(void *old, void *cur, Settings::SingleSetting::ValueKind kind) {
    const u32 gameSeed = *reinterpret_cast<u32 *>(cur);
    sColorSeed         = gameSeed * 0x41C64E6D + 0x3039;
}

void initGameSeed(TApplication *app) {
    u16 seed    = rand();
    OSTick time = OSGetTick();

    int seed32 = (seed * (time & 0xFFFF)) ^ time;
    while (seed32 > gGameSeedSetting.getValueRange().mStop) {
        seed32 -= gGameSeedSetting.getValueRange().mStop;
    }

    while (seed32 < gGameSeedSetting.getValueRange().mStart) {
        seed32 += gGameSeedSetting.getValueRange().mStop;
    }

    gGameSeedSetting.setInt(seed32);
}

u32 getColorSeed() { return sColorSeed; }