#include "p_common.hxx"

#include <SMS/Enemy/Conductor.hxx>
#include <SMS/MapObj/MapObjRail.hxx>

using namespace Randomizer;

//--------------------------------------------------------------------------
// ExScene Solver
//--------------------------------------------------------------------------

static f64 sCurvePeriod         = 0.0;
static f64 sPrevTheta           = 0.0;
static f64 sPrevPhi             = 0.0;
static f32 sPrevUmaibouDistBias = 1.0f;

static TMario *sPlayers[4] = {};
static int sPlayersLoaded  = 0;

static TItem *sExItems[256] = {};
static int sExItemsLoaded   = 0;

static TMapObjBase *sSwitches[4] = {};
static int sSwitchesLoaded       = 0;

static int sRedCoinsPlaced = 0;
static int sShinesPlaced   = 0;

static TVec3f sLastItemPos = TVec3f::zero();

static void rotateVector(const TVec3f &pos, const TVec3f &axis, double angle, TVec3f &out) {
    TVec3f nAxis;
    PSVECNormalize(axis, nAxis);

    OSReport("Angle: %f\n", (f32)angle);

    double dot    = pos.dot(nAxis);
    double crossX = pos.z * nAxis.y - pos.y * nAxis.z;
    double crossY = pos.x * nAxis.z - pos.z * nAxis.x;
    double crossZ = pos.y * nAxis.x - pos.x * nAxis.y;

    out.x = (float)(pos.x * cos(angle) + crossX * sin(angle) + nAxis.x * dot * (1.0 - cos(angle)));
    out.y = (float)(pos.y * cos(angle) + crossY * sin(angle) + nAxis.y * dot * (1.0 - cos(angle)));
    out.z = (float)(pos.z * cos(angle) + crossZ * sin(angle) + nAxis.z * dot * (1.0 - cos(angle)));
}

static f64 randomDouble(f64 min, f64 max) { return min + (max - min) * Randomizer::randLerp64(); }

static void samplePointInOrientedCone(double *theta, double *phi, double thetaMax, double phiMax,
                                      double stretchX, double stretchY, double stretchZ,
                                      TVec3f &localPointOut) {
    f32 curTheta = *theta;
    f32 curPhi   = *phi;

    TVec3f axis = {cos(curTheta) * sin(curPhi), sin(curTheta), cos(curTheta) * cos(curPhi)};

    double cosTheta = randomDouble(cos(thetaMax), 1.0);
    double sinTheta = sqrtf(1.0 - cosTheta * cosTheta);
    double randPhi  = randomDouble(-phiMax, phiMax);

    TVec3f localPos = {sinTheta * sin(randPhi) * stretchX, cosTheta * stretchY,
                       sinTheta * cos(randPhi) * stretchZ};

    const f64 angleBounds = 0.05 * M_PI;

    rotateVector(localPos, axis, randomDouble(-angleBounds, angleBounds), localPointOut);

    *theta = atan2(sqrtf(localPointOut.x * localPointOut.x + localPointOut.y * localPointOut.y),
                   localPointOut.z);
    *phi   = atan2(localPointOut.x, localPointOut.z);
}

static void updateConeOrientation(double *theta, double *phi, double curvatureTheta,
                                  double curvaturePhi) {
    *theta += curvatureTheta;
    *phi += curvaturePhi;

    *theta = clamp(*theta, -M_PI / 4.0, M_PI / 4.0);
    if (*phi < 0.0) {
        *phi += 2.0 * M_PI;
    } else if (*phi > 2.0 * M_PI) {
        *phi -= 2.0 * M_PI;
    }
}

static void samplePathWithCurvature(double *theta, double *phi, double thetaMax, double phiMax,
                                    double curvatureTheta, double curvaturePhi, double stretchX,
                                    double stretchY, double stretchZ, TVec3f &localPointOut) {
    samplePointInOrientedCone(theta, phi, thetaMax, phiMax, stretchX, stretchY, stretchZ,
                              localPointOut);
    updateConeOrientation(theta, phi, curvatureTheta, curvaturePhi);
}

void SMSSolver::sampleRandomTopFloor(TMapCollisionData &collision, TVec3f &outT, TVec3f &outR,
                                     TVec3f &outS) {
    sampleRandomFloor(collision, outT, outR, outS, true);
}

BETTER_SMS_FOR_CALLBACK void resetExStageGlobals(TMarDirector *director) {
    sPrevTheta           = 0.0f;
    sPrevPhi             = 0.0f;
    sPrevUmaibouDistBias = 1.0f;
    for (int i = 0; i < 256; i++) {
        sExItems[i] = nullptr;
    }
    sPlayersLoaded  = 0;
    sExItemsLoaded  = 0;
    sLastItemPos    = TVec3f::zero();
    sSwitchesLoaded = 0;
    sRedCoinsPlaced = 0;
    sShinesPlaced   = 0;
}

BETTER_SMS_FOR_CALLBACK void removeExStageStickyWalls(TMarDirector *director) {
    TMapCollisionData::mIllegalCheckData.mFlags &= ~0x10;
}

static TGraphWeb *selectGraphWeb(TConductor *conductor, const char *name) {
    SMSSolver *solver =
        reinterpret_cast<SMSSolver *>(getSolver(gpMarDirector->mAreaID, gpMarDirector->mEpisodeID));
    if (SMS_isExMap__Fv() && !solver->isSecretCourseSimple()) {
        return conductor->getGraphByName("(null)");
    }
    return conductor->getGraphByName(name);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801ECD24, 0, 0, 0), selectGraphWeb);
SMS_PATCH_BL(SMS_PORT_REGION(0x801F0CB0, 0, 0, 0), selectGraphWeb);
SMS_PATCH_BL(SMS_PORT_REGION(0x801F1194, 0, 0, 0), selectGraphWeb);

static void solveUmaibou(const JDrama::TActor *actor, const HitActorInfo &info, TVec3f &outT,
                         TVec3f &outR, TVec3f &outS) {
    if (Randomizer::isRandomScale()) {
        getRandomizedScale(outS, info);
    } else {
        outS = actor->mScale;
    }

    TVec3f pprevObjPos = sSecretCourseLastPos;
    TVec3f prevObjPos  = sSecretCourseCurrentPos;

    TVec3f pprevObjRot = sSecretCourseLastRot;
    TVec3f prevObjRot  = sSecretCourseCurrentRot;

    const f32 thisDistBias = outS.z * 0.8f;
    f32 boundsX            = 0.0f;
    f32 boundsZ            = 0.0f;
    f32 theta              = 0.0f;
    f32 phi                = 0.0f;
    f32 adjustY            = 0.0f;

    size_t repeatCalc = 0;

    // This loop makes sure path generation doesn't exit the bounds of the collision grid
    do {
        theta = 2.0f * M_PI * randLerp32();  // Random angle around the sphere
        phi   = acosf(2.0f * randLerp32() -
                      1.0f);  // Random polar angle for uniform sampling on a sphere

        // Check if the angle difference is too acute (folding in on itself)
        while (fabs(sPrevPhi - phi) > M_PI * 0.3f) {
            phi = acosf(2.0f * randLerp32() - 1.0f);
        }

        phi = clamp<f32>(phi, M_PI * 0.3f, M_PI * 0.7f);

        // Check if the angle difference is too acute (folding in on itself)
        while (fabs(sPrevTheta - theta) > M_PI * 0.5f) {
            theta = 2.0f * M_PI * randLerp32();
        }

        f32 horizontalRadius;
        f32 verticalRadius;

        if (sSecretCourseFluddless) {
            horizontalRadius = sSecretCourseVertical ? 550.0f : 750.0f;
            verticalRadius   = sSecretCourseVertical ? 700.0f : 500.0f;
        } else {
            horizontalRadius = sSecretCourseVertical ? 700.0f : 1000.0f;
            verticalRadius   = sSecretCourseVertical ? 1000.0f : 700.0f;
        }

        horizontalRadius *= info.mExSpacialScale;
        verticalRadius *= info.mExSpacialScale;

        const f32 distBias =
            (sPrevUmaibouDistBias * cosf(sPrevTheta - theta) * 1.2f + thisDistBias) / 2.0f;

        // Calculate the point on the sphere surface
        const f32 sinPhi  = sinf(phi);
        const f32 adjustX = horizontalRadius * distBias * sinPhi * cosf(theta);
        adjustY           = verticalRadius * distBias * cosf(phi);
        const f32 adjustZ = horizontalRadius * distBias * sinPhi * sinf(theta);

        outT.x = prevObjPos.x + adjustX;
        outT.y = prevObjPos.y + adjustY;
        outT.z = prevObjPos.z + adjustZ;

        if (gpMapCollisionData) {
            boundsX = gpMapCollisionData->mAreaSizeX -
                      fabsf(1024.0f * 3 + 1024.0f * 2 * sinf(theta) * outS.z);
            boundsZ = gpMapCollisionData->mAreaSizeZ -
                      fabsf(1024.0f * 3 + 1024.0f * 2 * cosf(theta) * outS.z);
        } else {
            boundsX = 30000.0f;
            boundsZ = 30000.0f;
        }

        if (repeatCalc++ > 10) {
            if (prevObjPos.x > 0) {
                prevObjPos.x -= 200.0f;
            } else {
                prevObjPos.x += 200.0f;
            }
            if (prevObjPos.z > 0) {
                prevObjPos.z -= 200.0f;
            } else {
                prevObjPos.z += 200.0f;
            }
            repeatCalc = 0;
        }
    } while (fabsf(outT.x) >= boundsX || fabsf(outT.z) >= boundsZ ||
             outT.y < Max(100.0f, -adjustY) * 1.5f);

    sPrevUmaibouDistBias = thisDistBias;

    // For the spinning pillar blocks in secret courses like Ricco Harbor
    // we want to have future objects orient at the poles
    TVec3f dir = outT - prevObjPos;

    TVec3f dir_n;
    PSVECNormalize(dir, dir_n);

    TVec3f dir_scaled = dir_n;
    dir_scaled.scale(300.0f * info.mExSpacialScale * ((actor->mScale.x + actor->mScale.z) * 0.5f));

    TVec3f up = fabsf(dir_n.y) < 0.999f ? TVec3f::up() : TVec3f::forward();

    Mtx mtx;
    Matrix::normalToRotation(dir, TVec3f::up(), mtx);

    TVec3f t, s;
    Matrix::decompose(mtx, t, outR, s);

    sPrevPhi   = phi;
    sPrevTheta = theta;

    sSecretCourseLastPos    = prevObjPos;
    sSecretCourseCurrentPos = outT + dir_scaled;

    sSecretCourseLastRot    = sSecretCourseCurrentRot;
    sSecretCourseCurrentRot = outR;
}

static TMapObjBase *sRedCoinSwitch = nullptr;

void SMSSolver::solveExStageObject(TMapCollisionData &collision) {
    TVec3f outT = mActor->mTranslation;
    TVec3f outR = mActor->mRotation;
    TVec3f outS = mActor->mScale;

    if (mInfo.mIsSwitchObj) {
        sSwitches[sSwitchesLoaded++] = reinterpret_cast<TMapObjBase *>(mActor);
        return;
    }

    if (mInfo.mIsPlayer) {
        sPlayers[sPlayersLoaded++] = reinterpret_cast<TMario *>(mActor);
        return;
    }

    if (mInfo.mIsItemObj) {
        sExItems[sExItemsLoaded++] = reinterpret_cast<TItem *>(mActor);
        return;
    }

    if (isSecretCourseSimple()) {
        return;
    }

    if (Randomizer::isRandomScale()) {
        getRandomizedScale(outS, mInfo);
    } else {
        outS = mActor->mScale;
        outS.x *= mInfo.mScaleWeightXZ;
        outS.y *= mInfo.mScaleWeightY;
        outS.z *= mInfo.mScaleWeightXZ;
    }

    TVec3f pprevObjPos = sSecretCourseLastPos;
    TVec3f prevObjPos  = sSecretCourseCurrentPos;

    TVec3f pprevObjRot = sSecretCourseLastRot;
    TVec3f prevObjRot  = sSecretCourseCurrentRot;

    if (STR_EQUAL(mInfo.mObjectType, "Umaibou")) {
        solveUmaibou(mActor, mInfo, outT, outR, outS);
        mActor->mTranslation = outT;
        mActor->mRotation    = outR;
        mActor->mScale       = outS;
        return;
    }

    const f32 maxTheta     = M_PI * 0.3f * (sSecretCourseVertical ? 1.0f : 0.5f);
    const f32 maxThetaDiff = M_PI * 0.1f * (sSecretCourseVertical ? 1.0f : 0.5f);
    const f32 maxPhiDiff   = (mInfo.mIsExLinear ? M_PI * 0.3f : M_PI);

    size_t repeatCalc = 0;

    outT.y = 0.0f;

    f32 theta   = 0.0f;
    f32 phi     = 0.0f;
    f32 boundsX = 0.0f;
    f32 boundsZ = 0.0f;

    f32 horizontalRadius;
    f32 verticalRadius;
    if (sSecretCourseFluddless) {
        horizontalRadius = sSecretCourseVertical ? 600.0f : 800.0f;
        verticalRadius   = sSecretCourseVertical ? 300.0f : 200.0f;
    } else {
        horizontalRadius = sSecretCourseVertical ? 800.0f : 1000.0f;
        verticalRadius   = sSecretCourseVertical ? 400.0f : 300.0f;
    }

    do {
#if 1
        do {
            phi = acosf(2.0f * randLerp32() - 1.0f);
        } while (fabs(sPrevPhi - phi) > maxPhiDiff);

        const f32 phiDiff     = fabs(sPrevPhi - phi);
        const f32 thetaScaler = (maxPhiDiff - phiDiff) / (maxPhiDiff);

        if (thetaScaler <= 0.01f) {
            theta = sPrevTheta;
        } else {
            do {
                theta = 2.0f * M_PI * randLerp32();
            } while (fabs(sPrevTheta - theta) > maxThetaDiff);
        }

        theta = clamp<f32>(theta, -maxTheta, maxTheta);

        f32 ct = cosf(theta);
        f32 st = sinf(theta);
        f32 cp = cosf(phi);
        f32 sp = sinf(phi);

        const f32 scalar = mInfo.mExSpacialScale;

        const f32 adjustX = horizontalRadius * outS.x * ct * sp * scalar;
        const f32 adjustY =
            verticalRadius * outS.y * st * scalar;
        const f32 adjustZ = horizontalRadius * outS.z * ct * cp * scalar;
#else
        TVec3f adjust = TVec3f::zero();

        double theta = sPrevTheta;
        double phi   = sPrevPhi;

        double curvatureTheta = (sin(sCurvePeriod * -4.7) * cos(sCurvePeriod * 0.4) -
                                 cos(sCurvePeriod * 1.8) + sin(sCurvePeriod * -3.4)) *
                                0.2;
        double curvaturePhi = (sin(sCurvePeriod * -4.7) * cos(sCurvePeriod * -1.6) -
                               cos(sCurvePeriod * 1.2) + sin(sCurvePeriod * -3.4)) *
                              0.4;

        sCurvePeriod += 0.1;

        samplePathWithCurvature(&theta, &phi, M_PI * 0.4, M_PI * 0.2, curvatureTheta, curvaturePhi,
                                horizontalRadius, verticalRadius, horizontalRadius, adjust);

        f32 adjustX = adjust.x;
        f32 adjustY = adjust.y;
        f32 adjustZ = adjust.z;
#endif

        outT.x = prevObjPos.x + adjustX;
        outT.y = prevObjPos.y + adjustY;
        outT.z = prevObjPos.z + adjustZ;

        if (outT.y < 500.0f) {
            outT.y = 500.0f + (500.0f - outT.y);
        }

        if (gpMapCollisionData) {
            boundsX = gpMapCollisionData->mAreaSizeX -
                      fabsf(1024.0f * 3 + 1024.0f * 2 * sinf(theta) * outS.z);
            boundsZ = gpMapCollisionData->mAreaSizeZ -
                      fabsf(1024.0f * 3 + 1024.0f * 2 * cosf(theta) * outS.z);
        } else {
            boundsX = 30000.0f;
            boundsZ = 30000.0f;
        }

        if (repeatCalc++ > 10) {
            if (prevObjPos.x > 0) {
                prevObjPos.x -= 200.0f;
            } else {
                prevObjPos.x += 200.0f;
            }
            if (prevObjPos.z > 0) {
                prevObjPos.z -= 200.0f;
            } else {
                prevObjPos.z += 200.0f;
            }
            repeatCalc = 0;
        }

        sPrevTheta = theta;
        sPrevPhi   = phi;
    } while (fabsf(outT.x) >= boundsX || fabsf(outT.z) >= boundsZ || outT.y < 200.0f);

    sSecretCourseLastPos    = prevObjPos;
    sSecretCourseCurrentPos = outT;

    TVec3f dir = outT - prevObjPos;

    TVec3f dir_n;
    PSVECNormalize(dir, dir_n);

    TVec3f up = fabsf(dir_n.y) < 0.999f ? TVec3f::up() : TVec3f::forward();

    Mtx mtx;
    Matrix::normalToRotation(dir, TVec3f::up(), mtx);

    TVec3f t, s;
    Matrix::decompose(mtx, t, outR, s);

    sPrevPhi   = phi;
    sPrevTheta = theta;

    mActor->mTranslation = outT;
    mActor->mRotation    = outR;
    mActor->mScale       = outS;

    if (STR_EQUAL(mInfo.mObjectType, "WaterHitHideObj") ||
        STR_EQUAL(mInfo.mObjectType, "WaterHitPictureHideObj")) {
        addHitHideObj(reinterpret_cast<TMapObjBase *>(mActor));
    }

    if (STR_EQUAL(mActor->mKeyName, "normalvariant3") ||
        (STR_EQUAL(mActor->mKeyName, "WoodBlockLarge 0") && gpMarDirector->mAreaID == 32)) {
        sSecretCourseHasEndPlatform = true;
        sSecretCourseEndPlatform    = mActor->mTranslation;
        sSecretCourseShinePos       = mActor->mTranslation;
        sSecretCourseShinePos.y += 200.0f * scaleLinearAtAnchor(mActor->mScale.y, 1.5f, 1.0f);
    }
}

static const char *getRegisterNameAndDoChecks(TMapObjBase *obj) {
    JSUMemoryInputStream *in;
    SMS_FROM_GPR(31, in);

    load__Q26JDrama6TActorFR20JSUMemoryInputStream(obj, in);

    const char *registerName = in->readString();
    if (STR_EQUAL(registerName, "SkyIsland")) {
        sSecretCourseStartPlatform = obj->mTranslation;
        sSecretCourseStart         = obj->mTranslation;
        sSecretCourseStart.y += 300.0f;

        obj->mScale      = TVec3f(1.2f, 1.2f, 1.2f);
        obj->mRotation.x = 0.0f;
        obj->mRotation.z = 0.0f;
    }
    return registerName;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801AF76C, 0, 0, 0), getRegisterNameAndDoChecks);
SMS_WRITE_32(SMS_PORT_REGION(0x801AF770, 0, 0, 0), 0x60000000);
SMS_WRITE_32(SMS_PORT_REGION(0x801AF774, 0, 0, 0), 0x60000000);

void Randomizer::SMSSolver::solveExItemInPost(TItem *item, TMapCollisionData &collision) {
    HitActorInfo &actorInfo = getRandomizerInfo(item);

    if (actorInfo.mIsShineObj) {
        if (isSecretCourseSimple()) {
            sampleRandomFloor(collision, item->mTranslation, item->mRotation, item->mScale, true);
            item->mTranslation.y += 300.0f;
        } else {
            TVec3f *pos = sSecretCourseHasEndPlatform ? &sSecretCourseShinePos
                                                      : &sSecretCourseLastPos;

            f32 sampleY = sSecretCourseShinePos.y;
            f32 roofY;
            while (true) {
                const TBGCheckData *sample;
                sampleY = collision.checkGround(pos->x, 1000000.0f, pos->z, 0, &sample);

                if (sample == &TMapCollisionData::mIllegalCheckData ||
                    (sample->mType == 1536 || sample->mType == 2048)) {
                    sampleY = sSecretCourseCurrentPos.y;
                    break;
                }

                break;
            }

            item->mTranslation.x = pos->x;
            item->mTranslation.y = sampleY + 300.0f;
            item->mTranslation.z = pos->z;
        }

        if (Randomizer::isRandomScale()) {
            getRandomizedScale(item->mScale, actorInfo);
        }

        item->mInitialPosition = item->mTranslation;

        sLastItemPos = item->mTranslation;

        int layer   = (sShinesPlaced / 4) + 1;
        float angle = (sShinesPlaced % 4) * (M_PI / 2.0f);

        sSecretCourseShinePos = sSecretCourseEndPlatform;
        sSecretCourseShinePos.x += 300.0f * layer * cosf(angle);
        sSecretCourseShinePos.z += 300.0f * layer * sinf(angle);

        sShinesPlaced += 1;

        return;
    }

    do {
        sampleRandomFloor(collision, item->mTranslation, item->mRotation, item->mScale, true);
        item->mTranslation.y += 500.0f;
    } while (PSVECDistance(item->mTranslation, sLastItemPos) < 5000.0f);

    sLastItemPos = item->mTranslation;
}

void Randomizer::SMSSolver::solveExPost(TMapCollisionData &collision) {
    if (!isSecretCourseSimple()) {
        for (int i = 0; i < sPlayersLoaded; i++) {
            TMario *player = sPlayers[i];
            if (player == nullptr) {
                continue;
            }

            player->mTranslation = sSecretCourseStartPlatform;
            player->mTranslation.y += 200.0f;
            getRandomizedRotation(player->mRotation, getRandomizerInfo(player));
        }

        for (int i = 0; i < sSwitchesLoaded; i++) {
            TMapObjBase *obj = sSwitches[i];
            if (obj == nullptr) {
                continue;
            }

            HitActorInfo &info = getRandomizerInfo(obj);
            if (STR_EQUAL(info.mObjectType, "RedCoinSwitch") && sSecretCourseFluddless) {
                continue;
            }

            obj->mTranslation = sSecretCourseStartPlatform;

            switch (i) {
            default:
            case 0:
                obj->mTranslation.x += 500.0f;
                break;
            case 1:
                obj->mTranslation.x -= 500.0f;
                break;
            case 2:
                obj->mTranslation.z += 500.0f;
                break;
            case 3:
                obj->mTranslation.z -= 500.0f;
                break;
            }

            for (int i = 0; i < obj->mCollisionManager->mMapCollisionNum; ++i) {
                TMapCollisionBase *collision = obj->mCollisionManager->mMapCollisions[i];
                if (collision == nullptr) {
                    continue;
                }

#define moveSRT__17TMapCollisionMoveFRCQ29JGeometry8TVec3_f                                        \
    ((void (*)(TMapCollisionBase *, const TVec3f &, const TVec3f &, const TVec3f &))0x8018E22C)

                moveSRT__17TMapCollisionMoveFRCQ29JGeometry8TVec3_f(collision, obj->mTranslation,
                                                                    obj->mRotation, obj->mScale);

#undef moveSRT__17TMapCollisionMoveFRCQ29JGeometry8TVec3_f
            }

            // Multiply coin timer by 3
            if (gpMarDirector->mAreaID == TGameSequence::AREA_MAMMAEX0) {
                *(u32 *)((u8 *)obj + 0x138) *= 4;
            } else {
                *(u32 *)((u8 *)obj + 0x138) *= 3;
            }
        }
    } else {
        for (int i = 0; i < sSwitchesLoaded; i++) {
            TMapObjBase *obj = sSwitches[i];
            if (obj == nullptr) {
                continue;
            }

            // Multiply coin timer by 3
            if (gpMarDirector->mAreaID == TGameSequence::AREA_MAMMAEX0) {
                *(u32 *)((u8 *)obj + 0x138) *= 2.0f;
            } else {
                *(u32 *)((u8 *)obj + 0x138) *= 1.5f;
            }
        }
    }

    for (int i = 0; i < sExItemsLoaded; i++) {
        TItem *item = sExItems[i];
        if (item == nullptr) {
            continue;
        }

        solveExItemInPost(item, collision);
    }
}
