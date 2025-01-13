#include <Dolphin/DVD.h>

#include <JSystem/JGadget/List.hxx>
#include <JSystem/JGadget/UnorderedMap.hxx>

#include <SMS/Manager/FlagManager.hxx>
#include <SMS/Manager/PollutionManager.hxx>
#include <SMS/Map/Map.hxx>
#include <SMS/Map/MapCollisionStatic.hxx>
#include <SMS/Map/PollutionLayer.hxx>
#include <SMS/Strategic/ObjChara.hxx>
#include <SMS/Strategic/Strategy.hxx>
#include <SMS/raw_fn.hxx>

#include <BetterSMS/libs/constmath.hxx>
#include <BetterSMS/libs/geometry.hxx>
#include <BetterSMS/libs/global_unordered_map.hxx>
#include <BetterSMS/libs/global_vector.hxx>
#include <BetterSMS/libs/triangle.hxx>
#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"
#include "seed.hxx"
#include "settings.hxx"
#include "solver.hxx"
#include "surface.hxx"

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
    // Selects a random position within the collision grid (as a circle area)
    const f32 boundsX = collision.mAreaSizeX - (1024.0f * 2);
    const f32 boundsZ = collision.mAreaSizeZ - (1024.0f * 2);

    f32 rX     = boundsX * sqrtf(randLerp());
    f32 thetaX = sqrtf(randLerp()) * 2 * M_PI;

    f32 rZ     = boundsZ * sqrtf(randLerp());
    f32 thetaZ = sqrtf(randLerp()) * 2 * M_PI;

    out.x = rX * cosf(thetaX);
    out.y = 100000.0f;
    out.z = rZ * sinf(thetaZ);
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

    out.x *= actorInfo.mScaleWeightXZ;
    out.y *= actorInfo.mScaleWeightY;
    out.z *= actorInfo.mScaleWeightXZ;
}