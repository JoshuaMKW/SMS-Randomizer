#pragma once

#include <Dolphin/types.h>

#include <BetterSMS/settings.hxx>
#include <BetterSMS/module.hxx>

using namespace BetterSMS;

class RandomWarpSetting : public Settings::IntSetting {
public:
    enum State { OFF, LOCAL, GLOBAL };

    RandomWarpSetting(const char *name) : IntSetting(name, &RandomWarpSetting::sStateValue) {
        mValueRange = {0, 2, 1};
    }
    ~RandomWarpSetting() override {}

    void getValueName(char *dst) const override {
        switch (getInt()) {
        default:
        case State::OFF:
            strncpy(dst, "Off", 4);
            break;
        case State::LOCAL:
            strncpy(dst, "Local", 6);
            break;
        case State::GLOBAL:
            strncpy(dst, "Global", 7);
            break;
        }
    }

private:
    static int sStateValue;
};

class RandomMirrorModeSetting : public Settings::SwitchSetting {
public:
    RandomMirrorModeSetting(const char *name)
        : SwitchSetting(name, &RandomMirrorModeSetting::sStateValue) {}

    bool isUnlocked() const override { return BetterSMS::isModuleRegistered("Mirror Mode"); }

private:
    static bool sStateValue;
};

namespace Randomizer {
    u32 getGameSeed();
    bool isRandomCollectibles();
    bool isRandomObjects();
    bool isRandomEnemies();
    bool isRandomScale();
    bool isRandomColors();
    bool isRandomMusic();
    bool isRandomExStage();
    bool isRandomHPDamage();
    bool isRandomMirrorMode();
    RandomWarpSetting::State getRandomWarpsState();
}