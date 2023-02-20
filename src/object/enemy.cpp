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

    HitActorInfo &actorInfo     = getRandomizerInfo(actor);
    actorInfo.mFromSurfaceDist    = 0;
    actorInfo.mShouldRandomize     = Randomizer::isRandomEnemies();
    actorInfo.mIsItemObj           = true;
    actorInfo.mIsUnderwaterValid   = false;
    actorInfo.mIsWaterValid        = false;
    actorInfo.mIsGroundValid       = true;
    actorInfo.mShouldResizeUniform = true;
    actorInfo.mShouldResizeY       = false;
    actorInfo.mShouldResizeXZ      = false;
    actorInfo.mShouldRotateY       = true;
    actorInfo.mShouldRotateXZ      = false;
    return actor;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8003C628, 0, 0, 0), collectEnemy);

static void *collectEMario(void *vtable) {
    TMario *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo &actorInfo         = getRandomizerInfo(actor);
    actorInfo.mFromSurfaceDist    = 0;
    actorInfo.mShouldRandomize     = false;
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
SMS_PATCH_BL(SMS_PORT_REGION(0x80039BD0, 0, 0, 0), collectEMario);

static TSpineEnemy *collectWireTrap(TSpineEnemy *actor, const char *name) {
    __ct__11TSpineEnemyFPCc(actor, name);

    HitActorInfo &actorInfo         = getRandomizerInfo(actor);
    actorInfo.mFromSurfaceDist     = 0;
    actorInfo.mShouldRandomize     = false;
    actorInfo.mIsItemObj           = true;
    actorInfo.mIsUnderwaterValid   = false;
    actorInfo.mIsWaterValid        = false;
    actorInfo.mIsGroundValid       = true;
    actorInfo.mShouldResizeUniform = true;
    actorInfo.mShouldResizeY       = false;
    actorInfo.mShouldResizeXZ      = false;
    actorInfo.mShouldRotateY       = true;
    actorInfo.mShouldRotateXZ      = false;
    return actor;
}
//SMS_PATCH_BL(SMS_PORT_REGION(0x8010A258, 0, 0, 0), collectWireTrap);