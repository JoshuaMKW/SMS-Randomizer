#include <Dolphin/math.h>
#include <Dolphin/types.h>

#include <JSystem/JDrama/JDRActor.hxx>
#include <JSystem/JDrama/JDRNameRefGen.hxx>
#include <JSystem/JGeometry/JGMQuat.hxx>
#include <JSystem/JGeometry/JGMRotation.hxx>
#include <JSystem/JGeometry/JGMVec.hxx>

#include <SMS/Manager/FlagManager.hxx>
#include <SMS/Manager/PollutionManager.hxx>
#include <SMS/Map/Map.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/Map/MapCollisionStatic.hxx>
#include <SMS/Map/PollutionLayer.hxx>
#include <SMS/Strategic/Strategy.hxx>
#include <SMS/System/MarDirector.hxx>
#include <SMS/macros.h>
#include <SMS/rand.h>
#include <SMS/raw_fn.hxx>

#include <BetterSMS/libs/constmath.hxx>
#include <BetterSMS/libs/triangle.hxx>
#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"
#include "seed.hxx"
#include "settings.hxx"
#include "solver.hxx"

extern bool sIsMapLoaded;

static THitActor *sHitActorList[1024];
static size_t sHitActorCount = 0;

THitActor** getHitActors(size_t& count) {
    count = sHitActorCount;
    return sHitActorList;
}

BETTER_SMS_FOR_CALLBACK void resetActorList(TMarDirector *director) { sHitActorCount = 0; }

static void randomizeObject() {
    THitActor *actor;
    SMS_FROM_GPR(31, actor);

    sHitActorList[sHitActorCount++] = actor;

    Randomizer::ISolver *solver =
        Randomizer::getSolver(gpMarDirector->mAreaID, gpMarDirector->mEpisodeID);

    if (!solver->solve(actor, *gpMapCollisionData)) {
        OSReport("[Randomizer] Could not solve object \"%s\"!\n", actor->mKeyName);
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802F6EE0, 0, 0, 0), randomizeObject);
SMS_WRITE_32(SMS_PORT_REGION(0x802F6EE4, 0, 0, 0), 0x907E0010);
SMS_WRITE_32(SMS_PORT_REGION(0x802F6EE8, 0, 0, 0), 0x907E0014);

static void postRandomize() {
    Randomizer::ISolver *solver =
        Randomizer::getSolver(gpMarDirector->mAreaID, gpMarDirector->mEpisodeID);

    for (size_t i = 0; i < sHitActorCount; i++) {
        THitActor *actor = sHitActorList[i];

        HitActorInfo &actorInfo = getRandomizerInfo(actor);

        if (actorInfo.mIsBaseObj && !actorInfo.mIsNpcObj) {
            TMapObjGeneral *general = reinterpret_cast<TMapObjGeneral *>(actor);
            if (!general->mCollisionManager) {
                continue;
            }

            for (size_t i = 0; i < general->mCollisionManager->mMapCollisionNum; i++) {
                if (!general->mCollisionManager->mMapCollisions[i]) {
                    continue;
                }

                general->mCollisionManager->mMapCollisions[i]->_5C &= ~0x1;
                general->mCollisionManager->mMapCollisions[i]->moveSRT(
                    actor->mTranslation, actor->mRotation, actor->mScale);
            }
        }
    }

    if (!solver->postSolve(*gpMapCollisionData)) {
        OSReport("[Randomizer] Could not post-solve!\n");
    }

    *(bool *)(&gpMarDirector->_260) = true;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802998BC, 0, 0, 0), postRandomize);
SMS_WRITE_32(SMS_PORT_REGION(0x802998C0, 0, 0, 0), 0x60000000);