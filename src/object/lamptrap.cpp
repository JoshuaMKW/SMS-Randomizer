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

static void *collectLampTraps(void *vtable) {
    THitActor *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo *actorInfo         = getRandomizerInfo(actor);
    actorInfo->mFromSurfaceDist    = -2000;
    actorInfo->mShouldRandomize     = gRandomizeObjectsSetting.getBool();
    actorInfo->mIsItemObj           = false;
    actorInfo->mIsUnderwaterValid   = true;
    actorInfo->mIsWaterValid        = true;
    actorInfo->mIsGroundValid       = true;
    actorInfo->mShouldResizeUniform = false;
    actorInfo->mShouldResizeY       = false;
    actorInfo->mShouldResizeXZ      = false;
    actorInfo->mShouldRotateY       = true;
    actorInfo->mShouldRotateXZ      = false;
    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802016A0, 0, 0, 0), collectLampTraps);
SMS_PATCH_BL(SMS_PORT_REGION(0x80201E34, 0, 0, 0), collectLampTraps);