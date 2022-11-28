#include "actorinfo.hxx"

TDictI<HitActorInfo *> sHitActorMap;

HitActorInfo *getRandomizerInfo(THitActor *actor) {
    const u32 key = reinterpret_cast<u32>(actor);
    if (!sHitActorMap.hasKey(key)) {
        auto *info = new HitActorInfo();
        info->mShouldRandomize     = true;
        info->mShouldResizeUniform = true;
        info->mShouldResizeXZ      = true;
        info->mShouldResizeY       = true;
        info->mShouldRotateXZ      = false;
        info->mShouldRotateY       = true;
        info->mIsChangeStageObj    = false;
        info->mIsGroundValid       = true;
        info->mIsRoofValid         = false;
        info->mIsWallValid         = false;
        info->mIsWaterValid        = false;
        info->mIsItemObj           = false;
        info->mIsSprayableObj      = false;
        info->mIsUnderwaterValid   = false;
        info->mIsSurfaceBound      = false;
        info->mFromSurfaceDist     = 0;
        info->mAdjustRotation.set(0, 0, 0);
        sHitActorMap.set(key, info);
        return info;
    }
    return *sHitActorMap.get(key);
}

void initActorInfoMap(TMarDirector *director) {
    sHitActorMap.empty();
}