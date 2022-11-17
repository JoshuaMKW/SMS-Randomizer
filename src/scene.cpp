#include <Dolphin/types.h>

#include <JSystem/JDrama/JDRActor.hxx>
#include <JSystem/JDrama/JDRNameRefGen.hxx>

#include <SMS/rand.h>
#include <SMS/raw_fn.hxx>
#include <SMS/macros.h>
#include <SMS/Strategic/Strategy.hxx>
#include <SMS/Map/Map.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/Map/MapCollisionStatic.hxx>
#include <SMS/System/MarDirector.hxx>

#include <BetterSMS/libs/constmath.hxx>
#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"
#include "seed.hxx"
#include "settings.hxx"


bool sIsMapLoaded;
u32 sStageSeed;

bool isContextRandomizable(TMarDirector *director) {
    if (SMS_isExMap__Fv() && !gRandomizeExStageSetting.getBool())
        return false;
    switch (director->mAreaID) {
    case 15:  // File Select
        if (director->mEpisodeID == 0)
            return false;
        return true;
    case 33:  // Sand Bird
        if (director->mEpisodeID == 0)
            return false;
        return true;
    case 60:
        if (director->mEpisodeID == 0)
            return false;
        return true;
    default:
        return true;
    }
}

bool isContextAllowed(TMarDirector *director, f32 x, f32 y, f32 z, const HitActorInfo *actorInfo,
                      const TBGCheckData *floor) {
    // Filter out warp collision
    if ((floor->mCollisionType & 0x3FF) == 768)
        return false;

    switch (director->mAreaID) {
    case 0: {  // Airstrip
        // Prevent invisible room spawns
        if (y > 5000.0f)
            return false;

        // Prevent OOB spawns
        const TVec3f center {1000.0f, 0.0f, -2500.0f};
        const TVec3f diff { x, 0.0f, z };
        return PSVECDistance(diff, center) < 16000.0f;
    }
    case 2: {  // Bianco Hills
        // Prevent OOB slope spawns
        if (y < 1700.0f && !floor->isWaterSurface())
            return false;

        // This bounding check fixes stuff spawning OOB near hillside
        if (z > -7000.0f && z < 2200.0f) {
            if (x > 16000.0f && y > 3000.0f) {
                return false;
            }
        }

        // This bounding check fixes stuff spawning OOB near brick walls
        if (x > 10000.0f && y > 3800.0f && z > 7800.0f) {
            return false;
        }

        return true;
    }
    case 3: {  // Ricco Harbor
        // Prevent rising water bad spawns
        if (y < 0.0f && !actorInfo->mIsWaterValid)
            return false;
        return true;
    }
    case 5: {  // Pinna Beach
        // Prevent behind door bad spawn
        if (z < 2200.0f) {
            if (x > -5200.0f && x < -1800.0f) {
                return false;
            }
        }

        // Prevent left ocean bad spawn
        if (z < 0.0f && x < -6900.0f)
            return false;

        // Prevent right ocean bad spawn
        if (z > 8600.0f || x > 17000.0f)
            return false;
    }
    case 6: {  // Sirena Beach
        // Prevent ocean bad spawn
        if (z > 23000.0f)
            return false;
        if (x < -8000.0f || x > 8600.0f)
            return false;
        return true;
    }
    case 8: {  // Pianta Village
        // Prevent cheap trunk spawns
        if (y < -650.0f && (floor->mCollisionType & 0x3FF) == 1)
            return false;
        return true;
    }
    case 9: {  // Noki Bay
        // Prevent high ground OOB spawn
        if (x > -300.0f && y > 12000.0f && z > 1500.0f)
            return false;
        return true;
    }
    case 13: {  // Pinna Park
        // Prevent behind ferris wheel bad spawn
        if (x > 10000.0f && z > -7500.0f)
            return false;
        return true;
    }
    case 30: {  // Blooper surfing secret
        // Prevent OOB floor spawns
        if (y < -1000.0f)
            return false;
        return true;
    }
    case 52: {  // Corona mountain
        // Prevent OOB water spawns
        if (x < -2500.0f || x > 2500.0f)
            return false;
        return true;
    }
    default:
        return true;
    }
}

static size_t collectFloorsAtXZ(TMapCollisionData *collision, f32 x, f32 z, bool groundValid, bool waterValid, size_t capacity, const TBGCheckData **out, f32 *yOut) {
    size_t found = 0;

    f32 checkY = 100000.0f;
    while (found < capacity) {
        const TBGCheckData *tmpOut;
        const f32 groundY = collision->checkGround(x, checkY, z, 0, &tmpOut);

        checkY = groundY - 100.0f;

        if (tmpOut->mCollisionType == 1536 || tmpOut->mCollisionType == 2048)
            return found;

        if (!waterValid && (tmpOut->isWaterSurface() || tmpOut->mCollisionType == 267 ||
                            tmpOut->mCollisionType == 33035))
            return found;

        if (!groundValid && !(tmpOut->isWaterSurface() || tmpOut->mCollisionType == 267 ||
            tmpOut->mCollisionType == 33035))
            return found;

        out[found] = tmpOut;
        yOut[found] = groundY;
        found += 1;
    }

    return found;
}

static f32 solveWaterY(TMarDirector *director, f32 x, f32 y, f32 z, const HitActorInfo *actorInfo,
                       const TBGCheckData *water) {
    switch (director->mAreaID) {
    case 52: {  // Corona mountain
        if (actorInfo->mIsItemObj)
            return y + actorInfo->mFromGroundHeight + 400.0f;  // Adjust to make it not touch lava

        if (actorInfo->mIsUnderwaterValid && tryChance(70.0f)) {
            const f32 belowGroundY =
                gpMapCollisionData->checkGround(x, y - 10.0f, z, 0, &water);
            return lerp<f32>(belowGroundY + actorInfo->mFromGroundHeight, y - 10.0f,
                                randLerp());
        }

        return y + actorInfo->mFromGroundHeight;
    }
    default: {
        if (actorInfo->mIsUnderwaterValid && tryChance(70.0f)) {
            const f32 belowGroundY = gpMapCollisionData->checkGround(x, y - 10.0f, z, 0, &water);
            return lerp<f32>(belowGroundY + actorInfo->mFromGroundHeight, y - 10.0f, randLerp());
        }
        return y + actorInfo->mFromGroundHeight;
    }
    }
}

static f32 solveGroundY(TMarDirector *director, f32 x, f32 y, f32 z, const HitActorInfo *actorInfo,
                       const TBGCheckData *water) {
    switch (director->mAreaID) {
    default: {
        return y + actorInfo->mFromGroundHeight;
    }
    }
}

// -- MAP

void initMapLoadStatus(TMarDirector *director) {
    sIsMapLoaded = false;
    sStageSeed   = getGameSeed();
    sStageSeed  ^= ((director->mAreaID << 8) | director->mEpisodeID) * 0x5555;
    srand32(sStageSeed);
}

static void setMapLoaded(TMapCollisionStatic *staticCol) {
    staticCol->setUp();
    sIsMapLoaded = true;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801896E0, 0, 0, 0), setMapLoaded);


static TMapObjGeneral *collectWireTrap() {
    TMapObjGeneral *actor;
    SMS_FROM_GPR(31, actor);

    HitActorInfo *actorInfo      = getRandomizerInfo(actor);
    actorInfo->mShouldRandomize  = false;
    return actor;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8010A274, 0, 0, 0), collectWireTrap);

// --

// riccoSeaPollutionS2
// mareSeaPollutionS12

static f32 getFromGroundHeight(THitActor *actor) {
    HitActorInfo *actorInfo = getRandomizerInfo(actor);
    if (actorInfo->mIsItemObj && tryChance(5.0f)) {
        return actorInfo->mFromGroundHeight + 3000.0f;
    }
    return actorInfo->mFromGroundHeight;
}

static u32 randomizeObject() {
    THitActor *actor;
    SMS_FROM_GPR(31, actor);

    actor->mRootRef = JDrama::TNameRefGen::getInstance()->getRootNameRef();

    if (!isContextRandomizable(gpMarDirector))
        return 0;

    if (!sIsMapLoaded)
        return 0;

    if (actor == gpMarioAddress)
        return 0;

    HitActorInfo *actorInfo = getRandomizerInfo(actor);
    if (!actorInfo->mShouldRandomize)
        return 0;

    constexpr size_t MaxFloors = 8;
    constexpr size_t MaxTries = 100;

    const f32 boundsX          = gpMapCollisionData->mAreaSizeX;
    const f32 boundsZ          = gpMapCollisionData->mAreaSizeZ;
    const f32 fromGroundHeight = getFromGroundHeight(actor);
    const u32 gameSeed         = getGameSeed();

    const bool isUnderwaterValid     = actorInfo->mIsUnderwaterValid;
    const bool isWaterValid     = actorInfo->mIsWaterValid;
    const bool isGroundValid   = actorInfo->mIsGroundValid;

    const bool isCoronaMountain = gpMarDirector->mAreaID == 52;

    size_t tries = 0;
    u32 key      = (actor->mKeyCode * 0x41C64E6D + 0x3039) ^ gameSeed;
    srand32(key);
    {
        f32 trueObjX, trueObjY, trueObjZ;

        const TBGCheckData *out[MaxFloors];
        f32 groundY[MaxFloors];

        do {
            if (actorInfo->mIsWallBound) {
                
            } else {
                f32 rX     = boundsX * sqrtf(randLerp());
                f32 thetaX = sqrtf(randLerp()) * 2 * M_PI;

                f32 rZ     = boundsZ * sqrtf(randLerp());
                f32 thetaZ = sqrtf(randLerp()) * 2 * M_PI;

                trueObjX = rX * cosf(thetaX);
                trueObjZ = rZ * sinf(thetaZ);

                const size_t floorsFound = collectFloorsAtXZ(
                    gpMapCollisionData, trueObjX, trueObjZ, isGroundValid,
                    isWaterValid || isCoronaMountain, MaxFloors, out, groundY);

                if (floorsFound == 0) {
                    tries += 1;
                    continue;
                }

                const int decidedFloor = floorsFound > 1 ? lerp<int>(0, floorsFound - 1, randLerp()) : 0;
                const TBGCheckData *floor = out[decidedFloor];
                trueObjY = groundY[decidedFloor];

                if (!isContextAllowed(gpMarDirector, trueObjX, trueObjY, trueObjZ, actorInfo, floor)) {
                    tries += 1;
                    continue;
                }

                if (floor->isWaterSurface() || floor->mCollisionType == 267 || floor->mCollisionType == 33035) {
                    {
                        const TBGCheckData *belowWater;
                        gpMapCollisionData->checkGround(trueObjX, trueObjY - 10.0f, trueObjZ, 0,
                                                        &belowWater);

                        // Below the water is death floor, therefore count as OOB
                        if (belowWater->mCollisionType == 1536 ||
                            belowWater->mCollisionType == 2048) {
                            tries += 1;
                            continue;
                        }
                    }
                    trueObjY =
                        solveWaterY(gpMarDirector, trueObjX, trueObjY, trueObjZ, actorInfo, floor);
                } else {
                    trueObjY =
                        solveGroundY(gpMarDirector, trueObjX, trueObjY, trueObjZ, actorInfo, floor);
                }

                break;
            }
        } while (tries < MaxTries);

        actor->mPosition.x = trueObjX;
        actor->mPosition.y = trueObjY;
        actor->mPosition.z = trueObjZ;
    }

    if (gRandomizeScaleSetting.getBool()) {
        if (actorInfo->mShouldResizeUniform) {
            const f32 scaleLerp = randLerp();

            if (actorInfo->mShouldResizeXZ) {
                actor->mSize.x = lerp<f32>(0.4f, 2.5f, scaleLerp);
                actor->mSize.z = lerp<f32>(0.4f, 2.5f, scaleLerp);
            }

            if (actorInfo->mShouldResizeY)
                actor->mSize.y = lerp<f32>(0.4f, 2.5f, scaleLerp);
        } else {
            if (actorInfo->mShouldResizeXZ) {
                actor->mSize.x = lerp<f32>(0.4f, 2.5f, randLerp());
                actor->mSize.z = lerp<f32>(0.4f, 2.5f, randLerp());
            }

            if (actorInfo->mShouldResizeY)
                actor->mSize.y = lerp<f32>(0.4f, 2.5f, randLerp());
        }
    }

    if (actorInfo->mShouldRotateXZ) {
        actor->mRotation.x = lerp<f32>(0, 360, randLerp());
        actor->mRotation.z = lerp<f32>(0, 360, randLerp());
    }

    if (actorInfo->mShouldRotateY)
        actor->mRotation.y = lerp<f32>(0, 360, randLerp());

    srand(gameSeed);

    return 0;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802F6EE0, 0, 0, 0), randomizeObject);
SMS_WRITE_32(SMS_PORT_REGION(0x802F6EE4, 0, 0, 0), 0x907E0010);
SMS_WRITE_32(SMS_PORT_REGION(0x802F6EE8, 0, 0, 0), 0x907E0014);