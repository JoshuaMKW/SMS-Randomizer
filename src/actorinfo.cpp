#include <JSystem/JGadget/UnorderedMap.hxx>

#include "actorinfo.hxx"

struct _HitActorStruct {
    JDrama::TActor *mActor;
    HitActorInfo mInfo;
};

#define MAX_HIT_ACTOR_LIST_SIZE 1024
_HitActorStruct sHitActorList[MAX_HIT_ACTOR_LIST_SIZE];
size_t sHitActorListSize = 0;

HitActorInfo &getRandomizerInfo(JDrama::TActor *actor) {
    for (size_t i = 0; i < sHitActorListSize; i++) {
        if (sHitActorList[i].mActor == actor) {
            return sHitActorList[i].mInfo;
        }
    }

    SMS_ASSERT(sHitActorListSize < MAX_HIT_ACTOR_LIST_SIZE,
               "HitActorList is full! Increase the size of the list or use less objects!");

    sHitActorList[sHitActorListSize++] = {actor, HitActorInfo()};
    _HitActorStruct &info              = sHitActorList[sHitActorListSize - 1];

    info.mInfo.mShouldRandomize     = true;
    info.mInfo.mShouldResizeUniform = true;
    info.mInfo.mShouldResizeXZ      = true;
    info.mInfo.mShouldResizeY       = true;
    info.mInfo.mShouldRotateXZ      = false;
    info.mInfo.mShouldRotateY       = true;
    info.mInfo.mIsChangeStageObj    = false;
    info.mInfo.mIsGroundValid       = true;
    info.mInfo.mIsRoofValid         = false;
    info.mInfo.mIsWallValid         = false;
    info.mInfo.mIsWaterValid        = false;
    info.mInfo.mIsPlayer            = false;
    info.mInfo.mIsBaseObj        = false;
    info.mInfo.mIsLiveActor         = false;
    info.mInfo.mIsItemObj           = false;
    info.mInfo.mIsShineObj          = false;
    info.mInfo.mIsSprayableObj      = false;
    info.mInfo.mIsSwitchObj         = false;
    info.mInfo.mIsUnderwaterValid   = false;
    info.mInfo.mIsSurfaceBound      = true;
    info.mInfo.mIsExLinear          = false;
    info.mInfo.mFromSurfaceDist     = 0;
    info.mInfo.mAdjustRotation.set(0, 0, 0);
    info.mInfo.mSurfaceNormal.set(0, 1, 0);

    return info.mInfo;
}

void initActorInfoMap(TMarDirector *director) { sHitActorListSize = 0; }