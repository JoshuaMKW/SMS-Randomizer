#pragma once

#include <Dolphin/types.h>

#include <SMS/System/Application.hxx>

void initGameSeed(TApplication *);
u32 getGameSeed();
u32 getColorSeed();

void srand32(u32 seed);
u32 rand32();
f32 randLerp();
bool tryChance(f32 percent);