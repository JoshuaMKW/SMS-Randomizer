#include <JSystem/JGadget/UnorderedMap.hxx>
#include <BetterSMS/libs/global_unordered_map.hxx>

#include "actorinfo.hxx"

BetterSMS::TGlobalUnorderedMap<JDrama::TActor *, HitActorInfo> sHitActorMap;

HitActorInfo &getRandomizerInfo(JDrama::TActor *actor) {
    auto status = sHitActorMap.emplace(actor, HitActorInfo());
    HitActorInfo &info = status.first->second;

    if (status.second) {
        info.mShouldRandomize     = true;
        info.mShouldResizeUniform = true;
        info.mShouldResizeXZ      = true;
        info.mShouldResizeY       = true;
        info.mShouldRotateXZ      = false;
        info.mShouldRotateY       = true;
        info.mIsChangeStageObj    = false;
        info.mIsGroundValid       = true;
        info.mIsRoofValid         = false;
        info.mIsWallValid         = false;
        info.mIsWaterValid        = false;
        info.mIsPlayer            = false;
        info.mIsItemObj           = false;
        info.mIsShineObj          = false;
        info.mIsSprayableObj      = false;
        info.mIsSwitchObj         = false;
        info.mIsUnderwaterValid   = false;
        info.mIsSurfaceBound      = true;
        info.mIsExLinear          = false;
        info.mFromSurfaceDist     = 0;
        info.mAdjustRotation.set(0, 0, 0);
        info.mSurfaceNormal.set(0, 1, 0);
    }

    return info;
}

void initActorInfoMap(TMarDirector *director) {
    sHitActorMap.clear();
}