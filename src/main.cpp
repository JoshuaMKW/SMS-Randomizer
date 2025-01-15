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

#include "settings.hxx"
#include "p_settings.hxx"

extern void initActorInfoMap(TMarDirector *);
extern void initMapLoadStatus(TMarDirector *);
extern void initGameSeed(TApplication *);
extern void initDefaultSolver(TApplication *);
extern void randomizeMirrorMode(TMarDirector *director);
extern void setPlayerInitialHealth(TMario *, bool);
extern void emitHintEffectForHideObjs(TMarDirector *);

extern void initSettings(Settings::SettingsGroup &group);

extern void resetActorList(TMarDirector *director);
extern void resetExStageGlobals(TMarDirector *director);
extern void removeExStageStickyWalls(TMarDirector *director);

extern void recalculateObjectsWhenEulerChange(TMarDirector *director);

// Module definition

BetterSMS::ModuleInfo sModuleInfo{"Randomizer", 1, 0, &gSettingsGroup};

static void initModule() {
    // Register callbacks
    BetterSMS::Game::addBootCallback(initGameSeed);
    BetterSMS::Game::addBootCallback(initDefaultSolver);
    BetterSMS::Stage::addInitCallback(initActorInfoMap);
    BetterSMS::Stage::addInitCallback(initMapLoadStatus);
    BetterSMS::Stage::addInitCallback(randomizeMirrorMode);
    BetterSMS::Player::addInitCallback(setPlayerInitialHealth);
    BetterSMS::Stage::addUpdateCallback(emitHintEffectForHideObjs);

    BetterSMS::Stage::addInitCallback(resetActorList);
    BetterSMS::Stage::addInitCallback(resetExStageGlobals);
    BetterSMS::Stage::addUpdateCallback(removeExStageStickyWalls);

    // Register settings
    initSettings(gSettingsGroup);
    {
        auto &saveInfo        = gSettingsGroup.getSaveInfo();
        saveInfo.mSaveName    = "Sunshine Randomizer";
        saveInfo.mBlocks      = 1;
        saveInfo.mGameCode    = 'GMSR';
        saveInfo.mCompany     = 0x3031;  // '01'
        saveInfo.mBannerFmt   = CARD_BANNER_CI;
        saveInfo.mBannerImage = reinterpret_cast<const ResTIMG *>(gSaveBnr);
        saveInfo.mIconFmt     = CARD_ICON_CI;
        saveInfo.mIconSpeed   = CARD_SPEED_SLOW;
        saveInfo.mIconCount   = 2;
        saveInfo.mIconTable   = reinterpret_cast<const ResTIMG *>(gSaveIcon);
        saveInfo.mSaveGlobal  = false;
    }

    BetterSMS::registerModule(sModuleInfo);
}

// Definition block
KURIBO_MODULE_BEGIN("Randomizer", "JoshuaMK", "v1.0") {
    // Set the load and unload callbacks to our registration functions
    KURIBO_EXECUTE_ON_LOAD { initModule(); }
}
KURIBO_MODULE_END()
