#include <JSystem/JGeometry/JGMRotation.hxx>
#include <JSystem/JGeometry/JGMVec.hxx>

#include <SMS/rand.h>
#include <SMS/raw_fn.hxx>
#include <SMS/macros.h>
#include <SMS/Strategic/Strategy.hxx>
#include <SMS/Map/Map.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/Map/MapCollisionStatic.hxx>
#include <SMS/System/MarDirector.hxx>
#include <SMS/Manager/FlagManager.hxx>
#include <SMS/Manager/PollutionManager.hxx>
#include <SMS/Manager/MarioParticleManager.hxx>
#include <SMS/Map/PollutionLayer.hxx>

#include <BetterSMS/libs/triangle.hxx>
#include <BetterSMS/libs/constmath.hxx>
#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"
#include "seed.hxx"
#include "settings.hxx"

static JGadget::TList<TMapObjBase *> sHitHideObjs;

void resetHitHideObjs() { sHitHideObjs.erase(sHitHideObjs.begin(), sHitHideObjs.end()); }

void addHitHideObj(TMapObjBase* actor) { sHitHideObjs.insert(sHitHideObjs.end(), actor); }

void emitHintEffectForHideObjs(TMarDirector *director) {
    if (director->mCurState != TMarDirector::STATE_NORMAL)
        return;

    for (auto &obj : sHitHideObjs) {
        if (!obj->mActorData)
            continue;

        // TODO: stop emitting when hide obj is shown

        auto *model = obj->getModel();
        
        auto *emitter = gpMarioParticleManager->emitAndBindToMtxPtr(55, model->mBaseMtx, 0, nullptr);
        if (emitter)
            emitter->mSize1 = { 1.25f, 0.9f, 1.25f };
    }
}