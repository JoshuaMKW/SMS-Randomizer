#include <Dolphin/DVD.h>

#include <JSystem/JGadget/UnorderedMap.hxx>
#include <JSystem/JGadget/List.hxx>

#include <SMS/Map/MapCollisionStatic.hxx>
#include <SMS/Map/PollutionLayer.hxx>
#include <SMS/Manager/FlagManager.hxx>
#include <SMS/Manager/PollutionManager.hxx>
#include <SMS/raw_fn.hxx>

#include <BetterSMS/libs/triangle.hxx>
#include <BetterSMS/libs/constmath.hxx>
#include <BetterSMS/libs/global_unordered_map.hxx>
#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"
#include "seed.hxx"
#include "settings.hxx"
#include "solver.hxx"

static JGadget::TList<u16> sWarpIDWhiteList;
static TGlobalUnorderedMap<u16, Randomizer::BaseSolver *> sSolvers;
static Randomizer::BaseSolver *sDefaultSolver = nullptr;

static void initWarpList(TNameRefPtrAryT<TNameRefAryT<TScenarioArchiveName>> *stageList) {
    gpApplication.mStageArchiveAry = stageList;

    sWarpIDWhiteList.erase(sWarpIDWhiteList.begin(), sWarpIDWhiteList.end());

    int areaID = 0;
    for (auto &iter : gpApplication.mStageArchiveAry->mChildren) {
        auto *area    = reinterpret_cast<TNameRefAryT<TScenarioArchiveName> *>(iter);
        int episodeID = 0;
        for (auto &episode : area->mChildren) {
            char stageName[128];
            snprintf(stageName, 128, "/data/scene/%s", episode.mArchiveName);
            char *loc = strstr(stageName, ".arc");
            if (loc) {
                strncpy(loc, ".szs", 4);
            }

            if (DVDConvertPathToEntrynum(stageName) >= 0) {
                sWarpIDWhiteList.insert(sWarpIDWhiteList.end(), ((areaID + 1) << 8) | episodeID);
            }

            episodeID++;
        }
        areaID++;
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802A6BE8, 0, 0, 0), initWarpList);

void initDefaultSolver(TApplication *app) { sDefaultSolver = new (JKRHeap::sRootHeap, 4) Randomizer::SMSSolver(); }

/*
/ API
*/
namespace Randomizer {
    bool isWarpIDValid(u16 warpID) {
        for (auto &id : sWarpIDWhiteList) {
            if (warpID == id) {
                return true;
            }
        }
        return false;
    }

    bool isSolverRegistered(u8 area, u8 episode) {
        const u16 warpID = ((area + 1) << 8) | episode;
        return sSolvers.contains(warpID);
    }

    BaseSolver *getSolver(u8 area, u8 episode) {
        const u16 warpID = ((area + 1) << 8) | episode;
        if (!isSolverRegistered(area, episode))
            return sDefaultSolver;
        return sSolvers[warpID];
    }

    bool registerSolver(u8 area, u8 episode, BaseSolver *solver) {
        const u16 warpID = ((area + 1) << 8) | episode;
        if (isSolverRegistered(area, episode)) {
            OSReport("Tried to register a solver for a stage that already has one! (Area: %d, "
                     "Episode: %d)\n",
                     area, episode);
            return false;
        }
        sSolvers[warpID] = solver;
        return true;
    }

    bool deregisterSolver(u8 area, u8 episode) {
        const u16 warpID = ((area + 1) << 8) | episode;
        if (!isSolverRegistered(area, episode))
            return false;
        sSolvers.erase(warpID);
        return true;
    }

    bool setDefaultSolver(BaseSolver *solver) {
        if (!solver)
            return false;
        sDefaultSolver = solver;
        return true;
    }
}

extern void initializeDefaultActorInfo(const TMarDirector &director, HitActorInfo &actorInfo);
extern void resetHitHideObjs();
extern void addHitHideObj(TMapObjBase *actor);

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

static THitActor *sGelatoWarp = nullptr;

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

// -- MAP

extern void resetStageWarpInfo();

void initMapLoadStatus(TMarDirector *director) {
    u8 shineID = SMS_getShineIDofExStage__FUc(director->mAreaID);
    if (shineID == 255)
        sSecretCourseFluddless = false;
    else
        sSecretCourseFluddless = !TFlagManager::smInstance->getShineFlag(shineID);

    sStageSeed   = Randomizer::getGameSeed();
    sStageSeed ^= ((director->mAreaID << 8) | director->mEpisodeID);
    sStageSeed *= 0x12345678;

    if (sSecretCourseFluddless)
        sStageSeed ^= 0xDEADBEEF;

    Randomizer::srand32(sStageSeed);

    sIsMapLoaded = false;

    getSecretCourseStart(*gpMarDirector, sSecretCourseStart);
    sSecretCourseStartPlatform = sSecretCourseStart;
    sSecretCourseCurrentPos    = sSecretCourseStart;
    sSecretCourseLastPos       = sSecretCourseCurrentPos;
    sSecretCourseCurrentDir    = sSecretCourseCurrentPos;
    sSecretCourseCurrentDir.negate();
    sSecretCourseCurrentDir.normalize();

    sSecretCourseObjectsLoaded  = 0;
    sSecretCourseSwitchesLoaded = 0;

    sSecretCourseVertical = Randomizer::tryChance(30.0f);

    sGelatoWarp = nullptr;

    resetStageWarpInfo();
    resetHitHideObjs();
}

static void setMapLoaded(TMapCollisionStatic *staticCol) {
    staticCol->setUp();
    sIsMapLoaded = true;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801896E0, 0, 0, 0), setMapLoaded);

#define STR_EQUAL(a, b) strcmp(a, b) == 0

static f32 getAreaOfTriangle(const TVectorTriangle &triangle) {
    const f32 lengthA = PSVECDistance(triangle.a, triangle.b);
    const f32 lengthB = PSVECDistance(triangle.b, triangle.c);
    const f32 lengthC = PSVECDistance(triangle.c, triangle.a);

    const f32 sP = (lengthA + lengthB + lengthC) / 2.0f;
    return sqrtf(sP * (sP - lengthA) * (sP - lengthB) * (sP - lengthC));
}

static f32 getFromGroundHeight(const HitActorInfo &actorInfo) {
    if (actorInfo.mIsItemObj && Randomizer::tryChance(5.0f)) {
        return actorInfo.mFromSurfaceDist + 3000.0f;
    }
    return actorInfo.mFromSurfaceDist;
}

static void rotateWithNormal(const TVec3f &normal, TVec3f &out) {
    Mtx mtx;

    {
        const TVec3f up     = TVec3f::up();
        const TVec3f origin = TVec3f::zero();
        const TVec3f look   = {-normal.x, -normal.y, -normal.z};
        C_MTXLookAt(mtx, origin, up, look);
    }


    out.setRotation(mtx);
}

static size_t collectFloorsAtXZ(const TMapCollisionData &collision, f32 x, f32 z, bool groundValid,
                                bool waterValid, size_t capacity, const TBGCheckData **out,
                                f32 *yOut) {
    size_t found = 0;

    f32 checkY = 100000.0f;
    while (found < capacity) {
        const TBGCheckData *tmpOut;
        const f32 groundY = collision.checkGround(x, checkY, z, 4, &tmpOut);

        checkY = groundY - 100.0f;

        if (tmpOut->mType == 1536 || tmpOut->mType == 2048)
            return found;

        if (!waterValid &&
            (tmpOut->isWaterSurface() || tmpOut->mType == 267 || tmpOut->mType == 33035))
            return found;

        if (!groundValid &&
            !(tmpOut->isWaterSurface() || tmpOut->mType == 267 || tmpOut->mType == 33035))
            return found;

        out[found]  = tmpOut;
        yOut[found] = groundY;
        found += 1;
    }

    return found;
}

static size_t collectTrisAtBlock(TBGCheckList *list, size_t max, const TBGCheckData **out,
                                 u16 ignoreType) {
    size_t wallsFound = 0;
    while (list && list->mColTriangle && wallsFound < max) {
        if (list->mColTriangle->mType != ignoreType)
            out[wallsFound++] = list->mColTriangle;
        list = list->mNextTriangle;
    }
    return wallsFound;
}

bool isContextMakeSecretCourse(const TMarDirector &director) {
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

void Randomizer::getRandomizedPointOnTriangle(TVec3f &out, const TVectorTriangle &triangle) {
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

void Randomizer::getRandomizedPosition(TVec3f &out, const TMapCollisionData &collision) {
    const f32 boundsX = collision.mAreaSizeX;
    const f32 boundsZ = collision.mAreaSizeZ;

    f32 rX     = boundsX * sqrtf(randLerp());
    f32 thetaX = sqrtf(randLerp()) * 2 * M_PI;

    f32 rZ     = boundsZ * sqrtf(randLerp());
    f32 thetaZ = sqrtf(randLerp()) * 2 * M_PI;

    out.x = rX * cosf(thetaX);
    out.z = rZ * sinf(thetaZ);

    const TBGCheckData *fout;
    out.y = collision.checkGround(out.x, 100000.0f, out.z, 0, &fout);
}

void Randomizer::getRandomizedRotation(TVec3f &out, const HitActorInfo &actorInfo) {
    if (actorInfo.mShouldRotateXZ) {
        out.x = lerp<f32>(0.0f, 360.0f, randLerp()) + actorInfo.mAdjustRotation.x;
        out.z = lerp<f32>(0.0f, 360.0f, randLerp()) + actorInfo.mAdjustRotation.z;
    }

    if (actorInfo.mShouldRotateY) {
        out.y = lerp<f32>(0.0f, 360.0f, randLerp()) + actorInfo.mAdjustRotation.y;
    }
}

void Randomizer::getRandomizedScale(TVec3f &out, const HitActorInfo &actorInfo) {
    if (!Randomizer::isRandomScale())
        return;

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

namespace Randomizer {
    BaseSolver::BaseSolver(THitActor *actor) : mActor(actor) {
        mInfo             = getRandomizerInfo(actor);
        mInfo.mObjectKey = mActor->mKeyName;
        initializeDefaultActorInfo(*gpMarDirector, mInfo);
    }

    void BaseSolver::setTarget(THitActor *actor) {
        mActor            = actor;
        mInfo             = getRandomizerInfo(actor);
        mInfo.mObjectKey = mActor->mKeyName;
        initializeDefaultActorInfo(*gpMarDirector, mInfo);
    }

    bool BaseSolver::isContextValid() const {
        if (!mInfo.mShouldRandomize)
            return false;

        if (STR_EQUAL(mActor->mKeyName, "randomizer_off"))
            return false;

        return true;
    }

    void BaseSolver::sampleRandomFloor(TMapCollisionData &collision, TVec3f &outT, TVec3f &outR,
                                       TVec3f &outS) {
        const bool isUnderwaterValid = mInfo.mIsUnderwaterValid;
        const bool isWaterValid      = mInfo.mIsWaterValid;
        const bool isGroundValid     = mInfo.mIsGroundValid;

        constexpr size_t MaxFloors = 8;
        const TBGCheckData *floors[MaxFloors];
        f32 groundY[MaxFloors];

        for (size_t i = 0; i < getSampleMax(); ++i) {
            getRandomizedPosition(outT, collision);

            const size_t floorsFound =
                collectFloorsAtXZ(collision, outT.x, outT.z, isGroundValid, isWaterValid,
                                  MaxFloors, floors, groundY);

            if (floorsFound == 0)
                continue;

            const int decidedFloor =
                floorsFound > 1 ? lerp<f32>(0.0f, floorsFound - 0.01f, randLerp()) : 0;
            const TBGCheckData *floor = floors[decidedFloor];
            outT.y                  = groundY[decidedFloor];

            adjustSampledFloor(outT, *floor);

            if (!isSampledFloorValid(outT, *floor))
                continue;

            if (mInfo.mIsSurfaceBound) {
                rotateWithNormal(floor->mNormal, outR);

                outR.x += mInfo.mAdjustRotation.x;
                outR.y += mInfo.mAdjustRotation.y;
                outR.z += mInfo.mAdjustRotation.z;
            } else {
                getRandomizedRotation(outR, mInfo);
            }

            getRandomizedScale(outS, mInfo);
            break;
        }
    }

    void BaseSolver::sampleRandomWall(TMapCollisionData &collision, TVec3f &outT, TVec3f &outR,
                                      TVec3f &outS) {
        constexpr f32 gridFraction = 1.0f / 1024.0f;

        const TBGCheckData *staticWalls[256];

        for (size_t i = 0; i < getSampleMax(); ++i) {
            size_t staticFound = 0;
            while (staticFound == 0) {
                getRandomizedPosition(outT, collision);

                const f32 boundsX = collision.mAreaSizeX;
                const f32 boundsZ = collision.mAreaSizeZ;

                /* Sample a block from the grid to pull walls from */
                const auto blockIdxX = static_cast<int>(gridFraction * (outT.x + boundsX));
                const auto blockIdxZ = static_cast<int>(gridFraction * (outT.z + boundsZ));

                staticFound = collectTrisAtBlock(
                    collision.mStaticCollisionRoot[blockIdxX + (blockIdxZ * collision.mBlockXCount)]
                        .mCheckList[TBGCheckListRoot::WALL]
                        .mNextTriangle,
                    256, staticWalls, 5);
            }

            const int decidedWall =
                staticFound > 1 ? lerp<f32>(0.0f, staticFound - 0.01f, randLerp()) : 0;
            const TBGCheckData *wall = staticWalls[decidedWall];

            TVectorTriangle triangle(wall->mVertices[0], wall->mVertices[1], wall->mVertices[2]);
            getRandomizedPointOnTriangle(outT, triangle);

            const TBGCheckData *floor;
            f32 groundY = collision.checkGround(outT.x, outT.y, outT.z, 1, &floor);

            if (floor->mType == 1536 || floor->mType == 2048) {
                if (outT.y - groundY < 800.0f)
                    continue;
            } else {
                if (outT.y - groundY < 200.0f)
                    continue;
            }

            if (!isSampledWallValid(outT, *wall))
                continue;

            TVec3f scaledNormal = wall->mNormal;
            scaledNormal.scale(mInfo.mFromSurfaceDist + 1.0f);
            outT.add(scaledNormal);

            if (mInfo.mIsSurfaceBound) {
                rotateWithNormal(wall->mNormal, outR);
                outR.x += mInfo.mAdjustRotation.x;
                outR.y += mInfo.mAdjustRotation.y;
                outR.z += mInfo.mAdjustRotation.z;
            } else {
                getRandomizedRotation(outR, mInfo);
            }

            getRandomizedScale(outS, mInfo);
            break;
        }
    }

    void BaseSolver::sampleRandomRoof(TMapCollisionData &collision, TVec3f &outT, TVec3f &outR,
                                      TVec3f &outS) {
        constexpr f32 gridFraction = 1.0f / 1024.0f;

        const TBGCheckData *staticRoofs[256];

        for (size_t i = 0; i < getSampleMax(); ++i) {
            size_t staticFound = 0;
            while (staticFound == 0) {
                getRandomizedPosition(outT, collision);

                const f32 boundsX = collision.mAreaSizeX;
                const f32 boundsZ = collision.mAreaSizeZ;

                /* Sample a block from the grid to pull walls from */
                const auto blockIdxX = static_cast<int>(gridFraction * (outT.x + boundsX));
                const auto blockIdxZ = static_cast<int>(gridFraction * (outT.z + boundsZ));

                staticFound = collectTrisAtBlock(
                    collision.mStaticCollisionRoot[blockIdxX + (blockIdxZ * collision.mBlockXCount)]
                        .mCheckList[TBGCheckListRoot::ROOF]
                        .mNextTriangle,
                    256, staticRoofs, 5);
            }

            const int decidedRoof =
                staticFound > 1 ? lerp<f32>(0.0f, staticFound - 0.01f, randLerp()) : 0;
            const TBGCheckData *roof = staticRoofs[decidedRoof];

            TVectorTriangle triangle(roof->mVertices[0], roof->mVertices[1], roof->mVertices[2]);
            getRandomizedPointOnTriangle(outT, triangle);

            const TBGCheckData *floor;
            f32 groundY = collision.checkGround(outT.x, outT.y, outT.z, 1, &floor);

            if (outT.y - groundY < 200.0f)
               continue;

            if (!isSampledRoofValid(outT, *roof))
                continue;

            TVec3f scaledNormal = roof->mNormal;
            scaledNormal.scale(mInfo.mFromSurfaceDist + 1.0f);
            outT.add(scaledNormal);

            if (mInfo.mIsSurfaceBound) {
                rotateWithNormal(roof->mNormal, outR);
                outR.x += mInfo.mAdjustRotation.x;
                outR.y += mInfo.mAdjustRotation.y;
                outR.z += mInfo.mAdjustRotation.z;
            } else {
                getRandomizedRotation(outR, mInfo);
            }

            getRandomizedScale(outS, mInfo);
            break;
        }
    }

    void BaseSolver::sampleRandomSky(TMapCollisionData &collision, TVec3f &outT, TVec3f &outR,
                                     TVec3f &outS) {
        for (size_t i = 0; i < getSampleMax(); ++i) {
            const f32 fromGroundHeight = getFromGroundHeight(mInfo);
            getRandomizedPosition(outT, collision);

            f32 groundY;
            const TBGCheckData *floor;
            const size_t floorsFound =
                collectFloorsAtXZ(collision, outT.x, outT.z, true, true, 1, &floor, &groundY);

            if (floorsFound == 0)
                continue;

            outT.y = groundY;

            if (!isSampledFloorValid(outT, *floor))
                continue;

            getRandomizedRotation(outR, mInfo);
            getRandomizedScale(outS, mInfo);

            outT.y += lerp(200.0f, 3000.0f, randLerp());
            break;
        }
    }

    bool SMSSolver::isContextValid() const {
        if (!BaseSolver::isContextValid())
            return false;

        if (SMS_isExMap__Fv()) {
            if (!Randomizer::isRandomExStage())
                return false;

            constexpr const char *SkyIslandKey = "\x8B\xF3\x93\x87";
            if (STR_EQUAL(mActor->mKeyName, SkyIslandKey)) {
                sSecretCourseStartPlatform = mActor->mTranslation;
                return false;
            }
        }

        switch (gpMarDirector->mAreaID) {
        case 1: {  // Delfino Plaza
            if (STR_EQUAL(mActor->mKeyName, "efDokanGate 0"))
                return false;
            if (STR_EQUAL(mActor->mKeyName, "efDokanGate 1"))
                return false;
        }
        case 4: {  // Gelato Beach
            if (STR_EQUAL(mActor->mKeyName, "ShiningStone"))
                return false;
            if (STR_EQUAL(mActor->mKeyName, "coral00"))
                return false;
            if (STR_EQUAL(mActor->mKeyName, "coral01"))
                return false;
            return true;
        }
        case 6: {  // Sirena Beach
            constexpr const char *hutKey =
                "\x83\x56\x83\x8C\x83\x69\x83\x56\x83\x87\x83\x62\x83\x76";
            if (STR_EQUAL(mActor->mKeyName, hutKey))
                return false;
            return true;
        }
        case 9:                            // Noki Bay
            if (gpMarDirector->mEpisodeID == 2)  // Bottle mission entry
                return false;
            return true;
        case 13:                                 // Pinna Park
            if (gpMarDirector->mEpisodeID == 6)  // Talk to guy before fight
                return false;
            return true;
        case 15:  // File Select
            if (gpMarDirector->mEpisodeID == 0)
                return false;
            return true;
        case 33:  // Sand Bird
            if (gpMarDirector->mEpisodeID == 0)
                return false;
            return true;
        case 60:
            if (gpMarDirector->mEpisodeID == 0)
                return false;
            return true;
        default:
            return true;
        }
    }

    bool SMSSolver::solve(TMapCollisionData &collision) {
        if (gpMarDirector->mAreaID == 4) {  // Collect Castle Warp
            if (STR_EQUAL(mInfo.mObjectKey, "\x83\x58\x83\x65\x81\x5B\x83\x57\x90\xD8\x91\xD6"
                                                 "\x81\x69\x8D\xBB\x82\xCC\x8F\xE9\x81\x6A")) {
                sGelatoWarp = mActor;
            }
        }

        if (!mInfo.mShouldRandomize)
            return false;

        if (!isContextValid())
            return false;

        if (gpMarDirector->mAreaID == 1) {  // Delfino Plaza
            if (strcmp(mActor->mKeyName, "GateToRicco") == 0) {
                mInfo.mFromSurfaceDist = 100.0f;
            }
        }

        const u32 gameSeed = Randomizer::getGameSeed();
        u32 key            = (mActor->mKeyCode * 0x41C64E6D + 0x3039) ^ gameSeed;

        if (sSecretCourseFluddless)
            key *= 0x51678391;

        Randomizer::srand32(key);

        {
            const bool isEntity =
                ((mInfo.mIsItemObj || mInfo.mIsPlayer) && !mInfo.mIsShineObj);

            if (isContextMakeSecretCourse(*gpMarDirector) && !isEntity)
                solveExStageObject(*gpMapCollisionData);
            else
                solveStageObject(*gpMapCollisionData);
        }

        if (STR_EQUAL(mInfo.mObjectType, "WaterHitHideObj") ||
            STR_EQUAL(mInfo.mObjectType, "WaterHitPictureHideObj")) {
            addHitHideObj(reinterpret_cast<TMapObjBase *>(mActor));
        }

        if (STR_EQUAL(mActor->mKeyName, "normalvariant3") ||
            (STR_EQUAL(mActor->mKeyName, "WoodBlockLarge 0") && gpMarDirector->mAreaID == 32)) {
            sSecretCourseShinePos = mActor->mTranslation;
        }

        if (gpMarDirector->mAreaID == 4) {
            if (STR_EQUAL(mInfo.mObjectType, "SandCastle") && sGelatoWarp) {
                sGelatoWarp->mTranslation = mActor->mTranslation;
                sGelatoWarp->mRotation = mActor->mRotation;
            }
        }

        Randomizer::srand32(gameSeed);

        return true;
    }

    void SMSSolver::adjustSampledFloor(TVec3f &sampledPos, const TBGCheckData &floor) {
        if (floor.isWaterSurface() || floor.mType == 267 || floor.mType == 33035) {
            const TBGCheckData *belowWater;
            gpMapCollisionData->checkGround(sampledPos.x, sampledPos.y - 10.0f, sampledPos.z, 0,
                                            &belowWater);

            // Below the water is death floor, therefore count as OOB
            if (belowWater->mType == 1536 || belowWater->mType == 2048)
                return;

            const TBGCheckData *data;
            switch (gpMarDirector->mAreaID) {
            case 52: {  // Corona mountain
                if (mInfo.mIsItemObj) {
                    sampledPos.y +=
                        mInfo.mFromSurfaceDist + 400.0f;  // Adjust to make it not touch lava
                    return;
                }

                if (mInfo.mIsUnderwaterValid && tryChance(70.0f)) {
                    const f32 belowGroundY = gpMapCollisionData->checkGround(
                        sampledPos.x, sampledPos.y - 10.0f, sampledPos.z, 0, &data);
                    sampledPos.y = lerp<f32>(belowGroundY + mInfo.mFromSurfaceDist,
                                             sampledPos.y - 10.0f, randLerp());
                    return;
                }

                sampledPos.y += getFromGroundHeight(mInfo);
                return;
            }
            default: {
                if (mInfo.mIsUnderwaterValid && tryChance(70.0f)) {
                    const f32 belowGroundY = gpMapCollisionData->checkGround(
                        sampledPos.x, sampledPos.y - 10.0f, sampledPos.z, 0, &data);
                    sampledPos.y = lerp<f32>(belowGroundY + mInfo.mFromSurfaceDist,
                                             sampledPos.y - 10.0f, randLerp());
                    return;
                }
                sampledPos.y += getFromGroundHeight(mInfo);
                return;
            }
            }
        }

        sampledPos.y += getFromGroundHeight(mInfo);
    }

    void SMSSolver::adjustSampledWall(TVec3f &sampledPos, const TBGCheckData &wall) {}
    void SMSSolver::adjustSampledRoof(TVec3f &sampledPos, const TBGCheckData &roof) {}
    void SMSSolver::adjustSampledSky(TVec3f &sampledPos) {}

    bool SMSSolver::isSampledFloorValid(TVec3f &sampledPos, const TBGCheckData &floor) {
        const u16 type = floor.mType & 0xFFF;

        // Filter out warp and OOB collision
        if (type == 768 || type == 1536 || type == 2048)
            return false;

        if (mInfo.mIsPlayer) {
            u32 polType = gpPollution->getPollutionType(sampledPos.x, sampledPos.y, sampledPos.z);
            if (polType == POLLUTION_FIRE || polType == POLLUTION_ELEC ||
                polType == POLLUTION_BURN || polType == POLLUTION_SINK_DIE) {
                if (gpPollution->isPolluted(sampledPos.x, sampledPos.y, sampledPos.z)) {
                    return false;
                }
            }
        }

        switch (gpMarDirector->mAreaID) {
        case 0:
        case 20: {  // Airstrip
            // Prevent invisible room spawns
            if (sampledPos.y > 5000.0f)
                return false;

            // Prevent OOB spawns
            const TVec3f center{800.0f, 0.0f, -2000.0f};
            const TVec3f diff{sampledPos.x, 0.0f, sampledPos.z};
            return PSVECDistance(diff, center) < 15000.0f;
        }
        case 2: {  // Bianco Hills
            // Prevent OOB slope spawns
            if (sampledPos.y < 1700.0f && !floor.isWaterSurface())
                return false;

            // This bounding check fixes stuff spawning OOB near hillside
            if (sampledPos.z > -7000.0f && sampledPos.z < 2200.0f) {
                if (sampledPos.x > 16000.0f && sampledPos.y > 3000.0f) {
                    return false;
                }
            }

            // This bounding check fixes stuff spawning OOB near brick walls
            if (sampledPos.x > 10000.0f && sampledPos.y > 3800.0f && sampledPos.z > 7800.0f) {
                return false;
            }

            return true;
        }
        case 3: {  // Ricco Harbor
            // Prevent rising water bad spawns
            if (sampledPos.y < 0.0f && !floor.isWaterSurface())
                return false;
            return true;
        }
        case 4: {  // Gelato Beach
            if (STR_EQUAL(mInfo.mObjectType, "SandCastle")) {
                if (floor.mType != 0x8701 || sampledPos.y > 300.0f)  // Beach Sand
                    return false;
                return true;
            }
        }
        case 5: {  // Pinna Beach
            // Prevent behind door bad spawn
            if (sampledPos.z < 2200.0f) {
                if (sampledPos.x > -5200.0f && sampledPos.x < -1800.0f) {
                    return false;
                }
            }

            // Prevent left ocean bad spawn
            if (sampledPos.z < 0.0f && sampledPos.x < -6900.0f)
                return false;

            // Prevent right ocean bad spawn
            if (sampledPos.z > 8600.0f || sampledPos.x > 17000.0f)
                return false;

            return true;
        }
        case 6: {  // Sirena Beach
            // Prevent ocean bad spawn
            if (sampledPos.z > 23000.0f)
                return false;
            if (sampledPos.x < -8000.0f || sampledPos.x > 8600.0f)
                return false;
            return true;
        }
        case 8: {  // Pianta Village
            if (sampledPos.y < -650.0f) {
                if ((floor.mType & 0xFFF) == 10 && mInfo.mIsItemObj)
                    return true;
                return false;
            }
            return true;
        }
        case 9: {  // Noki Bay
            // Prevent high ground OOB spawn
            if (sampledPos.x > -300.0f && sampledPos.y > 12000.0f && sampledPos.z > 1500.0f)
                return false;
            if (sampledPos.y < 0.0f && !floor.isWaterSurface())
                return false;
            return true;
        }
        case 13: {  // Pinna Park
            // Prevent behind ferris wheel bad spawn
            if (sampledPos.x > 10000.0f && sampledPos.z > -7500.0f)
                return false;
            return true;
        }
        case 30: {  // Blooper surfing secret
            // Prevent OOB floor spawns
            if (sampledPos.y < -1000.0f)
                return false;
            return true;
        }
        case 44:  // Bottle
            // Prevent OOB spawns
            if (PSVECMag(sampledPos) > 3300.0f)
                return false;
            if (sampledPos.y > -4000.0f)
                return false;
            return true;
        case 52: {  // Corona mountain
            // Prevent OOB water spawns
            if (sampledPos.x < -2500.0f || sampledPos.x > 2500.0f)
                return false;
            return true;
        }
        case 55: {  // Bianco Hills
            // Prevent Petey windmill OOB
            return PSVECMag(sampledPos) < 1800.0f;
        }
        default:
            return true;
        }
    }

    bool SMSSolver::isSampledWallValid(TVec3f &sampledPos, const TBGCheckData &wall) {
        // Filter out warp collision
        if ((wall.mType & 0xFFF) == 5)
            return false;

        if (mInfo.mIsSprayableObj && sampledPos.y < 100.0f)
            return false;

        const f32 wallArea =
            getAreaOfTriangle({wall.mVertices[0], wall.mVertices[1], wall.mVertices[2]});

        OSReport("Area of triangle (%.02f cm)\n", wallArea);

        if (wallArea < 50000.0f)
            return false;

        switch (gpMarDirector->mAreaID) {
        case 0: {  // Airstrip
            // Prevent invisible room spawns
            if (sampledPos.y > 5000.0f)
                return false;
            return true;
        }
        case 1: {  // Delfino Plaza
            if (sampledPos.y > 3000.0f && sampledPos.z < -8200.0f)
                return false;
            if (sampledPos.y <= 0.0f)
                return false;
            return true;
        }
        case 2: {  // Bianco Hills
            return true;
        }
        case 3: {  // Ricco Harbor
            if (sampledPos.x > -2000.0f)
                return sampledPos.y < 4600.0f;
            return sampledPos.y < 3600.0f;
        }
        case 5: {  // Pinna Beach
            return true;
        }
        case 6: {  // Sirena Beach
            if (sampledPos.y > 3000.0f)
                return false;
            return true;
        }
        case 8: {  // Pianta Village
            // Prevent cheap trunk spawns
            if (sampledPos.y < -650.0f)
                return false;
            return true;
        }
        case 9: {  // Noki Bay
            // Prevent high ground OOB spawn
            if (sampledPos.y > 12000.0f)
                return false;
            return true;
        }
        case 13: {  // Pinna Park
            // Prevent behind ferris wheel bad spawn
            if (sampledPos.x > 10000.0f && sampledPos.z > -7500.0f)
                return false;
            return true;
        }
        case 24: {  // Lily Pad Secret
            return sampledPos.y < 0.0f;
        }
        case 30: {  // Blooper surfing secret
            // Prevent OOB floor spawns
            if (sampledPos.y < 0.0f)
                return false;
            return true;
        }
        case 44:  // Bottle
            // Prevent OOB spawns
            if (sampledPos.y > -4000.0f)
                return false;
            return true;
        case 52: {  // Corona mountain
            return true;
        }
        default:
            return true;
        }
    }

    bool SMSSolver::isSampledRoofValid(TVec3f &sampledPos, const TBGCheckData &roof) {
        if (sampledPos.y > 100000.0f)
            return false;

        if (mInfo.mIsSprayableObj && sampledPos.y <= 0.0f)
            return false;

        switch (gpMarDirector->mAreaID) {
        case 0: {  // Airstrip
            // Prevent invisible room spawns
            if (sampledPos.y > 5000.0f)
                return false;
            return true;
        }
        case 1: {  // Delfino Plaza
            if (sampledPos.y > 3000.0f && sampledPos.z < -8200.0f)
                return false;
            if (sampledPos.y <= 0.0f)
                return false;
            return true;
        }
        case 2: {  // Bianco Hills
            return true;
        }
        case 3: {  // Ricco Harbor
            if (sampledPos.x > -2000.0f)
                return sampledPos.y < 4600.0f;
            return sampledPos.y < 3600.0f;
        }
        case 5: {  // Pinna Beach
            return true;
        }
        case 6: {  // Sirena Beach
            if (sampledPos.y > 3000.0f)
                return false;
            return true;
        }
        case 8: {  // Pianta Village
            // Prevent cheap trunk spawns
            if (sampledPos.y < -650.0f)
                return false;
            return true;
        }
        case 9: {  // Noki Bay
            // Prevent high ground OOB spawn
            if (sampledPos.y > 12000.0f)
                return false;
            return true;
        }
        case 13: {  // Pinna Park
            // Prevent behind ferris wheel bad spawn
            if (sampledPos.x > 10000.0f && sampledPos.z > -7500.0f)
                return false;
            return true;
        }
        case 24: {  // Lily Pad Secret
            return sampledPos.y < 0.0f;
        }
        case 30: {  // Blooper surfing secret
            // Prevent OOB floor spawns
            if (sampledPos.y < 0.0f)
                return false;
            return true;
        }
        case 44:  // Bottle
            // Prevent OOB spawns
            if (sampledPos.y > -4000.0f)
                return false;
            return true;
        case 52: {  // Corona mountain
            return true;
        }
        default:
            return true;
        }
    }

    
    void SMSSolver::solveStageObject(TMapCollisionData &collision) {
        PlaneSelection selection = PlaneSelection::SKY;

        if (mInfo.mIsGroundValid || mInfo.mIsWaterValid || mInfo.mIsUnderwaterValid) {
            selection = PlaneSelection::FLOOR;
            if (mInfo.mIsWallValid && tryChance(50.0f))
                selection = PlaneSelection::WALL;
            if (mInfo.mIsRoofValid && tryChance(25.0f))
                selection = PlaneSelection::ROOF;
        } else if (mInfo.mIsWallValid) {
            selection = PlaneSelection::WALL;
            if (mInfo.mIsRoofValid && tryChance(35.0f))
                selection = PlaneSelection::ROOF;
        } else if (mInfo.mIsRoofValid) {
            selection = PlaneSelection::ROOF;
        }

        switch (selection) {
        default:
        case PlaneSelection::FLOOR:
            sampleRandomFloor(collision, mActor->mTranslation, mActor->mRotation, mActor->mScale);
            break;
        case PlaneSelection::ROOF:
            sampleRandomRoof(collision, mActor->mTranslation, mActor->mRotation, mActor->mScale);
            break;
        case PlaneSelection::WALL:
            sampleRandomWall(collision, mActor->mTranslation, mActor->mRotation, mActor->mScale);
            break;
        case PlaneSelection::SKY:
            sampleRandomSky(collision, mActor->mTranslation, mActor->mRotation, mActor->mScale);
            break;
        }

        OSReport("Sampled position: {%.02f, %.02f, %.02f}\n", mActor->mTranslation.x, mActor->mTranslation.y, mActor->mTranslation.z);
    }

    void SMSSolver::solveExStageObject(TMapCollisionData &collision) {
        TVec3f outT;
        TVec3f outR;
        TVec3f outS;

        getRandomizedScale(outS, mInfo);

        if (mInfo.mIsShineObj) {
            const TBGCheckData *roof;
            const f32 roofHeight =
                collision.checkRoof(sSecretCourseShinePos.x, sSecretCourseShinePos.y + 200.0f,
                                    sSecretCourseShinePos.z, 0, &roof);

            outT.x = sSecretCourseShinePos.x;
            outT.y = Min(roofHeight - 100.0f,
                           sSecretCourseShinePos.y + mInfo.mFromSurfaceDist + 200.0f);
            outT.z = sSecretCourseShinePos.z;

            sSecretCourseShinePos.x += 200.0f;

            getRandomizedRotation(outR, mInfo);

            mActor->mTranslation = outT;
            mActor->mRotation = outR;
            mActor->mScale     = outS;
            return;
        }

        if (mInfo.mIsSwitchObj) {
            switch (sSecretCourseSwitchesLoaded) {
            default:
            case 0:
                outT = sSecretCourseStartPlatform;
                outT.x += 500.0f;
                break;
            case 1:
                outT = sSecretCourseStartPlatform;
                outT.x -= 500.0f;
                break;
            case 2:
                outT = sSecretCourseStartPlatform;
                outT.z += 500.0f;
                break;
            case 3:
                outT = sSecretCourseStartPlatform;
                outT.z -= 500.0f;
                break;
            }

            mActor->mTranslation = outT;
            mActor->mRotation = outR;
            mActor->mScale = outS;
            return;
        }

        if (mInfo.mIsExLinear) {
            Mtx rot;

            TVec3f pprevObjPos = sSecretCourseLastPos;
            TVec3f prevObjPos  = sSecretCourseCurrentPos;

            do {
                float xzDistance = 0;
                do {
                    const f32 theta = 2.0f * M_PI * randLerp();
                    const f32 phi   = acosf(2.0f * randLerp() - 1.0f);

                    f32 horizontalRadius;
                    f32 verticalRadius;
                    if (sSecretCourseFluddless) {
                        horizontalRadius = sSecretCourseVertical ? 1400.0f : 2000.0f;
                        verticalRadius   = sSecretCourseVertical ? 800.0f : 1300.0f;
                    } else {
                        horizontalRadius = sSecretCourseVertical ? 2000.0f : 2500.0f;
                        verticalRadius   = sSecretCourseVertical ? 1400.0f : 1200.0f;
                    }

                    const f32 adjustX = horizontalRadius * outS.x * sinf(phi) * cosf(theta);
                    const f32 adjustY = verticalRadius * outS.y * sinf(phi) * sinf(theta) + 100.0f;
                    const f32 adjustZ = horizontalRadius * outS.z * cosf(phi);

                    outT.x = prevObjPos.x + adjustX;
                    outT.y = prevObjPos.y + adjustY;
                    outT.z = prevObjPos.z + adjustZ;

                    xzDistance = sqrtf(adjustX * adjustX + adjustZ * adjustZ);
                } while (xzDistance < (sSecretCourseVertical ? 700.0f : 900.0f));


                if (outT.y < 500.0f) {
                    outT.y = 500.0f + (500.0f - outT.y);
                }

            } while (PSVECDistance(prevObjPos, outT) > PSVECDistance(pprevObjPos, outT));

            sSecretCourseLastPos    = sSecretCourseCurrentPos;
            sSecretCourseCurrentPos = outT;

            {
                const TVec3f up     = TVec3f::up();
                const TVec3f origin = prevObjPos;
                C_MTXLookAt(rot, origin, up, outT);
            }

            PSMTXInverse(rot, rot);
            outR.setRotation(rot);

            mActor->mTranslation = outT;
            mActor->mRotation = outR;
            mActor->mScale     = outS;
            return;
        }
        outT.y = 0.0f;

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

        const f32 adjustX = horizontalRadius * outS.x * sinf(phi) * cosf(theta);
        const f32 adjustY = verticalRadius * outS.y * sinf(phi) * sinf(theta) +
                            (sSecretCourseVertical ? 50.0f : 0.0f);
        const f32 adjustZ = horizontalRadius * outS.z * cosf(phi);

        outT.x = sSecretCourseCurrentPos.x + adjustX;
        outT.y = sSecretCourseCurrentPos.y + adjustY;
        outT.z = sSecretCourseCurrentPos.z + adjustZ;

        if (outT.y < 500.0f) {
            outT.y = 500.0f + (500.0f - outT.y);
        }

        sSecretCourseLastPos    = sSecretCourseCurrentPos;
        sSecretCourseCurrentPos = outT;

        getRandomizedRotation(outR, mInfo);

        mActor->mTranslation = outT;
        mActor->mRotation = outR;
        mActor->mScale     = outS;
    }
}

#undef STR_EQUAL