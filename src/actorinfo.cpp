#include "actorinfo.hxx"

TDictI<HitActorInfo *> sHitActorMap;

HitActorInfo *getRandomizerInfo(THitActor *actor) {
    const u32 key = reinterpret_cast<u32>(actor);
    if (!sHitActorMap.hasKey(key)) {
        auto *info = new HitActorInfo();
        sHitActorMap.set(key, info);
        return info;
    }
    return *sHitActorMap.get(key);
}

void initActorInfoMap(TMarDirector *director) {
    sHitActorMap.empty();
}