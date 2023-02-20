#include <Dolphin/types.h>
#include <Dolphin/math.h>

#include <JSystem/JDrama/JDRActor.hxx>
#include <JSystem/JDrama/JDRNameRefGen.hxx>
#include <JSystem/JGeometry/JGMQuat.hxx>
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
#include <SMS/Map/PollutionLayer.hxx>

#include <BetterSMS/libs/triangle.hxx>
#include <BetterSMS/libs/constmath.hxx>
#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"
#include "seed.hxx"
#include "settings.hxx"
#include "solver.hxx"

extern bool sIsMapLoaded;

static void randomizeObject() {
    THitActor *actor;
    SMS_FROM_GPR(31, actor);

    if (!sIsMapLoaded || !gpMapCollisionData)
        return;

    Randomizer::BaseSolver *solver = Randomizer::getSolver(gpMarDirector->mAreaID, gpMarDirector->mEpisodeID);
    solver->setTarget(actor);

    if (!solver->solve(*gpMapCollisionData)) {
        auto &info = solver->getInfo();
        OSReport("[Randomizer] Could not solve object \"%s (%s)\"!\n", info.mObjectType,
                 info.mObjectKey);
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802F6EE0, 0, 0, 0), randomizeObject);
SMS_WRITE_32(SMS_PORT_REGION(0x802F6EE4, 0, 0, 0), 0x907E0010);
SMS_WRITE_32(SMS_PORT_REGION(0x802F6EE8, 0, 0, 0), 0x907E0014);