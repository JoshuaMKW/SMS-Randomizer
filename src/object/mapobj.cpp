#include <System/MarNameRefGen.hxx>
#include <Dolphin/types.h>
#include <JSystem/JSupport/JSUMemoryStream.hxx>

#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/MapObj/MapObjGeneral.hxx>
#include <SMS/MoveBG/Item.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>
#include <SMS/rand.h>

#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"
#include "settings.hxx"

static bool isMarioRandomizable(const TMarDirector &director, const TMario *actor) {
    switch (director.mAreaID) {
    default:
        return !SMS_isExMap__Fv();
    }
}

#define STR_EQUAL(a, b) strcmp(a, b) == 0

static THitActor *collectGeneralActors(const TMarNameRefGen *gen, const char *name) {
    auto *actor            = reinterpret_cast<THitActor *>(gen->getNameRef(name));

    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize = true;

    if (STR_EQUAL(name, "MapObjSmoke")) {  // Delfino Plaza
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "MapObjFlag")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "MapObjWaterSpray")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "MapObjSoundGroup")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "MapObjFloatOnSea")) {
        actorInfo->mShouldRandomize   = true;
        actorInfo->mIsGroundValid     = false;
        actorInfo->mIsWaterValid      = true;
        actorInfo->mIsUnderwaterValid = false;
    } else if (STR_EQUAL(name, "MapStaticObj")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "WoodBox")) {
        actorInfo->mShouldRandomize     = true;
        actorInfo->mShouldResizeUniform = true;
    } else if (STR_EQUAL(name, "JellyGate")) {
        actorInfo->mShouldRandomize = gRandomizeObjectsSetting.getBool();
        actorInfo->mFromSurfaceDist = 5;
        actorInfo->mAdjustRotation  = {0, 0, 0};
        actorInfo->mShouldRotateXZ  = true;
        actorInfo->mShouldRotateY   = true;
        actorInfo->mIsWallValid     = true;
        actorInfo->mIsSurfaceBound  = true;
    } else if (STR_EQUAL(name, "Billboard")) {
        actorInfo->mShouldRandomize = gRandomizeObjectsSetting.getBool();
        actorInfo->mFromSurfaceDist = 0;
        actorInfo->mAdjustRotation  = {0, 90, 0};
        actorInfo->mShouldRotateXZ  = true;
        actorInfo->mShouldRotateY   = true;
        actorInfo->mIsWallValid     = true;
        actorInfo->mIsSurfaceBound  = true;
    } else if (STR_EQUAL(name, "MonumentShine")) {
        actorInfo->mShouldRandomize = true;
        actorInfo->mFromSurfaceDist = 900;
    } else if (STR_EQUAL(name, "BellDolpicTV")) {
        actorInfo->mShouldRandomize  = true;
        actorInfo->mFromSurfaceDist = 800;
    } else if (STR_EQUAL(name, "BellDolpicPolice")) {
        actorInfo->mShouldRandomize  = true;
        actorInfo->mFromSurfaceDist = 800;
    } else if (STR_EQUAL(name, "DemoCannon")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "DokanGate")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "DptCoronaFence")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "MareGate")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "WaterRecoverObj")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "WaterHitHideObj")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "WindmillRoof")) {  // Bianco Hills
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "SandEgg")) {  // Gelato Beach
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "SandBombBase")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "SandLeafBase")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "coral00")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "coral01")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "LeanMirror")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "TeethOfJuicer")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "bosshanachan0")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "Shining")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "Merrygoround")) {  // Pinna
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "ShellCup")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "AmiKing")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "Rocket")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "PinnaDoor")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "BalloonKoopaJr")) {  // TODO: Attempt to randomize
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "SirenaShop")) {  // Sirena Beach
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "ChestRevolve")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "Closet")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "PosterTeresa")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "Door")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "PanelRevolve")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "PanelBreak")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "PictureTeresa")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "casinoRoulette")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "ItemSlotDrum")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "SlotDrum")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "SirenaCasinoRoof")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "Donchou")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "SakuCasino")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "RailFence")) {  // Pianta Village
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "FireWanwan")) {  // TODO: Find a way to randomize
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "BossWanwan")) {  // TODO: Find a way to randomize
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "MareEventBumpyWall")) {  // Noki
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "MareEventPoint")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "MareFall")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "CoinFish")) {  // TODO: Find a way to randomize
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "SakuCasino")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "SakuCasino")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "FerrisLOD")) {  // General
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "WaterRecoverObj")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "WaterHitPictureHideObj")) {
        actorInfo->mShouldRandomize = gRandomizeCollectiblesSetting.getBool();
        actorInfo->mFromSurfaceDist = 0;
        actorInfo->mAdjustRotation  = {0, 0, 0};
        actorInfo->mShouldRotateXZ  = true;
        actorInfo->mShouldRotateY   = true;
        actorInfo->mIsWallValid     = true;
        actorInfo->mIsSurfaceBound  = true;
        actorInfo->mIsSprayableObj  = true;
    } else if (STR_EQUAL(name, "HideObjPictureTwin")) {
        actorInfo->mShouldRandomize = gRandomizeCollectiblesSetting.getBool();
        actorInfo->mFromSurfaceDist = 0;
        actorInfo->mAdjustRotation  = {0, 0, 0};
        actorInfo->mShouldRotateXZ  = true;
        actorInfo->mShouldRotateY   = true;
        actorInfo->mIsWallValid     = true;
        actorInfo->mIsSurfaceBound  = true;
        actorInfo->mIsSprayableObj  = true;
    } else if (STR_EQUAL(name, "SunModel")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "SunsetModel")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "GoalFlag")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "EMario")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "Manhole")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "AnimalMew")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "FishoidA")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "FishoidB")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "FishoidC")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "FishoidD")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "EffectPinnaFunsui")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "FruitsBoat")) {
        actorInfo->mShouldRandomize   = true;
        actorInfo->mIsGroundValid     = false;
        actorInfo->mIsWaterValid      = true;
        actorInfo->mIsUnderwaterValid = false;
        actorInfo->mFromSurfaceDist  = 0;
    } else if (STR_EQUAL(name, "FruitsBoatB")) {
        actorInfo->mShouldRandomize   = true;
        actorInfo->mIsGroundValid     = false;
        actorInfo->mIsWaterValid      = true;
        actorInfo->mIsUnderwaterValid = false;
        actorInfo->mFromSurfaceDist   = 0;
    } else if (STR_EQUAL(name, "MuddyBoat")) {
        actorInfo->mShouldRandomize = false;
    } else if (STR_EQUAL(name, "NozzleBox")) {
        actorInfo->mShouldRandomize     = true;
        actorInfo->mIsGroundValid       = true;
        actorInfo->mIsWaterValid        = false;
        actorInfo->mIsUnderwaterValid   = false;
        actorInfo->mShouldResizeUniform = false;
        actorInfo->mShouldResizeXZ      = false;
        actorInfo->mShouldResizeY       = false;
        actorInfo->mShouldRotateXZ      = false;
        actorInfo->mShouldRotateY       = true;
    } else if (STR_EQUAL(name, "Mario")) {
        actorInfo->mShouldRandomize     = isMarioRandomizable(*gpMarDirector, reinterpret_cast<TMario *>(actor));
        actorInfo->mIsGroundValid       = true;
        actorInfo->mIsWaterValid        = true;
        actorInfo->mIsUnderwaterValid   = true;
        actorInfo->mShouldResizeUniform = true;
        actorInfo->mShouldResizeXZ      = false;
        actorInfo->mShouldResizeY       = false;
        actorInfo->mShouldRotateXZ      = false;
        actorInfo->mShouldRotateY       = true;
    }

    return actor;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802FA628, 0, 0, 0), collectGeneralActors);

#undef STR_EQUAL

static void *collectMapStaticObj(void *vtable) {
    TMapObjBase *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize = false;
    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80195DA0, 0, 0, 0), collectMapStaticObj);

static void *collectMapObjFlag(void *vtable) {
    TMapObjGeneral *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize = false;

    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801DBE14, 0, 0, 0), collectMapObjFlag);

static void *collectMapObjGrass(void *vtable) {
    THitActor *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize = false;
    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801E95B8, 0, 0, 0), collectMapObjGrass);

//static void *collectMapObjWaterSpray(void *vtable) {
//    THitActor *actor;
//    SMS_FROM_GPR(31, actor);
//
//    *(void **)actor = vtable;
//
//    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
//    actorInfo->mShouldRandomize = false;
//    return vtable;
//}
//SMS_PATCH_BL(SMS_PORT_REGION(0x801C10BC, 0, 0, 0), collectMapObjGrass);
//
//static void collectMapObjSmokeLoad(TMapObjBase *actor, JSUMemoryInputStream &in) {
//    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
//    actorInfo->mShouldRandomize = false;
//    load__12TMapObjSmokeFR20JSUMemoryInputStream(actor, &in);
//}
//SMS_PATCH_BL(SMS_PORT_REGION(0x801e71d0, 0, 0, 0), collectMapObjSmokeLoad);

static void collectMapObjChangeStage(TMapObjBase *actor, JSUMemoryInputStream &in) {
    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize = false;
    load__11TMapObjBaseFR20JSUMemoryInputStream(actor, &in);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801C173C, 0, 0, 0), collectMapObjChangeStage);

static void collectMapObjStartDemoLoad(TItem *actor, JSUMemoryInputStream &in) {
    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize = false;
    load__11TMapObjBaseFR20JSUMemoryInputStream(actor, &in);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801C15E4, 0, 0, 0), collectMapObjStartDemoLoad);

static void *collectBellWatermill(void *vtable) {
    TMapObjGeneral *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize = false;

    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801C4B0C, 0, 0, 0), collectBellWatermill);