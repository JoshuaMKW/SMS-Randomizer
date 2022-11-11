#include <Dolphin/types.h>

#include <SMS/rand.h>
#include <SMS/System/Application.hxx>

static u32 sGameSeed = 0;

void initGameSeed(TApplication *app) {
	u16 seed = rand();
    OSTick time = OSGetTick();

	sGameSeed = (seed * (time & 0xFFFF)) ^ time;
}

u32 getGameSeed() { return sGameSeed; }