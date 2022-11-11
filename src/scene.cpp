#include <Dolphin/types.h>

#include <JSystem/JDrama/JDRActor.hxx>

#include <SMS/rand.h>
#include <SMS/macros.h>
#include <SMS/Strategic/Strategy.hxx>
#include <SMS/Map/Map.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/Map/MapCollisionStatic.hxx>
#include <SMS/System/MarDirector.hxx>

#include <BetterSMS/libs/constmath.hxx>
#include <BetterSMS/module.hxx>

#include "seed.hxx"
#include "settings.hxx"

static bool sIsMapLoaded = false;
static TDictI<THitActor *> sHitActorMap;
static u32 sStageSeed    = 0;

// -- MAP

void initMapLoadStatus(TMarDirector *director) {
    sIsMapLoaded = false;
    sStageSeed   = getGameSeed();
    sStageSeed  ^= ((director->mAreaID << 8) | director->mEpisodeID) * 0x5555;
    srand(sStageSeed);
}

static void setMapLoaded(TMapCollisionStatic *staticCol) {
    staticCol->setUp();
    sIsMapLoaded = true;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801896E0, 0, 0, 0), setMapLoaded);

static THitActor *collectHitActor(THitActor *actor) {
    sHitActorMap.set(reinterpret_cast<u32>(actor), actor);
    actor->mObjectType = 0;
    return actor;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80223F74, 0, 0, 0), collectHitActor);

// --

static f32 getFromGroundHeight(THitActor *obj) { return 100.0f; }

void randomizeObject(THitActor *obj, JSUMemoryInputStream &in) {
    obj->load(in);

    if (!sIsMapLoaded || !sHitActorMap.hasKey(reinterpret_cast<u32>(obj)))
        return;

    if (obj == gpMarioAddress)
        return;

    const f32 boundsX = gpMapCollisionData->mAreaSizeX;
    const f32 boundsZ = gpMapCollisionData->mAreaSizeZ;
    const f32 fromGroundHeight = getFromGroundHeight(obj);
    const u32 gameSeed = getGameSeed();

    {
        f32 trueObjX, trueObjY, trueObjZ;
        const TBGCheckData *out;

        do {
        // TODO: Clamp to circlular range
            trueObjX = lerp<f32>(-boundsX, boundsX, static_cast<f32>(rand() ^ rand()) / 65535.0f);
            trueObjZ = lerp<f32>(-boundsZ, boundsZ, static_cast<f32>(rand() ^ rand()) / 65535.0f);
            trueObjY = gpMapCollisionData->checkGround(trueObjX, 100000.0f, trueObjZ, 0, &out);
        } while (out->mCollisionType == 1536 || out->mCollisionType == 2048);

        obj->mPosition.x = trueObjX;
        obj->mPosition.y = trueObjY;
        obj->mPosition.z = trueObjZ;
    }

    obj->mRotation.y = lerp<f32>(0, 360, static_cast<f32>(rand()) / 65535.0f);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802A07A0, 0, 0, 0), randomizeObject);