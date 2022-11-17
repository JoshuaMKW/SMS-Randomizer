#pragma once

#include <Dolphin/types.h>

#include <BetterSMS/settings.hxx>

using namespace BetterSMS;

class PrintUIntSetting : public Settings::IntSetting {
public:
    PrintUIntSetting() = delete;
    PrintUIntSetting(const char *name, void *valuePtr) : IntSetting(name, valuePtr) {}
    ~PrintUIntSetting() override {}

    void getValueStr(char *dst) const override { snprintf(dst, 11, "%lu", static_cast<u32>(getInt())); }
};

extern Settings::SettingsGroup gSettingsGroup;

extern PrintUIntSetting gGameSeedSetting;
extern Settings::SwitchSetting gRandomizeCollectiblesSetting;
extern Settings::SwitchSetting gRandomizeObjectsSetting;
extern Settings::SwitchSetting gRandomizeEnemiesSetting;
extern Settings::SwitchSetting gRandomizeWarpsSetting;
extern Settings::SwitchSetting gRandomizeScaleSetting;
extern Settings::SwitchSetting gRandomizeColorsSetting;
extern Settings::SwitchSetting gRandomizeMusicSetting;
extern Settings::SwitchSetting gRandomizeExStageSetting;
extern Settings::SwitchSetting gRandomizeHPDamageSetting;

extern const u8 gSaveBnr[];
extern const u8 gSaveIcon[];