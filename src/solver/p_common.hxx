#pragma once

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

extern bool sIsMapLoaded;
extern u32 sStageSeed;

extern TGlobalVector<u16> sWarpIDWhiteList;
extern TGlobalUnorderedMap<u16, Randomizer::ISolver *> sSolvers;
extern Randomizer::ISolver *sDefaultSolver;

extern TVec3f sSecretCourseStart;
extern TVec3f sSecretCourseStartPlatform;
extern TVec3f sSecretCourseCurrentPos;
extern TVec3f sSecretCourseLastPos;
extern TVec3f sSecretCourseCurrentRot;
extern TVec3f sSecretCourseLastRot;
extern TVec3f sSecretCourseCurrentDir;
extern TVec3f sSecretCourseShinePos;
extern size_t sSecretCourseObjectsLoaded;
extern size_t sSecretCourseSwitchesLoaded;
extern bool sSecretCourseVertical;
extern bool sSecretCourseFluddless;
extern bool sSecretCourseHasEndPlatform;

#define STR_EQUAL(a, b) (strcmp(a, b) == 0)

extern void initializeDefaultActorInfo(const TMarDirector &director, HitActorInfo &actorInfo);
extern void resetHitHideObjs();
extern void addHitHideObj(TMapObjBase *actor);
