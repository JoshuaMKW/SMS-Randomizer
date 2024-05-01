#pragma once

#include <Dolphin/types.h>

#include <SMS/System/Application.hxx>

namespace Randomizer {
	void srand32(u32 seed);
	u32 rand32();
	f32 randLerp();
	bool tryChance(f32 percent);
    u32 levelScramble(u32 value, u32 uid, bool byEpisode);
}
