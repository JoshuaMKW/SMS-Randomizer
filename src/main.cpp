#include <Dolphin/types.h>
#include <Dolphin/CARD.h>
#include <Dolphin/math.h>
#include <Dolphin/OS.h>
#include <Dolphin/string.h>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>

#include <SMS/System/Application.hxx>

#include <BetterSMS/game.hxx>
#include <BetterSMS/module.hxx>
#include <BetterSMS/player.hxx>
#include <BetterSMS/stage.hxx>
#include <BetterSMS/loading.hxx>
#include <BetterSMS/settings.hxx>
#include <BetterSMS/icons.hxx>

#include "settings.hxx"

extern void initActorInfoMap(TMarDirector *);
extern void initMapLoadStatus(TMarDirector *);
extern void initGameSeed(TApplication *);
extern void setPlayerInitialHealth(TMario *player, bool isMario);

// Module definition

static void initModule() {
    OSReport("Initializing Randomizer...\n");

    // Register callbacks
    BetterSMS::Game::registerOnBootCallback("Randomizer_InitGameSeed", initGameSeed);
    BetterSMS::Stage::registerInitCallback("Randomizer_MapLoadActorInit", initActorInfoMap);
    BetterSMS::Stage::registerInitCallback("Randomizer_MapLoadStatusInit", initMapLoadStatus);
    BetterSMS::Player::registerInitProcess("Randomizer_PlayerHealthInit", setPlayerInitialHealth);

    // Register settings
    gSettingsGroup.addSetting(&gGameSeedSetting);
    gSettingsGroup.addSetting(&gRandomizeCollectiblesSetting);
    gSettingsGroup.addSetting(&gRandomizeObjectsSetting);
    gSettingsGroup.addSetting(&gRandomizeEnemiesSetting);
    gSettingsGroup.addSetting(&gRandomizeWarpsSetting);
    gSettingsGroup.addSetting(&gRandomizeScaleSetting);
    gSettingsGroup.addSetting(&gRandomizeColorsSetting);
    gSettingsGroup.addSetting(&gRandomizeMusicSetting);
    gSettingsGroup.addSetting(&gRandomizeExStageSetting);
    gSettingsGroup.addSetting(&gRandomizeHPDamageSetting);
    {
        auto &saveInfo        = gSettingsGroup.getSaveInfo();
        saveInfo.mSaveName    = gSettingsGroup.getName();
        saveInfo.mBlocks      = 1;
        saveInfo.mGameCode    = 'GMSB';
        saveInfo.mCompany     = 0x3031;  // '01'
        saveInfo.mBannerFmt   = CARD_BANNER_CI;
        saveInfo.mBannerImage = GetResourceTextureHeader(gSaveBnr);
        saveInfo.mIconFmt     = CARD_ICON_CI;
        saveInfo.mIconSpeed   = CARD_SPEED_SLOW;
        saveInfo.mIconCount   = 2;
        saveInfo.mIconTable   = GetResourceTextureHeader(gSaveIcon);
        saveInfo.mSaveGlobal  = false;
    }
    BetterSMS::Settings::registerGroup("Randomizer", &gSettingsGroup);
}

static void deinitModule() {
    OSReport("Deinitializing Module...\n");

    // Cleanup callbacks
    BetterSMS::Stage::deregisterInitCallback("OurModule_StageInitCallBack");
    BetterSMS::Stage::deregisterUpdateCallback("OurModule_StageUpdateCallBack");
    BetterSMS::Stage::deregisterDraw2DCallback("OurModule_StageDrawCallBack");
}

// Definition block
KURIBO_MODULE_BEGIN("OurModule", "JoshuaMK", "v1.0") {
    // Set the load and unload callbacks to our registration functions
    KURIBO_EXECUTE_ON_LOAD { initModule(); }
    KURIBO_EXECUTE_ON_UNLOAD { deinitModule(); }
}
KURIBO_MODULE_END()
