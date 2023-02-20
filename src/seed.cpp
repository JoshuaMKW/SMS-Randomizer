#include <Dolphin/types.h>

#include <SMS/rand.h>
#include <SMS/System/Application.hxx>

#include "settings.hxx"
#include "p_settings.hxx"

static u32 sSeed = 0;
static u32 sColorSeed = 0;

extern PrintUIntSetting gGameSeedSetting;

namespace Randomizer {
    void srand32(u32 seed) { sSeed = seed; }

    u32 rand32() {
        sSeed = sSeed * 0x41C64E6D + 0x3039;
        return sSeed;
    }

    f32 randLerp() { return static_cast<f32>(rand32()) / 4294967295.0f; }

    bool tryChance(f32 percent) { return randLerp() < percent * 0.01; }
}  // namespace Randomizer

void gameSeedChanged(void *old, void *cur, Settings::SingleSetting::ValueKind kind) {
    const u32 gameSeed = *reinterpret_cast<u32 *>(cur);
    sColorSeed         = gameSeed * 0x41C64E6D + 0x3039;
}

void initGameSeed(TApplication *app) {
	u16 seed = rand();
    OSTick time = OSGetTick();

	gGameSeedSetting.setInt((seed * (time & 0xFFFF)) ^ time);
}

u32 getColorSeed() { return sColorSeed; }