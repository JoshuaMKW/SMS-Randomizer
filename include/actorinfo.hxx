#pragma once

#include <Dolphin/types.h>

#include <SMS/MapObj/MapObjGeneral.hxx>
#include <SMS/System/MarDirector.hxx>
#include <SMS/macros.h>

#include <BetterSMS/libs/container.hxx>

struct HitActorInfo {
    bool mShouldRandomize     : 1;
    bool mIsUnderwaterValid   : 1;
    bool mIsWaterValid        : 1;
    bool mIsGroundValid       : 1;
    bool mIsWallBound         : 1;
    bool mIsChangeStageObj    : 1;
    bool mIsItemObj           : 1;
    bool mShouldResizeUniform : 1;
    bool mShouldResizeY       : 1;
    bool mShouldResizeXZ      : 1;
    bool mShouldRotateY       : 1;
    bool mShouldRotateXZ      : 1;
    s16 mFromGroundHeight;
};

HitActorInfo *getRandomizerInfo(THitActor *actor);