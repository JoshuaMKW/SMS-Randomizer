#include <Dolphin/types.h>

#include <JSystem/JDrama/JDRActor.hxx>
#include <JSystem/JDrama/JDRNameRefGen.hxx>
#include <JSystem/JGeometry/JGMQuat.hxx>
#include <JSystem/JGeometry/JGMRotation.hxx>
#include <JSystem/JGeometry/JGMVec.hxx>

#include <SMS/rand.h>
#include <SMS/raw_fn.hxx>
#include <SMS/macros.h>
#include <SMS/Strategic/Strategy.hxx>
#include <SMS/Map/Map.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/Map/MapCollisionStatic.hxx>
#include <SMS/System/MarDirector.hxx>

#include <BetterSMS/libs/triangle.hxx>
#include <BetterSMS/libs/geometry.hxx>
#include <BetterSMS/libs/constmath.hxx>
#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"
#include "seed.hxx"
#include "settings.hxx"

constexpr const char *randoOffKey = "randomizer_off";

bool sIsMapLoaded;
u32 sStageSeed;

static f32 getAreaOfTriangle(const TVectorTriangle& triangle) {
    const f32 lengthA = PSVECDistance(triangle.a, triangle.b);
    const f32 lengthB = PSVECDistance(triangle.b, triangle.c);
    const f32 lengthC = PSVECDistance(triangle.c, triangle.a);

    const f32 sP = (lengthA + lengthB + lengthC) / 2.0f;
    return sqrtf(sP * (sP - lengthA) * (sP - lengthB) * (sP - lengthC));
}

static void rotateWithNormal(const TVec3f& normal, TVec3f& out) {
    using namespace JGeometry;

    TQuat4f quat;
    TVec3f up = TVec3f::up();

    //TRotation3<TMatrix34<SMatrix34<f32>>> rotation;
    ////rotation.setLookDir(normal, TVec3f::up());
    //setLookDir__Q29JGeometry64TRotation3_Q(&rotation, &normal, &up);

    Mtx mtx;
    TVec3f origin = {0, 0, 0};
    C_MTXLookAt(mtx, origin, up, normal);

    /*out.z = radiansToAngle(atan2f(mtx[1][0], mtx[0][0]));
    out.y = radiansToAngle(atan2f(-mtx[2][0], sqrtf(1.0f - mtx[2][0] * mtx[2][0])));
    out.x = radiansToAngle(atan2f(mtx[2][1], mtx[2][2]));*/

    f32 sy = sqrtf(mtx[2][1] * mtx[2][1] + mtx[2][0] * mtx[2][0]);
    if (sy < 10 * __FLT_EPSILON__) {
        out.x = radiansToAngle(atan2f(mtx[2][1], mtx[2][0]));
        out.y = radiansToAngle(atan2f(sy, mtx[2][2]));
        out.z = 0;
    } else {
        out.x = radiansToAngle(atan2f(mtx[2][1], mtx[2][0]));
        out.y = radiansToAngle(atan2f(sy, mtx[2][2]));
        out.z = radiansToAngle(atan2f(mtx[1][2], -mtx[0][2]));
    }
}

bool isContextRandomizable(const TMarDirector &director, const HitActorInfo &actorInfo, const THitActor &actor) {
    if (SMS_isExMap__Fv() && !gRandomizeExStageSetting.getBool())
        return false;

    if (!actorInfo.mShouldRandomize)
        return false;

    if (strcmp(actor.mKeyName, randoOffKey) == 0)
        return false;

    switch (director.mAreaID) {
    case 4: {  // Gelato Beach
        if (strcmp(actor.mKeyName, "ShiningStone") == 0)
            return false;
        if (strcmp(actor.mKeyName, "coral00") == 0)
            return false;
        if (strcmp(actor.mKeyName, "coral01") == 0)
            return false;
        return true;
    }
    case 6: {  // Sirena Beach
        constexpr const char *hutKey = "\x83\x56\x83\x8C\x83\x69\x83\x56\x83\x87\x83\x62\x83\x76";
        if (strcmp(actor.mKeyName, hutKey) == 0)
            return false;
        return true;
    }
    case 9:  // Noki Bay
        if (director.mEpisodeID == 2)  // Bottle mission entry
            return false;
        return true;
    case 13:  // Pinna Park
        if (director.mEpisodeID == 6)  // Talk to guy before fight
            return false;
        return true;
    case 15:  // File Select
        if (director.mEpisodeID == 0)
            return false;
        return true;
    case 33:  // Sand Bird
        if (director.mEpisodeID == 0)
            return false;
        return true;
    case 60:
        if (director.mEpisodeID == 0)
            return false;
        return true;
    default:
        return true;
    }
}

bool isGroundContextAllowed(const TMarDirector &director, f32 x, f32 y, f32 z, const HitActorInfo &actorInfo,
                            const TBGCheckData &floor) {
    // Filter out warp collision
    if ((floor.mType & 0x3FF) == 768)
        return false;

    TVec3f position = {x, y, z};

    switch (director.mAreaID) {
    case 0:
    case 20: {  // Airstrip
        // Prevent invisible room spawns
        if (y > 5000.0f)
            return false;

        // Prevent OOB spawns
        const TVec3f center {800.0f, 0.0f, -2000.0f};
        const TVec3f diff { x, 0.0f, z };
        return PSVECDistance(diff, center) < 15000.0f;
    }
    case 2: {  // Bianco Hills
        // Prevent OOB slope spawns
        if (y < 1700.0f && !floor.isWaterSurface())
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
        if (y < 0.0f && !floor.isWaterSurface())
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

        return true;
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
        if (y < -650.0f && (floor.mType & 0x3FF) == 1)
            return false;
        return true;
    }
    case 9: {  // Noki Bay
        // Prevent high ground OOB spawn
        if (x > -300.0f && y > 12000.0f && z > 1500.0f)
            return false;
        if (y < 0.0f && !floor.isWaterSurface())
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
    case 44:  // Bottle
        // Prevent OOB spawns
        if (PSVECMag(position) > 3300.0f)
            return false;
        if (y > -4000.0f)
            return false;
        return true;
    case 52: {  // Corona mountain
        // Prevent OOB water spawns
        if (x < -2500.0f || x > 2500.0f)
            return false;
        return true;
    }
    case 55: {  // Bianco Hills
        // Prevent Petey windmill OOB
        return PSVECMag(position) < 1800.0f;
    }
    default:
        return true;
    }
}

bool isWallContextAllowed(const TMarDirector &director, f32 x, f32 y, f32 z,
                          const HitActorInfo &actorInfo, const TBGCheckData &wall) {
    // Filter out warp collision
    if ((wall.mType & 0x3FF) == 5)
        return false;

    if (actorInfo.mIsSprayableObj && y <= 0.0f)
        return false;

    const f32 wallArea =
        getAreaOfTriangle({wall.mVertices[0], wall.mVertices[1], wall.mVertices[2]});

    OSReport("Area of triangle (%.02f cm)\n", wallArea);

    if (wallArea < 100000.0f)
        return false;

    TVec3f position = {x, y, z};
    TVec3f origin   = {0.0f, 0.0f, 0.0f};

    switch (director.mAreaID) {
    case 0: {  // Airstrip
        // Prevent invisible room spawns
        if (y > 5000.0f)
            return false;
        return true;
    }
    case 1: {  // Delfino Plaza
        if (y > 3000.0f && z < -8200.0f)
            return false;
        if (y <= 0.0f)
            return false;
        return true;
    }
    case 2: {  // Bianco Hills
        return true;
    }
    case 3: {  // Ricco Harbor
        if (x > -2000.0f)
            return y < 4600.0f;
        return y < 3600.0f;
    }
    case 5: {  // Pinna Beach
        return true;
    }
    case 6: {  // Sirena Beach
        if (y > 3000.0f)
            return false;
        return true;
    }
    case 8: {  // Pianta Village
        // Prevent cheap trunk spawns
        if (y < -650.0f)
            return false;
        return true;
    }
    case 9: {  // Noki Bay
        // Prevent high ground OOB spawn
        if (y > 12000.0f)
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
        if (y < 0.0f)
            return false;
        return true;
    }
    case 44:  // Bottle
        // Prevent OOB spawns
        if (y > -4000.0f)
            return false;
        return true;
    case 52: {  // Corona mountain
        return true;
    }
    default:
        return true;
    }
}

static f32 getFromGroundHeight(const HitActorInfo &actorInfo) {
    if (actorInfo.mIsItemObj && tryChance(5.0f)) {
        return actorInfo.mFromSurfaceDist + 3000.0f;
    }
    return actorInfo.mFromSurfaceDist;
}

static void getRandomizedPointOnTriangle(TVec3f& out, const TVectorTriangle& triangle) {
    const f32 lerpA = sqrt(randLerp());
    const f32 lerpB = randLerp();

    TVec3f compA = triangle.a;
    TVec3f compB = triangle.b;
    TVec3f compC = triangle.c;

    compA.scale(1 - lerpA);
    compB.scale(lerpA * (1 - lerpB));
    compC.scale(lerpA * lerpB);

    out.set(compA + compB + compC);
}

static void getRandomizedPositionXZ(TVec3f &out, const TMapCollisionData &collision) {
    const f32 boundsX          = collision.mAreaSizeX;
    const f32 boundsZ          = collision.mAreaSizeZ;

    f32 rX     = boundsX * sqrtf(randLerp());
    f32 thetaX = sqrtf(randLerp()) * 2 * M_PI;

    f32 rZ     = boundsZ * sqrtf(randLerp());
    f32 thetaZ = sqrtf(randLerp()) * 2 * M_PI;

    out.x = rX * cosf(thetaX);
    out.z = rZ * sinf(thetaZ);
}

static void getRandomizedRotation(TVec3f &out, const HitActorInfo &actorInfo) {
    if (actorInfo.mShouldRotateXZ) {
        out.x = lerp<f32>(0, 360, randLerp()) + actorInfo.mAdjustRotation.x;
        out.z = lerp<f32>(0, 360, randLerp()) + actorInfo.mAdjustRotation.z;
    }

    if (actorInfo.mShouldRotateY) {
        out.y = lerp<f32>(0, 360, randLerp()) + actorInfo.mAdjustRotation.y;
    }
}

static void getRandomizedScale(TVec3f &out, const HitActorInfo &actorInfo) {
    if (gRandomizeScaleSetting.getBool()) {
        if (actorInfo.mShouldResizeUniform) {
            const f32 scaleLerp = randLerp();

            if (actorInfo.mShouldResizeXZ) {
                out.x = lerp<f32>(0.4f, 2.5f, scaleLerp);
                out.z = lerp<f32>(0.4f, 2.5f, scaleLerp);
            }

            if (actorInfo.mShouldResizeY)
                out.y = lerp<f32>(0.4f, 2.5f, scaleLerp);
        } else {
            if (actorInfo.mShouldResizeXZ) {
                out.x = lerp<f32>(0.4f, 2.5f, randLerp());
                out.z = lerp<f32>(0.4f, 2.5f, randLerp());
            }

            if (actorInfo.mShouldResizeY)
                out.y = lerp<f32>(0.4f, 2.5f, randLerp());
        }
    }
}

static size_t collectFloorsAtXZ(const TMapCollisionData &collision, f32 x, f32 z, bool groundValid, bool waterValid, size_t capacity, const TBGCheckData **out, f32 *yOut) {
    size_t found = 0;

    f32 checkY = 100000.0f;
    while (found < capacity) {
        const TBGCheckData *tmpOut;
        const f32 groundY = collision.checkGround(x, checkY, z, 0, &tmpOut);

        checkY = groundY - 100.0f;

        if (tmpOut->mType == 1536 || tmpOut->mType == 2048)
            return found;

        if (!waterValid && (tmpOut->isWaterSurface() || tmpOut->mType == 267 ||
                            tmpOut->mType == 33035))
            return found;

        if (!groundValid && !(tmpOut->isWaterSurface() || tmpOut->mType == 267 ||
            tmpOut->mType == 33035))
            return found;

        out[found] = tmpOut;
        yOut[found] = groundY;
        found += 1;
    }

    return found;
}

static size_t collectWallsAtBlock(TBGCheckList *list, size_t max, const TBGCheckData **out) {
    size_t wallsFound = 0;
    while (list && list->mColTriangle && wallsFound < max) {
        if (list->mColTriangle->mType != 5)
            out[wallsFound++] = list->mColTriangle;
        list = list->mNextTriangle;
    }
    return wallsFound;
}

static f32 solveWaterY(const TMarDirector &director, f32 x, f32 y, f32 z, const HitActorInfo &actorInfo,
                       const TBGCheckData &water) {
    const TBGCheckData *data;
    switch (director.mAreaID) {
    case 52: {  // Corona mountain
        if (actorInfo.mIsItemObj)
            return y + actorInfo.mFromSurfaceDist + 400.0f;  // Adjust to make it not touch lava

        if (actorInfo.mIsUnderwaterValid && tryChance(70.0f)) {
            const f32 belowGroundY = gpMapCollisionData->checkGround(x, y - 10.0f, z, 0, &data);
            return lerp<f32>(belowGroundY + actorInfo.mFromSurfaceDist, y - 10.0f, randLerp());
        }

        return y + actorInfo.mFromSurfaceDist;
    }
    default: {
        if (actorInfo.mIsUnderwaterValid && tryChance(70.0f)) {
            const f32 belowGroundY = gpMapCollisionData->checkGround(x, y - 10.0f, z, 0, &data);
            return lerp<f32>(belowGroundY + actorInfo.mFromSurfaceDist, y - 10.0f, randLerp());
        }
        return y + actorInfo.mFromSurfaceDist;
    }
    }
}

static f32 solveGroundY(const TMarDirector &director, f32 x, f32 y, f32 z, const HitActorInfo &actorInfo,
                       const TBGCheckData &ground) {
    switch (director.mAreaID) {
    default: {
        return y + actorInfo.mFromSurfaceDist;
    }
    }
}

static bool getRandomizedFloorPosition(TVec3f &outPos, TVec3f &outRot, TVec3f &outScl,
                                       const TMarDirector &director,
                                       const TMapCollisionData &collision,
                                       const HitActorInfo &actorInfo) {
    const bool isUnderwaterValid = actorInfo.mIsUnderwaterValid;
    const bool isWaterValid      = actorInfo.mIsWaterValid;
    const bool isGroundValid     = actorInfo.mIsGroundValid;
    const f32 fromGroundHeight = getFromGroundHeight(actorInfo);

    const bool isCoronaMountain = director.mAreaID == 52;

    constexpr size_t MaxFloors = 8;
    const TBGCheckData *floors[MaxFloors];
    f32 groundY[MaxFloors];

    getRandomizedPositionXZ(outPos, collision);

    const size_t floorsFound =
        collectFloorsAtXZ(collision, outPos.x, outPos.z, isGroundValid,
                          isWaterValid, MaxFloors, floors, groundY);

    if (floorsFound == 0)
        return false;

    const int decidedFloor    = floorsFound > 1 ? lerp<int>(0, floorsFound - 1, randLerp()) : 0;
    const TBGCheckData *floor = floors[decidedFloor];
    outPos.y                     = groundY[decidedFloor];

    if (!isGroundContextAllowed(director, outPos.x, outPos.y, outPos.z, actorInfo, *floor))
        return false;

    if (floor->isWaterSurface() || floor->mType == 267 || floor->mType == 33035) {
        {
            const TBGCheckData *belowWater;
            collision.checkGround(outPos.x, outPos.y - 10.0f, outPos.z, 0, &belowWater);

            // Below the water is death floor, therefore count as OOB
            if (belowWater->mType == 1536 || belowWater->mType == 2048)
                return false;
        }
        outPos.y = solveWaterY(director, outPos.x, outPos.y, outPos.z, actorInfo, *floor);
    } else {
        outPos.y = solveGroundY(director, outPos.x, outPos.y, outPos.z, actorInfo, *floor);
    }

    if (actorInfo.mIsSurfaceBound) {
        /*outRot.set(radiansToAngle(atan2f(floor->mNormal.z, floor->mNormal.y)) +
                       actorInfo.mAdjustRotation.x,
                   radiansToAngle(atan2f(floor->mNormal.x, floor->mNormal.z)) +
                       actorInfo.mAdjustRotation.y,
                   radiansToAngle(atan2f(floor->mNormal.y, floor->mNormal.x)) +
                       actorInfo.mAdjustRotation.z);*/
        rotateWithNormal(floor->mNormal, outRot);
        /*outRot.x += actorInfo.mAdjustRotation.x;
        outRot.y += actorInfo.mAdjustRotation.y;
        outRot.z += actorInfo.mAdjustRotation.z;*/
    } else {
        getRandomizedRotation(outRot, actorInfo);
    }

    getRandomizedScale(outScl, actorInfo);

    return true;
}

static bool getRandomizedWallPosition(TVec3f &outPos, TVec3f &outRot, TVec3f &outScl,
                                      const TMarDirector &director,
                                      TMapCollisionData &collision,
                                      const HitActorInfo &actorInfo) {
    constexpr f32 gridFraction = 1.0f / 1024.0f;

    const TBGCheckData *staticWalls[256];

    size_t staticFound = 0;
    while (staticFound == 0) {
        getRandomizedPositionXZ(outPos, collision);

        const f32 boundsX  = collision.mAreaSizeX;
        const f32 boundsZ  = collision.mAreaSizeZ;

        /* Sample a block from the grid to pull walls from */
        const auto blockIdxX = static_cast<int>(gridFraction * (outPos.x + boundsX));
        const auto blockIdxZ = static_cast<int>(gridFraction * (outPos.z + boundsZ));

        staticFound = collectWallsAtBlock(
            collision.mStaticCollisionRoot[blockIdxX + (blockIdxZ * collision.mBlockXCount)]
                .mCheckList[TBGCheckListRoot::WALL]
                .mNextTriangle,
            256, staticWalls);
    }

    const int decidedWall    = staticFound > 1 ? lerp<int>(0, staticFound - 1, randLerp()) : 0;
    const TBGCheckData *wall = staticWalls[decidedWall];
    
    TVectorTriangle triangle(wall->mVertices[0], wall->mVertices[1], wall->mVertices[2]);
    getRandomizedPointOnTriangle(outPos, triangle);

    const TBGCheckData *floor;
    f32 groundY = collision.checkGround(outPos.x, outPos.y, outPos.z, 1, &floor);

    if (floor->mType == 1536 || floor->mType == 2048) {
        if (outPos.y - groundY < 800.0f)
            return false;
    } else {
        if (outPos.y - groundY < 200.0f)
            return false;
    }

    if (!isWallContextAllowed(director, outPos.x, outPos.y, outPos.z, actorInfo, *wall))
        return false;

    TVec3f scaledNormal = wall->mNormal;
    scaledNormal.scale(actorInfo.mFromSurfaceDist + 1.0f);
    outPos.add(scaledNormal);

    if (actorInfo.mIsSurfaceBound) {
        /*outRot.set(actorInfo.mAdjustRotation.x,
                   radiansToAngle(atan2f(floor->mNormal.x, floor->mNormal.z)) +
                       actorInfo.mAdjustRotation.y,
                   actorInfo.mAdjustRotation.z);*/
        rotateWithNormal(wall->mNormal, outRot);
        /*outRot.x += actorInfo.mAdjustRotation.x;
        outRot.y += actorInfo.mAdjustRotation.y;
        outRot.z += actorInfo.mAdjustRotation.z;*/
    } else {
        getRandomizedRotation(outRot, actorInfo);
    }

    getRandomizedScale(outScl, actorInfo);

    return true;
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

// --

// riccoSeaPollutionS2
// mareSeaPollutionS12

static u32 randomizeObject() {
    THitActor *actor;
    SMS_FROM_GPR(31, actor);

    actor->mRootRef = JDrama::TNameRefGen::getInstance()->getRootNameRef();

    if (!sIsMapLoaded)
        return 0;

    HitActorInfo *actorInfo = getRandomizerInfo(actor);
    if (!actorInfo->mShouldRandomize)
        return 0;

    if (!isContextRandomizable(*gpMarDirector, *actorInfo, *actor))
        return 0;

    
    if (gpMarDirector->mAreaID == 1) {  // Delfino Plaza
        if (strcmp(actor->mKeyName, "GateToRicco") == 0) {
            actorInfo->mFromSurfaceDist = 100.0f;
        }
    }

    constexpr size_t MaxFloors = 8;
    constexpr size_t MaxTries = 100;

    const u32 gameSeed         = getGameSeed();

    size_t tries = 0;
    u32 key      = (actor->mKeyCode * 0x41C64E6D + 0x3039) ^ gameSeed;
    srand32(key);
    {
        TVec3f truePosition = actor->mPosition;
        TVec3f trueRotation = actor->mRotation;
        TVec3f trueScale = actor->mSize;

        const TBGCheckData *out[MaxFloors];
        f32 groundY[MaxFloors];

        do {
            bool positionFound;

            bool useWalls = actorInfo->mIsWallValid;
            if (actorInfo->mIsGroundValid) {
                useWalls &= tryChance(75.0f);
            }

            if (useWalls)
                positionFound =
                    getRandomizedWallPosition(truePosition, trueRotation, trueScale, *gpMarDirector,
                                              *gpMapCollisionData, *actorInfo);
            else
                positionFound =
                    getRandomizedFloorPosition(truePosition, trueRotation, trueScale, *gpMarDirector,
                                               *gpMapCollisionData, *actorInfo);
            if (positionFound)
                break;

            tries += 1;
        } while (tries < MaxTries);

        actor->mPosition = truePosition;
        actor->mRotation = trueRotation;
        actor->mSize     = trueScale;
    }

    srand(gameSeed);

    return 0;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802F6EE0, 0, 0, 0), randomizeObject);
SMS_WRITE_32(SMS_PORT_REGION(0x802F6EE4, 0, 0, 0), 0x907E0010);
SMS_WRITE_32(SMS_PORT_REGION(0x802F6EE8, 0, 0, 0), 0x907E0014);