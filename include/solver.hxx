#pragma once

#include <BetterSMS/libs/boundbox.hxx>
#include <BetterSMS/libs/global_vector.hxx>

#include <Dolphin/types.h>
#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/JDrama/JDRDisplay.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/System/Application.hxx>

#include "actorinfo.hxx"
#include <BetterSMS/libs/triangle.hxx>

namespace Randomizer {

    const TGlobalVector<u16> &getWarpIDWhiteList();

    class ISolver {
    public:
        virtual ~ISolver() = default;

        virtual void init(TMarDirector *director)                               = 0;
        virtual bool solve(JDrama::TActor *actor, TMapCollisionData &collision) = 0;
        virtual bool solve(TGraphWeb *web, TMapCollisionData &collision)        = 0;
        virtual bool postSolve(TMapCollisionData &collision)                    = 0;
    };

    class SMSSolver : public ISolver {
    public:
        enum class PlaneSelection { FLOOR, ROOF, WALL, SKY };

    public:
        SMSSolver() = default;

        void init(TMarDirector *director) override;
        bool solve(JDrama::TActor *actor, TMapCollisionData &collision) override;
        bool solve(TGraphWeb *web, TMapCollisionData &collision) override;
        bool postSolve(TMapCollisionData &collision) override;

        void setTarget(JDrama::TActor *actor);
        virtual size_t getSampleMax() const { return SMS_isExMap__Fv() ? 100000 : 100; }

        virtual bool isSecretCourseSimple() const;

    protected:
        virtual bool isContextValid() const;
        virtual bool isContextMakeSecretCourse(const TMarDirector &director) const;

        virtual bool isSampledFloorValid(TVec3f &sampledPos, const TBGCheckData &floor);
        virtual bool isSampledWallValid(TVec3f &sampledPos, const TBGCheckData &wall);
        virtual bool isSampledRoofValid(TVec3f &sampledPos, const TBGCheckData &roof);

        virtual void adjustSampledFloor(TVec3f &sampledPos, const TBGCheckData &floor);
        virtual void adjustSampledWall(TVec3f &sampledPos, const TBGCheckData &wall);
        virtual void adjustSampledRoof(TVec3f &sampledPos, const TBGCheckData &roof);
        virtual void adjustSampledSky(TVec3f &sampledPos);

        bool isCurrentActorValid() const;
        void sampleRandomFloor(TMapCollisionData &collision, TVec3f &outPos, TVec3f &outRot,
                               TVec3f &outScl, bool forceTop = false);
        void sampleRandomWall(TMapCollisionData &collision, TVec3f &outPos, TVec3f &outRot,
                              TVec3f &outScl);
        void sampleRandomRoof(TMapCollisionData &collision, TVec3f &outPos, TVec3f &outRot,
                              TVec3f &outScl);
        void sampleRandomSky(TMapCollisionData &collision, TVec3f &outPos, TVec3f &outRot,
                             TVec3f &outScl);

        void sampleRandomTopFloor(TMapCollisionData &collision, TVec3f &outPos, TVec3f &outRot,
                                  TVec3f &outScl);

        void solveStageObject(TMapCollisionData &collision);
        void solveExStageObject(TMapCollisionData &collision);
        void solveExItemInPost(TItem *item, TMapCollisionData &collision);
        void solveExPost(TMapCollisionData &collision);

        void solveGraphWeb(TMapCollisionData &collision, TGraphWeb *web, const BoundingBox &bb);

    protected:
        JDrama::TActor *mActor;
        HitActorInfo mInfo;
    };

    bool isWarpIDValid(u16 warpID);

    bool isSolverRegistered(u8 area, u8 episode);
    ISolver *getSolver(u8 area, u8 episode);

    bool registerSolver(u8 area, u8 episode, ISolver *solver);
    bool deregisterSolver(u8 area, u8 episode);
    bool setDefaultSolver(ISolver *solver);

    void getRandomizedPointOnTriangle(TVec3f &out, const TVectorTriangle &triangle);
    void getRandomizedPosition(TVec3f &out, const TMapCollisionData &collision);
    void getRandomizedRotation(TVec3f &out, const HitActorInfo &actorInfo);
    void getRandomizedScale(TVec3f &out, const HitActorInfo &actorInfo);

};  // namespace Randomizer