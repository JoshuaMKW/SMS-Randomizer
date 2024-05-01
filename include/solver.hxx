#pragma once

#include <Dolphin/types.h>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRDisplay.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <BetterSMS/libs/global_vector.hxx>

#include "actorinfo.hxx"
#include <BetterSMS/libs/triangle.hxx>

namespace Randomizer {
    const TGlobalVector<u16> &getWarpIDWhiteList();

    class BaseSolver {
    public:
        enum class PlaneSelection { FLOOR, ROOF, WALL, SKY };

        BaseSolver() = default;
        BaseSolver(THitActor *actor);

        void setTarget(THitActor *actor);
        const HitActorInfo &getInfo() const { return mInfo; }

        virtual size_t getSampleMax() const { return 100; }

        virtual void init(TMarDirector *director) {}
        virtual bool solve(TMapCollisionData &collision) = 0;

        virtual void adjustSampledFloor(TVec3f &sampledPos, const TBGCheckData &floor) {}
        virtual void adjustSampledWall(TVec3f &sampledPos, const TBGCheckData &wall) {}
        virtual void adjustSampledRoof(TVec3f &sampledPos, const TBGCheckData &roof) {}
        virtual void adjustSampledSky(TVec3f &sampledPos) {}

        virtual bool isContextValid() const;

        virtual bool isSampledFloorValid(TVec3f &sampledPos, const TBGCheckData &floor) {
            return true;
        }

        virtual bool isSampledWallValid(TVec3f &sampledPos, const TBGCheckData &wall) {
            return true;
        }

        virtual bool isSampledRoofValid(TVec3f &sampledPos, const TBGCheckData &roof) {
            return true;
        }

    protected:
        void sampleRandomFloor(TMapCollisionData &collision, TVec3f &outPos, TVec3f &outRot,
                               TVec3f &outScl);
        void sampleRandomWall(TMapCollisionData &collision, TVec3f &outPos, TVec3f &outRot,
                              TVec3f &outScl);
        void sampleRandomRoof(TMapCollisionData &collision, TVec3f &outPos, TVec3f &outRot,
                              TVec3f &outScl);
        void sampleRandomSky(TMapCollisionData &collision, TVec3f &outPos, TVec3f &outRot,
                             TVec3f &outScl);

        THitActor *mActor;
        HitActorInfo mInfo;
    };

    class SMSSolver : public BaseSolver {
    public:
        SMSSolver() = default;
        SMSSolver(THitActor *actor) : BaseSolver(actor) {}

        void init(TMarDirector *director) override;
        bool solve(TMapCollisionData &collision) override;

        void adjustSampledFloor(TVec3f &sampledPos, const TBGCheckData &floor) override;
        void adjustSampledWall(TVec3f &sampledPos, const TBGCheckData &wall) override;
        void adjustSampledRoof(TVec3f &sampledPos, const TBGCheckData &roof) override;
        void adjustSampledSky(TVec3f &sampledPos) override;

        bool isContextValid() const override;

        bool isSampledFloorValid(TVec3f &sampledPos, const TBGCheckData &floor) override;
        bool isSampledWallValid(TVec3f &sampledPos, const TBGCheckData &wall) override;
        bool isSampledRoofValid(TVec3f &sampledPos, const TBGCheckData &roof) override;

    protected:
        void solveStageObject(TMapCollisionData &collision);
        void solveExStageObject(TMapCollisionData &collision);
    };

    bool isWarpIDValid(u16 warpID);

    bool isSolverRegistered(u8 area, u8 episode);
    BaseSolver *getSolver(u8 area, u8 episode);

    bool registerSolver(u8 area, u8 episode, BaseSolver *solver);
    bool deregisterSolver(u8 area, u8 episode);
    bool setDefaultSolver(BaseSolver *solver);

    void getRandomizedPointOnTriangle(TVec3f &out, const TVectorTriangle &triangle);
    void getRandomizedPosition(TVec3f &out, const TMapCollisionData &collision);
    void getRandomizedRotation(TVec3f &out, const HitActorInfo &actorInfo);
    void getRandomizedScale(TVec3f &out, const HitActorInfo &actorInfo);
};  // namespace BetterSMS