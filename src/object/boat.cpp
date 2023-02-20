#include <Dolphin/types.h>
#include <JSystem/JSupport/JSUMemoryStream.hxx>

#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/MapObj/MapObjGeneral.hxx>
#include <SMS/MoveBG/Item.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>

#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"

static void *collectMudBoat(void *vtable) {
    THitActor *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo &actorInfo     = getRandomizerInfo(actor);
    actorInfo.mShouldRandomize = false;
    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801D7E1C, 0, 0, 0), collectMudBoat);