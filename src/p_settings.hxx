#pragma once

#include <Dolphin/types.h>

#include <BetterSMS/settings.hxx>

using namespace BetterSMS;

class PrintUIntSetting : public Settings::IntSetting {
public:
    PrintUIntSetting() = delete;
    PrintUIntSetting(const char *name, void *valuePtr) : IntSetting(name, valuePtr) {
        extern void gameSeedChanged(void *old, void *cur, Settings::SingleSetting::ValueKind kind);
        mValueChangedCB = gameSeedChanged;
    }
    ~PrintUIntSetting() override {}

    void getValueStr(char *dst) const override {
        snprintf(dst, 11, "%lu", static_cast<u32>(getInt()));
    }
};

extern Settings::SettingsGroup gSettingsGroup;
extern const u8 gSaveBnr[];
extern const u8 gSaveIcon[];