#include <Dolphin/types.h>
#include <JSystem/JSupport/JSUMemoryStream.hxx>

#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/MapObj/MapObjGeneral.hxx>
#include <SMS/MapObj/MapObjRail.hxx>
#include <SMS/MoveBG/Item.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"
#include "settings.hxx"

static THitActor *collectHitActor(THitActor *actor) {
    HitActorInfo &actorInfo        = getRandomizerInfo(actor);
    actorInfo.mFromSurfaceDist     = 0;
    actorInfo.mShouldRandomize     = Randomizer::isRandomObjects();
    actorInfo.mIsUnderwaterValid   = false;
    actorInfo.mIsWaterValid        = false;
    actorInfo.mIsGroundValid       = true;
    actorInfo.mShouldResizeUniform = true;
    actorInfo.mShouldResizeY       = true;
    actorInfo.mShouldResizeXZ      = true;
    actorInfo.mShouldRotateY       = true;
    actorInfo.mShouldRotateXZ      = false;

    actor->mObjectType = 0;
    return actor;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80223F74, 0, 0, 0), collectHitActor);

static TBGCheckData *collectLiveActor() {
    TLiveActor *actor;
    SMS_FROM_GPR(31, actor);

    HitActorInfo &actorInfo        = getRandomizerInfo(actor);
    actorInfo.mFromSurfaceDist     = 0;
    actorInfo.mShouldRandomize     = Randomizer::isRandomObjects();
    actorInfo.mIsLiveActor         = true;
    actorInfo.mIsUnderwaterValid   = false;
    actorInfo.mIsWaterValid        = false;
    actorInfo.mIsGroundValid       = true;
    actorInfo.mShouldResizeUniform = true;
    actorInfo.mShouldResizeY       = true;
    actorInfo.mShouldResizeXZ      = true;
    actorInfo.mShouldRotateY       = true;
    actorInfo.mShouldRotateXZ      = false;

    actor->mObjectType = 0;
    return &gpMapCollisionData->mIllegalCheckData;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80218F64, 0, 0, 0), collectLiveActor);

static TMapObjBase *collectMapObjBase(TMapObjBase *actor, const char *name) {
    __ct__10TLiveActorFPCc(actor, name);

    HitActorInfo &actorInfo        = getRandomizerInfo(actor);
    actorInfo.mFromSurfaceDist     = 0;
    actorInfo.mShouldRandomize     = Randomizer::isRandomObjects();
    actorInfo.mIsBaseObj           = true;
    actorInfo.mIsUnderwaterValid   = false;
    actorInfo.mIsWaterValid        = false;
    actorInfo.mIsGroundValid       = true;
    actorInfo.mShouldResizeUniform = true;
    actorInfo.mShouldResizeY       = true;
    actorInfo.mShouldResizeXZ      = true;
    actorInfo.mShouldRotateY       = true;
    actorInfo.mShouldRotateXZ      = false;

    return actor;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801AF6BC, 0, 0, 0), collectMapObjBase);

static TMapObjGeneral *collectMapObjGeneral(TMapObjGeneral *actor) {
    HitActorInfo &actorInfo        = getRandomizerInfo(actor);
    actorInfo.mFromSurfaceDist     = 0;
    actorInfo.mShouldRandomize     = Randomizer::isRandomObjects();
    actorInfo.mIsBaseObj           = true;
    actorInfo.mIsUnderwaterValid   = false;
    actorInfo.mIsWaterValid        = false;
    actorInfo.mIsGroundValid       = true;
    actorInfo.mShouldResizeUniform = true;
    actorInfo.mShouldResizeY       = true;
    actorInfo.mShouldResizeXZ      = true;
    actorInfo.mShouldRotateY       = true;
    actorInfo.mShouldRotateXZ      = false;

    actor->_04 = 0.0f;
    return actor;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801B2FD8, 0, 0, 0), collectMapObjGeneral);

static TRailMapObj *collectMapObjRail(TRailMapObj *actor, const char *name) {
    __ct__11TMapObjBaseFPCc(actor, name);

    HitActorInfo &actorInfo        = getRandomizerInfo(actor);
    actorInfo.mFromSurfaceDist     = 0;
    actorInfo.mShouldRandomize     = Randomizer::isRandomObjects();
    actorInfo.mIsRailObj           = true;
    actorInfo.mIsUnderwaterValid   = false;
    actorInfo.mIsWaterValid        = false;
    actorInfo.mIsGroundValid       = true;
    actorInfo.mShouldResizeUniform = true;
    actorInfo.mShouldResizeY       = true;
    actorInfo.mShouldResizeXZ      = true;
    actorInfo.mShouldRotateY       = true;
    actorInfo.mShouldRotateXZ      = false;

    return actor;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801f19e0, 0, 0, 0), collectMapObjRail);
SMS_PATCH_BL(SMS_PORT_REGION(0x801f0780, 0, 0, 0), collectMapObjRail);