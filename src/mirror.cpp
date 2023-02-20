#include <BetterSMS/module.hxx>
#include "settings.hxx"

// Mirror Mode exports
extern void SetMirrorModeActive(bool active);
extern bool GetMirrorModeActive();

void randomizeMirrorMode(TMarDirector *director) {
    if (!BetterSMS::isModuleRegistered("Mirror Mode") || !Randomizer::isRandomMirrorMode())
        return;

    u32 seed = Randomizer::getGameSeed();
    seed += director->mAreaID * 0x54321231;

    SetMirrorModeActive((seed & 1) == 0);
}