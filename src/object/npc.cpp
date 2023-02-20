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

static TBaseNPC *collectNPC(TBaseNPC *actor, const char *name) {
    u32 id;
    SMS_FROM_GPR(29, id);

    __ct__11TSpineEnemyFPCc(actor, name);

    HitActorInfo &actorInfo         = getRandomizerInfo(actor);
    actorInfo.mFromSurfaceDist    = 0;
    if (id == 0x4000018 && gpMarDirector->mAreaID == 1 &&
        gpMarDirector->mEpisodeID == 1)
        actorInfo.mShouldRandomize = false;
    else
        actorInfo.mShouldRandomize = true;
    actorInfo.mIsItemObj           = false;
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
SMS_PATCH_BL(SMS_PORT_REGION(0x80208090, 0, 0, 0), collectNPC);