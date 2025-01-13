#include "p_common.hxx"

bool sIsMapLoaded;
u32 sStageSeed;

TGlobalVector<u16> sWarpIDWhiteList;
TGlobalUnorderedMap<u16, Randomizer::ISolver *> sSolvers;
Randomizer::ISolver *sDefaultSolver = nullptr;

TVec3f sSecretCourseStart          = TVec3f::zero();
TVec3f sSecretCourseStartPlatform  = TVec3f::zero();
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

}  // namespace Randomizer
