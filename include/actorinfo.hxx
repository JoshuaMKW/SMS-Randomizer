#pragma once

#include <Dolphin/types.h>

#include <SMS/MapObj/MapObjGeneral.hxx>
#include <SMS/System/MarDirector.hxx>
#include <SMS/macros.h>

#include <BetterSMS/libs/container.hxx>

struct HitActorInfo {
    bool mShouldRandomize     : 1;
    bool mShouldResizeUniform : 1;
    bool mShouldResizeY       : 1;
    bool mShouldResizeXZ      : 1;
    bool mShouldRotateY       : 1;
    bool mShouldRotateXZ      : 1;
    bool mIsGroundValid       : 1;
    bool mIsRoofValid         : 1;
    bool mIsWallValid         : 1;
    bool mIsWaterValid        : 1;
    bool mIsUnderwaterValid   : 1;
    bool mIsSurfaceBound      : 1;
    bool mIsPlayer            : 1;
    bool mIsChangeStageObj    : 1;
    bool mIsItemObj           : 1;
    bool mIsShineObj          : 1;
    bool mIsSprayableObj      : 1;
    bool mIsSwitchObj         : 1;
    bool mIsExLinear          : 1;
    s16 mFromSurfaceDist   = 0;
    TVec3s mAdjustRotation;
};

HitActorInfo *getRandomizerInfo(THitActor *actor);