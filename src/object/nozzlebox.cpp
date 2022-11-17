#include <Dolphin/types.h>
#include <JSystem/JSupport/JSUMemoryStream.hxx>

#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/MapObj/MapObjGeneral.hxx>
#include <SMS/MoveBG/Item.hxx>
#include <SMS/MoveBG/NozzleBox.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"
#include "settings.hxx"

static void *collectNozzleBox(void *vtable) {
    TNozzleBox *actor;
    SMS_FROM_GPR(31, actor);

    HitActorInfo *actorInfo         = getRandomizerInfo(actor);
    actorInfo->mFromGroundHeight    = 0;
    actorInfo->mShouldRandomize     = gRandomizeCollectiblesSetting.getBool();
    actorInfo->mIsItemObj           = false;
    actorInfo->mIsUnderwaterValid   = false;
    actorInfo->mIsWaterValid        = false;
    actorInfo->mIsGroundValid       = true;
    actorInfo->mShouldResizeUniform = true;
    actorInfo->mShouldResizeY       = true;
    actorInfo->mShouldResizeXZ      = true;
    actorInfo->mShouldRotateY       = true;
    actorInfo->mShouldRotateXZ      = false;
    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801BB220, 0, 0, 0), collectNozzleBox);