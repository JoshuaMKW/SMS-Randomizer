#include "p_common.hxx"

#include <SMS/Camera/PolarSubCamera.hxx>

bool sIsMapLoaded;
u32 sStageSeed;

TGlobalVector<u16> sWarpIDWhiteList;
TGlobalUnorderedMap<u16, Randomizer::ISolver *> sSolvers;
Randomizer::ISolver *sDefaultSolver = nullptr;

TVec3f sSecretCourseStart          = TVec3f::zero();
TVec3f sSecretCourseStartPlatform  = TVec3f::zero();
TVec3f sSecretCourseEndPlatform    = TVec3f::zero();
TVec3f sSecretCourseCurrentPos     = TVec3f::zero();
TVec3f sSecretCourseLastPos        = TVec3f::zero();
TVec3f sSecretCourseCurrentRot     = TVec3f::zero();
TVec3f sSecretCourseLastRot        = TVec3f::zero();
TVec3f sSecretCourseCurrentDir     = TVec3f::zero();
TVec3f sSecretCourseShinePos       = TVec3f::zero();
size_t sSecretCourseObjectsLoaded  = 0;
size_t sSecretCourseSwitchesLoaded = 0;
bool sSecretCourseVertical         = false;
bool sSecretCourseFluddless        = false;
bool sSecretCourseHasEndPlatform   = false;

void initDefaultSolver(TApplication *app) {
    sDefaultSolver = new (JKRHeap::sRootHeap, 4) Randomizer::SMSSolver();
}

/*
/ API
*/
namespace Randomizer {

    const TGlobalVector<u16> &getWarpIDWhiteList() { return sWarpIDWhiteList; }

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
        return sSolvers.find(warpID) != sSolvers.begin();
    }

    ISolver *getSolver(u8 area, u8 episode) {
        const u16 warpID = ((area + 1) << 8) | episode;
        if (!isSolverRegistered(area, episode))
            return sDefaultSolver;
        return sSolvers[warpID];
    }

    bool registerSolver(u8 area, u8 episode, ISolver *solver) {
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

    bool setDefaultSolver(ISolver *solver) {
        if (!solver)
            return false;
        sDefaultSolver = solver;
        return true;
    }

    void SMSSolver::setTarget(JDrama::TActor *actor) {
        mActor           = actor;
        mInfo            = getRandomizerInfo(actor);
        mInfo.mObjectKey = mActor->mKeyName;
        initializeDefaultActorInfo(*gpMarDirector, mInfo);
    }

    bool SMSSolver::isContextMakeSecretCourse(const TMarDirector &director) const {
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

    bool SMSSolver::isSecretCourseSimple() const {
        switch (gpMarDirector->mAreaID) {
        case TGameSequence::AREA_BIANCOEX1:
            return true;
        case TGameSequence::AREA_COROEX0:
        case TGameSequence::AREA_COROEX1:
        case TGameSequence::AREA_MAMMAEX1:
        case TGameSequence::AREA_RICOEX0:
        case TGameSequence::AREA_SIRENAEX0:
            return true;
        case TGameSequence::AREA_COROEX2:
        case TGameSequence::AREA_COROEX4:
        case TGameSequence::AREA_COROEX5:
        case TGameSequence::AREA_MAMMAEX0:
        case TGameSequence::AREA_MONTEEX0:
        case TGameSequence::AREA_RICOEX1:
        case TGameSequence::AREA_SIRENAEX1:
            return false;
        }
        return false;
    }

    bool SMSSolver::solve(JDrama::TActor *actor, TMapCollisionData &collision) {
        setTarget(actor);

        if (!mInfo.mShouldRandomize)
            return true;

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
        Randomizer::srand64(diffuse_u64(key));

        {
            if (isContextMakeSecretCourse(*gpMarDirector)) {
                solveExStageObject(*gpMapCollisionData);
            } else {
                solveStageObject(*gpMapCollisionData);
            }
        }

        Randomizer::srand32(sStageSeed);
        Randomizer::srand64(diffuse_u64(sStageSeed));

        getRandomizerInfo(mActor) = mInfo;

        return true;
    }

#define FLT_MAX 3.402823466e+38F

    bool SMSSolver::solve(TGraphWeb *web, TMapCollisionData &collision) {
        BoundingBox web_bb;

        TVec3f min = {FLT_MAX, FLT_MAX, FLT_MAX};
        TVec3f max = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

        for (size_t i = 0; i < web->mNodeCount; ++i) {
            TVec3f pos = web->indexToPoint(i);
            min.x      = Min(min.x, pos.x);
            min.y      = Min(min.y, pos.y);
            min.z      = Min(min.z, pos.z);
            max.x      = Max(max.x, pos.x);
            max.y      = Max(max.y, pos.y);
            max.z      = Max(max.z, pos.z);
        }

        web_bb.center = {(min.x + max.x) / 2.0f, (min.y + max.y) / 2.0f, (min.z + max.z) / 2.0f};

        web_bb.size = {(max.x - min.x) / 2.0f, (max.y - min.y) / 2.0f, (max.z - min.z) / 2.0f};

        web_bb.rotation = {0.0f, 0.0f, 0.0f};

        // TODO: Implement web solver to understand randomized map state
        // probably have this run at the end of the stage generation
        solveGraphWeb(collision, web, web_bb);

        return true;
    }

    bool SMSSolver::postSolve(TMapCollisionData &collision) {
        if (SMS_isExMap__Fv()) {
            solveExPost(collision);
        }
        return true;
    }

    static u32 adjustCameraDemoForShine(f32 *camera) {
        TShine *shine;
        SMS_FROM_GPR(29, shine);

        f32 radius = 2500.0f;
        f32 angle = randLerp32() * M_PI;
        camera[3] = shine->mTranslation.x + cosf(angle) * radius;
        camera[4] = shine->mTranslation.y + sinf(M_PI / 4) * radius;
        camera[5] = shine->mTranslation.z + -sinf(angle) * radius;

        return 0x801C0000;
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x801BCF54, 0, 0, 0), adjustCameraDemoForShine);

}  // namespace Randomizer
