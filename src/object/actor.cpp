#include <Dolphin/types.h>
#include <JSystem/JSupport/JSUMemoryStream.hxx>

#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/MapObj/MapObjGeneral.hxx>
#include <SMS/MoveBG/Item.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"
#include "settings.hxx"

static THitActor *collectHitActor(THitActor *actor) {
    HitActorInfo *actorInfo         = getRandomizerInfo(actor);
    actorInfo->mFromGroundHeight    = 0;
    actorInfo->mShouldRandomize     = gRandomizeObjectsSetting.getBool();
    actorInfo->mIsItemObj           = false;
    actorInfo->mIsUnderwaterValid   = false;
    actorInfo->mIsWaterValid        = false;
    actorInfo->mIsGroundValid       = true;
    actorInfo->mShouldResizeUniform = true;
    actorInfo->mShouldResizeY       = true;
    actorInfo->mShouldResizeXZ      = true;
    actorInfo->mShouldRotateY       = true;
    actorInfo->mShouldRotateXZ      = false;

    actor->mObjectType = 0;
    return actor;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80223F74, 0, 0, 0), collectHitActor);

static TMapObjGeneral *collectMapObjGeneral(TMapObjGeneral *actor) {
    HitActorInfo *actorInfo = getRandomizerInfo(actor);
    actorInfo->mFromGroundHeight    = 0;
    actorInfo->mShouldRandomize     = gRandomizeObjectsSetting.getBool();
    actorInfo->mIsItemObj           = false;
    actorInfo->mIsUnderwaterValid   = false;
    actorInfo->mIsWaterValid        = false;
    actorInfo->mIsGroundValid       = true;
    actorInfo->mShouldResizeUniform = true;
    actorInfo->mShouldResizeY       = true;
    actorInfo->mShouldResizeXZ      = true;
    actorInfo->mShouldRotateY       = true;
    actorInfo->mShouldRotateXZ      = false;

    actor->_04 = 0.0f;
    return actor;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801B2FD8, 0, 0, 0), collectMapObjGeneral);