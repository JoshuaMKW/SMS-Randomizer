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
#include <BetterSMS/stage.hxx>
#include <BetterSMS/loading.hxx>
#include <BetterSMS/settings.hxx>
#include <BetterSMS/icons.hxx>

extern void initMapLoadStatus(TMarDirector *);

// Module definition

static void initModule() {
    OSReport("Initializing Randomizer...\n");

    // Register callbacks
    BetterSMS::Stage::registerInitCallback("Randomizer_MapLoadStatusInit", initMapLoadStatus);

    //// Register settings
    //sXSpeedSetting.setValueRange({-10, 10, 1});
    //sYSpeedSetting.setValueRange({-10, 10, 1});
    //sSettingsGroup.addSetting(&sXSpeedSetting);
    //sSettingsGroup.addSetting(&sYSpeedSetting);
    //{
    //    auto &saveInfo        = sSettingsGroup.getSaveInfo();
    //    saveInfo.mSaveName    = sSettingsGroup.getName();
    //    saveInfo.mBlocks      = 1;
    //    saveInfo.mGameCode    = 'GMSB';
    //    saveInfo.mCompany     = 0x3031;  // '01'
    //    saveInfo.mBannerFmt   = CARD_BANNER_CI;
    //    saveInfo.mBannerImage = GetResourceTextureHeader(sSaveBnr);
    //    saveInfo.mIconFmt     = CARD_ICON_CI;
    //    saveInfo.mIconSpeed   = CARD_SPEED_SLOW;
    //    saveInfo.mIconCount   = 2;
    //    saveInfo.mIconTable   = GetResourceTextureHeader(sSaveIcon);
    //    saveInfo.mSaveGlobal  = true;
    //}
    //BetterSMS::Settings::registerGroup("Demo Module", &sSettingsGroup);
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
