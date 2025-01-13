#pragma once

#include <Dolphin/types.h>

#include <SMS/MapObj/MapObjGeneral.hxx>
#include <SMS/System/MarDirector.hxx>
#include <SMS/macros.h>

#include <BetterSMS/libs/container.hxx>

#include "actorinfo.hxx"

bool isContextRandomizable(TMarDirector *director);
bool isGroundContextAllowed(TMarDirector *director, f32 x, f32 y, f32 z, const HitActorInfo *actorInfo,
                      const TBGCheckData *floor);