#include <Dolphin/types.h>
#include <JSystem/JSupport/JSUMemoryStream.hxx>

#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/MapObj/MapObjGeneral.hxx>
#include <SMS/MoveBG/Item.hxx>
#include <SMS/MoveBG/Shine.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"
#include "settings.hxx"

static void *collectItem(void *vtable) {
    TItem *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo &actorInfo = getRandomizerInfo(actor);
    actorInfo.mFromSurfaceDist     = 0;
    actorInfo.mShouldRandomize     = Randomizer::isRandomCollectibles();
    actorInfo.mIsItemObj           = true;
    actorInfo.mIsUnderwaterValid   = true;
    actorInfo.mIsWaterValid        = true;
    actorInfo.mIsGroundValid       = true;
    actorInfo.mShouldResizeUniform = true;
    actorInfo.mShouldResizeY       = false;
    actorInfo.mShouldResizeXZ      = false;
    actorInfo.mShouldRotateY       = true;
    actorInfo.mShouldRotateXZ      = false;
    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BEE34, 0, 0, 0), collectItem);

static void *collectCoin(void *vtable) {
    TItem *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo &actorInfo         = getRandomizerInfo(actor);
    actorInfo.mFromSurfaceDist     = 0;
    actorInfo.mShouldRandomize     = Randomizer::isRandomCollectibles();
    actorInfo.mIsItemObj           = true;
    actorInfo.mIsUnderwaterValid   = true;
    actorInfo.mIsWaterValid        = true;
    actorInfo.mIsGroundValid       = true;
    actorInfo.mShouldResizeUniform = true;
    actorInfo.mShouldResizeY       = true;
    actorInfo.mShouldResizeXZ      = true;
    actorInfo.mShouldRotateY       = true;
    actorInfo.mShouldRotateXZ      = false;

    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BE628, 0, 0, 0), collectCoin);

static void *collectCoinBlue(void *vtable) {
    TItem *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo &actorInfo         = getRandomizerInfo(actor);
    actorInfo.mFromSurfaceDist     = 0;
    actorInfo.mShouldRandomize     = Randomizer::isRandomCollectibles();
    actorInfo.mIsItemObj           = true;
    actorInfo.mIsUnderwaterValid   = true;
    actorInfo.mIsWaterValid        = true;
    actorInfo.mIsGroundValid       = true;
    actorInfo.mShouldResizeUniform = true;
    actorInfo.mShouldResizeY       = false;
    actorInfo.mShouldResizeXZ      = false;
    actorInfo.mShouldRotateY       = true;
    actorInfo.mShouldRotateXZ      = false;

    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BE0A8, 0, 0, 0), collectCoinBlue);

static void *collectCoinRed(void *vtable) {
    TItem *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo &actorInfo         = getRandomizerInfo(actor);
    actorInfo.mFromSurfaceDist     = 0;
    actorInfo.mShouldRandomize     = Randomizer::isRandomCollectibles();
    actorInfo.mIsItemObj           = true;
    actorInfo.mIsUnderwaterValid   = true;
    actorInfo.mIsWaterValid        = true;
    actorInfo.mIsGroundValid       = true;
    actorInfo.mShouldResizeUniform = true;
    actorInfo.mShouldResizeY       = false;
    actorInfo.mShouldResizeXZ      = false;
    actorInfo.mShouldRotateY       = true;
    actorInfo.mShouldRotateXZ      = false;

    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BE3B8, 0, 0, 0), collectCoinRed);

static TMapObjBase *collect1up(TMapObjBase *actor, const char *name) {
    __ct__11TMapObjBaseFPCc(actor, name);

    HitActorInfo &actorInfo         = getRandomizerInfo(actor);
    actorInfo.mFromSurfaceDist     = 0;
    actorInfo.mShouldRandomize     = Randomizer::isRandomCollectibles();
    actorInfo.mIsItemObj           = true;
    actorInfo.mIsUnderwaterValid   = true;
    actorInfo.mIsWaterValid        = true;
    actorInfo.mIsGroundValid       = true;
    actorInfo.mShouldResizeUniform = true;
    actorInfo.mShouldResizeY       = false;
    actorInfo.mShouldResizeXZ      = false;
    actorInfo.mShouldRotateY       = true;
    actorInfo.mShouldRotateXZ      = false;

    return actor;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801FE2B4, 0, 0, 0), collect1up);

static void *collectShine(void *vtable) {
    TShine *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo &actorInfo         = getRandomizerInfo(actor);
    actorInfo.mFromSurfaceDist     = 300;
    actorInfo.mShouldRandomize     = Randomizer::isRandomCollectibles();
    actorInfo.mIsItemObj           = true;
    actorInfo.mIsShineObj          = true;
    actorInfo.mIsUnderwaterValid   = false;
    actorInfo.mIsWaterValid        = false;
    actorInfo.mIsGroundValid       = true;
    actorInfo.mShouldResizeUniform = true;
    actorInfo.mShouldResizeY       = true;
    actorInfo.mShouldResizeXZ      = true;
    actorInfo.mShouldRotateY       = true;
    actorInfo.mShouldRotateXZ      = false;

    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BCB74, 0, 0, 0), collectShine);

static const char *makeCoinsVisible(TMapObjBase *actor) {
    if (Randomizer::isRandomCollectibles() &&
        strcmp(actor->mRegisterName, "invisible_coin") == 0) {
        actor->mRegisterName = "coin";
    }
    return actor->mRegisterName;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801B1AE8, 0, 0, 0), makeCoinsVisible);