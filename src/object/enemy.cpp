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

static TSpineEnemy *collectEnemy(TSpineEnemy *actor, const char *name) {
    __ct__10TLiveActorFPCc(actor, name);

    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mFromGroundHeight    = 0;
    actorInfo->mShouldRandomize     = gRandomizeEnemiesSetting.getBool();
    actorInfo->mIsItemObj           = true;
    actorInfo->mIsUnderwaterValid   = true;
    actorInfo->mIsWaterValid        = true;
    actorInfo->mIsGroundValid       = true;
    actorInfo->mShouldResizeUniform = true;
    actorInfo->mShouldResizeY       = false;
    actorInfo->mShouldResizeXZ      = false;
    actorInfo->mShouldRotateY       = true;
    actorInfo->mShouldRotateXZ      = false;
    return actor;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8003C628, 0, 0, 0), collectEnemy);