#include <Dolphin/types.h>
#include <Dolphin/math.h>

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
#include <SMS/Manager/FlagManager.hxx>

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

static TVec3f sSecretCourseStart          = TVec3f::zero();
static TVec3f sSecretCourseStartPlatform  = TVec3f::zero();
static TVec3f sSecretCourseCurrentPos     = TVec3f::zero();
static TVec3f sSecretCourseLastPos        = TVec3f::zero();
static TVec3f sSecretCourseCurrentDir     = TVec3f::zero();
static TVec3f sSecretCourseShinePos       = TVec3f::zero();
static size_t sSecretCourseObjectsLoaded  = 0;
static size_t sSecretCourseSwitchesLoaded = 0;
static bool sSecretCourseVertical         = false;
static bool sSecretCourseFluddless        = false;

static f32 getAreaOfTriangle(const TVectorTriangle& triangle) {
    const f32 lengthA = PSVECDistance(triangle.a, triangle.b);
    const f32 lengthB = PSVECDistance(triangle.b, triangle.c);
    const f32 lengthC = PSVECDistance(triangle.c, triangle.a);

    const f32 sP = (lengthA + lengthB + lengthC) / 2.0f;
    return sqrtf(sP * (sP - lengthA) * (sP - lengthB) * (sP - lengthC));
}

#define STR_EQUAL(a, b) strcmp(a, b) == 0

static void rotateWithNormal(const TVec3f& normal, TVec3f& out) {
    Mtx mtx;

    {
        const TVec3f up     = TVec3f::up();
        const TVec3f origin = TVec3f::zero();
        C_MTXLookAt(mtx, origin, up, normal);
    }

    out.setRotation(mtx);
}

bool isContextRandomizable(const TMarDirector &director, const HitActorInfo &actorInfo, const THitActor &actor) {
    if (SMS_isExMap__Fv()) {
        if (!gRandomizeExStageSetting.getBool())
            return false;

        constexpr const char *SkyIslandKey = "\x8B\xF3\x93\x87";
        if (STR_EQUAL(actor.mKeyName, SkyIslandKey)) {
            sSecretCourseStartPlatform = actor.mPosition;
            return false;
        }
    }

    if (!actorInfo.mShouldRandomize)
        return false;

    if (STR_EQUAL(actor.mKeyName, randoOffKey))
        return false;

    switch (director.mAreaID) {
    case 1: {  // Delfino Plaza
        if (STR_EQUAL(actor.mKeyName, "efDokanGate 0"))
            return false;
        if (STR_EQUAL(actor.mKeyName, "efDokanGate 1"))
            return false;
    }
    case 4: {  // Gelato Beach
        if (STR_EQUAL(actor.mKeyName, "ShiningStone"))
            return false;
        if (STR_EQUAL(actor.mKeyName, "coral00"))
            return false;
        if (STR_EQUAL(actor.mKeyName, "coral01"))
            return false;
        return true;
    }
    case 6: {  // Sirena Beach
        constexpr const char *hutKey = "\x83\x56\x83\x8C\x83\x69\x83\x56\x83\x87\x83\x62\x83\x76";
        if (STR_EQUAL(actor.mKeyName, hutKey))
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

bool isContextMakeSecretCourse(const TMarDirector& director) {
    if (!SMS_isExMap__Fv())
        return false;

    // Filter out whole courses
    if (director.mAreaID < 31)
        return false;

    if (director.mAreaID == 44)  // Noki Bottle
        return false;

    if (director.mAreaID == 52)  // Corona Mountain
        return false;

    return true;
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

    if (wallArea < 50000.0f)
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
    case 24: {  // Lily Pad Secret
        return y < 0.0f;
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

bool isRoofContextAllowed(const TMarDirector &director, f32 x, f32 y, f32 z,
                          const HitActorInfo &actorInfo, const TBGCheckData &wall) {
    if (y > 100000.0f)
        return false;

    if (actorInfo.mIsSprayableObj && y <= 0.0f)
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
    case 24: {  // Lily Pad Secret
        return y < 0.0f;
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

static void getSecretCourseStart(const TMarDirector &director, TVec3f &out) {
    switch (director.mAreaID) {
    case 31:  // Noki Secret
        out = {3000.0f, 4000.0f, 2000.0f};
        break;
    case 32:  // Gelato Secret
        out = {-5800.0f, 3800.0f, 200.0f};
        break;
    case 41:  // Yoshi Go-Round
        out = {800.0f, 1700.0f, 7700.0f};
        break;
    case 42:  // Pianta Village
        out = {-10900.0f, 1400.0f, 1900.0f};
        break;
    case 46:  // Hillside Cave Secret
        out = {0.0f, 8000.0f, 11000.0f};
        break;
    case 47:  // Hillside Cave Secret
        out = {-6100.0f, 3400.0f, 156.0f};
        break;
    case 48:  // Ricco Secret
        out = {-7200.0f, 4200.0f, 0.0f};
        break;
    case 50:  // Pinna Beach Secret
        out = {-8300.0f, 4600.0f, 0.0f};
        break;
    case 51:  // Gelato Secret
        out = {-9100.0f, 3650.0f, 1000.0f};
        break;
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

    if (isContextMakeSecretCourse(*gpMarDirector) && !actorInfo.mIsExLinear) {
        if (actorInfo.mShouldResizeXZ) {
            if (tryChance(5.0f)) {
                out.x *= 3.0f;
                out.z *= 3.0f;
            }
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

static size_t collectTrisAtBlock(TBGCheckList *list, size_t max, const TBGCheckData **out, u16 ignoreType) {
    size_t wallsFound = 0;
    while (list && list->mColTriangle && wallsFound < max) {
        if (list->mColTriangle->mType != ignoreType)
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
        rotateWithNormal(floor->mNormal, outRot);
        outRot.x += actorInfo.mAdjustRotation.x;
        outRot.y += actorInfo.mAdjustRotation.y;
        outRot.z += actorInfo.mAdjustRotation.z;
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

        staticFound = collectTrisAtBlock(
            collision.mStaticCollisionRoot[blockIdxX + (blockIdxZ * collision.mBlockXCount)]
                .mCheckList[TBGCheckListRoot::WALL]
                .mNextTriangle,
            256, staticWalls, 5);
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
        rotateWithNormal(wall->mNormal, outRot);
        outRot.x += actorInfo.mAdjustRotation.x;
        outRot.y += actorInfo.mAdjustRotation.y;
        outRot.z += actorInfo.mAdjustRotation.z;
    } else {
        getRandomizedRotation(outRot, actorInfo);
    }

    getRandomizedScale(outScl, actorInfo);

    return true;
}

static bool getRandomizedRoofPosition(TVec3f &outPos, TVec3f &outRot, TVec3f &outScl,
                                      const TMarDirector &director, TMapCollisionData &collision,
                                      const HitActorInfo &actorInfo) {
    constexpr f32 gridFraction = 1.0f / 1024.0f;

    const TBGCheckData *staticRoofs[256];

    size_t staticFound = 0;
    while (staticFound == 0) {
        getRandomizedPositionXZ(outPos, collision);

        const f32 boundsX = collision.mAreaSizeX;
        const f32 boundsZ = collision.mAreaSizeZ;

        /* Sample a block from the grid to pull walls from */
        const auto blockIdxX = static_cast<int>(gridFraction * (outPos.x + boundsX));
        const auto blockIdxZ = static_cast<int>(gridFraction * (outPos.z + boundsZ));

        staticFound = collectTrisAtBlock(
            collision.mStaticCollisionRoot[blockIdxX + (blockIdxZ * collision.mBlockXCount)]
                .mCheckList[TBGCheckListRoot::ROOF]
                .mNextTriangle,
            256, staticRoofs, 0xFFFF);
    }

    const int decidedRoof    = staticFound > 1 ? lerp<int>(0, staticFound - 1, randLerp()) : 0;
    const TBGCheckData *roof = staticRoofs[decidedRoof];

    TVectorTriangle triangle(roof->mVertices[0], roof->mVertices[1], roof->mVertices[2]);
    getRandomizedPointOnTriangle(outPos, triangle);

    const TBGCheckData *floor;
    f32 groundY = collision.checkGround(outPos.x, outPos.y, outPos.z, 1, &floor);

    if (outPos.y - groundY < 200.0f)
        return false;

    if (!isRoofContextAllowed(director, outPos.x, outPos.y, outPos.z, actorInfo, *roof))
        return false;

    TVec3f scaledNormal = roof->mNormal;
    scaledNormal.scale(actorInfo.mFromSurfaceDist + 1.0f);
    outPos.add(scaledNormal);

    if (actorInfo.mIsSurfaceBound) {
        rotateWithNormal(roof->mNormal, outRot);
        outRot.x += actorInfo.mAdjustRotation.x;
        outRot.y += actorInfo.mAdjustRotation.y;
        outRot.z += actorInfo.mAdjustRotation.z;
    } else {
        getRandomizedRotation(outRot, actorInfo);
    }

    getRandomizedScale(outScl, actorInfo);

    return true;
}

static bool getRandomizedSkyPosition(TVec3f &outPos, TVec3f &outRot, TVec3f &outScl,
                                       const TMarDirector &director,
                                       const TMapCollisionData &collision,
                                       const HitActorInfo &actorInfo) {
    const f32 fromGroundHeight   = getFromGroundHeight(actorInfo);
    getRandomizedPositionXZ(outPos, collision);

    f32 groundY;
    const TBGCheckData *floor;
    const size_t floorsFound = collectFloorsAtXZ(collision, outPos.x, outPos.z, true,
                                                 true, 1, &floor, &groundY);

    if (floorsFound == 0)
        return false;

    outPos.y = groundY;

    if (!isGroundContextAllowed(director, outPos.x, outPos.y, outPos.z, actorInfo, *floor))
        return false;

    getRandomizedRotation(outRot, actorInfo);
    getRandomizedScale(outScl, actorInfo);

    outPos.y += lerp(200.0f, 3000.0f, randLerp());

    return true;
}

// -- MAP

void initMapLoadStatus(TMarDirector *director) {
    u8 shineID = SMS_getShineIDofExStage__FUc(director->mAreaID);
    if (shineID == 255)
        sSecretCourseFluddless = false;
    else
        sSecretCourseFluddless = !TFlagManager::smInstance->getShineFlag(shineID);

    OSReport("Is Fluddless %d\n", sSecretCourseFluddless);

    sIsMapLoaded = false;
    sStageSeed   = getGameSeed();
    sStageSeed ^= ((director->mAreaID << 8) | director->mEpisodeID);
    sStageSeed *= 0x12345678;

    if (sSecretCourseFluddless)
        sStageSeed ^= 0xDEADBEEF;

    srand32(sStageSeed);

    getSecretCourseStart(*gpMarDirector, sSecretCourseStart);
    sSecretCourseStartPlatform = sSecretCourseStart;
    sSecretCourseCurrentPos = sSecretCourseStart;
    sSecretCourseLastPos    = sSecretCourseCurrentPos;
    sSecretCourseCurrentDir = sSecretCourseCurrentPos;
    sSecretCourseCurrentDir.negate();
    sSecretCourseCurrentDir.normalize();

    sSecretCourseObjectsLoaded = 0;
    sSecretCourseSwitchesLoaded = 0;

    sSecretCourseVertical = tryChance(30.0f);

}

static void setMapLoaded(TMapCollisionStatic *staticCol) {
    staticCol->setUp();
    sIsMapLoaded = true;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801896E0, 0, 0, 0), setMapLoaded);

// --

// riccoSeaPollutionS2
// mareSeaPollutionS12

enum class PlaneSelection {
    FLOOR,
    ROOF,
    WALL,
    SKY
};

static void randomizeStageObject(TVec3f &outPos, TVec3f &outRot, TVec3f &outScl,
                                 const TMarDirector &director, TMapCollisionData &collision,
                                 const HitActorInfo &actorInfo) {
    constexpr size_t MaxFloors = 8;
    constexpr size_t MaxTries  = 10000;

    const TBGCheckData *out[MaxFloors];
    f32 groundY[MaxFloors];

    size_t tries = 0;
    bool positionFound = false;

    PlaneSelection selection = PlaneSelection::SKY;

    if (actorInfo.mIsGroundValid || actorInfo.mIsWaterValid || actorInfo.mIsUnderwaterValid) {
        selection = PlaneSelection::FLOOR;
        if (actorInfo.mIsWallValid && tryChance(50.0f))
            selection = PlaneSelection::WALL;
        if (actorInfo.mIsRoofValid && tryChance(25.0f))
            selection = PlaneSelection::ROOF;
    } else if (actorInfo.mIsWallValid) {
        selection = PlaneSelection::WALL;
        if (actorInfo.mIsRoofValid && tryChance(35.0f))
            selection = PlaneSelection::ROOF;
    } else if (actorInfo.mIsRoofValid) {
        selection = PlaneSelection::ROOF;
    }

    do {
        switch (selection) {
        default:
        case PlaneSelection::FLOOR:
            positionFound = getRandomizedFloorPosition(outPos, outRot, outScl, director, collision, actorInfo);
            break;
        case PlaneSelection::ROOF:
            positionFound =
                getRandomizedRoofPosition(outPos, outRot, outScl, director, collision, actorInfo);
            break;
        case PlaneSelection::WALL:
            positionFound =
                getRandomizedWallPosition(outPos, outRot, outScl, director, collision, actorInfo);
            break;
        case PlaneSelection::SKY:
            positionFound =
                getRandomizedSkyPosition(outPos, outRot, outScl, director, collision, actorInfo);
            break;
        }
        if (positionFound)
            break;

        tries += 1;
    } while (tries < MaxTries);
}

static void randomizeSecretCourseObject(TVec3f &outPos, TVec3f &outRot, TVec3f &outScl,
                                 const TMarDirector &director, TMapCollisionData &collision,
                                 const HitActorInfo &actorInfo) {
    getRandomizedScale(outScl, actorInfo);

    if (actorInfo.mIsShineObj) {
        const TBGCheckData *roof;
        const f32 roofHeight = collision.checkRoof(sSecretCourseShinePos.x, sSecretCourseShinePos.y + 200.0f,
                                                   sSecretCourseShinePos.z, 0, &roof);

        outPos.x = sSecretCourseShinePos.x;
        outPos.y = Min(roofHeight - 100.0f, sSecretCourseShinePos.y + actorInfo.mFromSurfaceDist + 200.0f);
        outPos.z = sSecretCourseShinePos.z;
        
        getRandomizedRotation(outRot, actorInfo);
        return;
    }

    if (actorInfo.mIsSwitchObj) {
        switch (sSecretCourseSwitchesLoaded) {
        default:
        case 0:
            outPos = sSecretCourseStartPlatform;
            outPos.x += 500.0f;
            break;
        case 1:
            outPos = sSecretCourseStartPlatform;
            outPos.x -= 500.0f;
            break;
        case 2:
            outPos = sSecretCourseStartPlatform;
            outPos.z += 500.0f;
            break;
        case 3:
            outPos = sSecretCourseStartPlatform;
            outPos.z -= 500.0f;
            break;
        }
        return;
    }

    if (actorInfo.mIsExLinear) {
        Mtx rot;

        TVec3f pprevObjPos = sSecretCourseLastPos;
        TVec3f prevObjPos  = sSecretCourseCurrentPos;

        do {
            const f32 theta = 2.0f * M_PI * randLerp();
            const f32 phi   = acosf(2.0f * randLerp() - 1.0f);

            f32 horizontalRadius;
            f32 verticalRadius;
            if (sSecretCourseFluddless) {
                horizontalRadius = sSecretCourseVertical ? 1400.0f : 2000.0f;
                verticalRadius   = sSecretCourseVertical ? 1400.0f : 1300.0f;
            } else {
                horizontalRadius = sSecretCourseVertical ? 2000.0f : 2500.0f;
                verticalRadius   = sSecretCourseVertical ? 1600.0f : 1500.0f;
            }

            const f32 adjustX = horizontalRadius * outScl.x * sinf(phi) * cosf(theta);
            const f32 adjustY = verticalRadius * outScl.y * sinf(phi) * sinf(theta) + 100.0f;
            const f32 adjustZ = horizontalRadius * outScl.z * cosf(phi);

            const f32 xzDistance = sqrtf(adjustX * adjustX + adjustZ * adjustZ);

            if (xzDistance < (sSecretCourseVertical ? 1100.0f : 1400.0f))
                continue;

            if (xzDistance < (sSecretCourseVertical ? 2200.0f : 2800.0f))
                continue;

            outPos.x = prevObjPos.x + adjustX;
            outPos.y = prevObjPos.y + adjustY;
            outPos.z = prevObjPos.z + adjustZ;

        } while (PSVECDistance(prevObjPos, outPos) >
                 PSVECDistance(pprevObjPos, outPos) && outPos.y < 500.0f);
        
        sSecretCourseLastPos    = sSecretCourseCurrentPos;
        sSecretCourseCurrentPos = outPos;

        {
            const TVec3f up     = TVec3f::up();
            const TVec3f origin = prevObjPos;
            C_MTXLookAt(rot, origin, up, outPos);
        }

        PSMTXInverse(rot, rot);
        outRot.setRotation(rot);
        return;
    }
    outPos.y = 0.0f;
    while (outPos.y < 500.0f) {
        const f32 theta = 2.0f * M_PI * randLerp();
        const f32 phi   = acosf(2.0f * randLerp() - 1.0f);

        f32 horizontalRadius;
        f32 verticalRadius;
        if (sSecretCourseFluddless) {
            horizontalRadius = sSecretCourseVertical ? 400.0f : 500.0f;
            verticalRadius   = sSecretCourseVertical ? 250.0f : 350.0f;
        } else {
            horizontalRadius = sSecretCourseVertical ? 550.0f : 700.0f;
            verticalRadius   = sSecretCourseVertical ? 600.0f : 500.0f;
        }

        const f32 adjustX = horizontalRadius * outScl.x * sinf(phi) * cosf(theta);
        const f32 adjustY = verticalRadius * outScl.y * sinf(phi) * sinf(theta) + (sSecretCourseVertical ? 50.0f : 0.0f);
        const f32 adjustZ = horizontalRadius * outScl.z * cosf(phi);

        outPos.x = sSecretCourseCurrentPos.x + adjustX;
        outPos.y = sSecretCourseCurrentPos.y + adjustY;
        outPos.z = sSecretCourseCurrentPos.z + adjustZ;

    }
    sSecretCourseLastPos    = sSecretCourseCurrentPos;
    sSecretCourseCurrentPos = outPos;

    getRandomizedRotation(outRot, actorInfo);
}

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

    const u32 gameSeed         = getGameSeed();
    u32 key            = (actor->mKeyCode * 0x41C64E6D + 0x3039) ^ gameSeed;

    if (sSecretCourseFluddless)
        key *= 0x51678391;

    srand32(key);

    {
        TVec3f truePosition = actor->mPosition;
        TVec3f trueRotation = actor->mRotation;
        TVec3f trueScale = actor->mSize;

        const bool isEntity =
            ((actorInfo->mIsItemObj || actorInfo->mIsPlayer) && !actorInfo->mIsShineObj);

        if (isContextMakeSecretCourse(*gpMarDirector) && !isEntity)
            randomizeSecretCourseObject(truePosition, trueRotation, trueScale, *gpMarDirector,
                                        *gpMapCollisionData, *actorInfo);
        else
            randomizeStageObject(truePosition, trueRotation, trueScale, *gpMarDirector,
                                 *gpMapCollisionData, *actorInfo);

        actor->mPosition = truePosition;
        actor->mRotation = trueRotation;
        actor->mSize     = trueScale;
    }

    if (STR_EQUAL(actor->mKeyName, "normalvariant3") ||
        (STR_EQUAL(actor->mKeyName, "WoodBlockLarge 0") && gpMarDirector->mAreaID == 32)) {
        sSecretCourseShinePos = actor->mPosition;
    }

    srand(gameSeed);

    return 0;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802F6EE0, 0, 0, 0), randomizeObject);
SMS_WRITE_32(SMS_PORT_REGION(0x802F6EE4, 0, 0, 0), 0x907E0010);
SMS_WRITE_32(SMS_PORT_REGION(0x802F6EE8, 0, 0, 0), 0x907E0014);

#undef STR_EQUAL