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

static TMapObjBase *collectSandBombBase(TMapObjBase *actor, const char *name) {
    __ct__11TMapObjBaseFPCc(actor, name);

    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize = false;

    return actor;
}
//SMS_PATCH_BL(SMS_PORT_REGION(0x801D2644, 0, 0, 0), collectSandBombBase);

static TMapObjBase *collectSandBase(TMapObjBase *actor, const char *name) {
    __ct__11TMapObjBaseFPCc(actor, name);

    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize = false;

    return actor;
}
//SMS_PATCH_BL(SMS_PORT_REGION(0x801D349C, 0, 0, 0), collectSandBase);