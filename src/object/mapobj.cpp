#include <Dolphin/types.h>
#include <JSystem/JSupport/JSUMemoryStream.hxx>

#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/MapObj/MapObjGeneral.hxx>
#include <SMS/MoveBG/Item.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>
#include <SMS/rand.h>

#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"

static void *collectMapStaticObj(void *vtable) {
    TMapObjBase *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize = false;
    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80195DA0, 0, 0, 0), collectMapStaticObj);

static void *collectMapObjFlag(void *vtable) {
    TMapObjGeneral *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize = false;

    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801DBE14, 0, 0, 0), collectMapObjFlag);

static void *collectMapObjGrass(void *vtable) {
    THitActor *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize = false;
    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801E95B8, 0, 0, 0), collectMapObjGrass);

static void collectMapObjChangeStage(TMapObjBase *actor, JSUMemoryInputStream &in) {
    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize = false;
    load__11TMapObjBaseFR20JSUMemoryInputStream(actor, &in);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801C173C, 0, 0, 0), collectMapObjChangeStage);

static void collectMapObjStartDemoLoad(TItem *actor, JSUMemoryInputStream &in) {
    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize = false;
    load__11TMapObjBaseFR20JSUMemoryInputStream(actor, &in);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801C15E4, 0, 0, 0), collectMapObjStartDemoLoad);

static void *collectBellWatermill(void *vtable) {
    TMapObjGeneral *actor;
    SMS_FROM_GPR(31, actor);

    *(void **)actor = vtable;

    HitActorInfo *actorInfo     = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize = false;

    return vtable;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801C4B0C, 0, 0, 0), collectBellWatermill);