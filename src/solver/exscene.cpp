#include "p_common.hxx"

#include <SMS/MapObj/MapObjRail.hxx>
#include <SMS/Enemy/Conductor.hxx>

using namespace Randomizer;

//--------------------------------------------------------------------------
// ExScene Solver
//--------------------------------------------------------------------------

static f32 sPrevTheta           = 0.0f;
static f32 sPrevPhi             = 0.0f;
static f32 sPrevUmaibouDistBias = 1.0f;

static TItem *sExItems[256] = {};
static int sExItemsLoaded   = 0;

static int sRedCoinsPlaced = 0;

static TVec3f sLastItemPos = TVec3f::zero();

void SMSSolver::sampleRandomTopFloor(TMapCollisionData &collision, TVec3f &outT, TVec3f &outR,
                                     TVec3f &outS) {
    const bool isUnderwaterValid = mInfo.mIsUnderwaterValid;
    const bool isWaterValid      = mInfo.mIsWaterValid;
    const bool isGroundValid     = mInfo.mIsGroundValid;

    constexpr size_t MaxFloors = 8;
    const TBGCheckData *floors[MaxFloors];
    f32 groundY[MaxFloors];

    TVec3f target = TVec3f::zero();

    for (size_t i = 0; i < getSampleMax(); ++i) {
        getRandomizedPosition(target, collision);

        const TBGCheckData *floor = nullptr;
        f32 groundY               = collision.checkGround(target.x, target.y, target.z, 0, &floor);

        if (floor == &collision.mIllegalCheckData)
            continue;

        adjustSampledFloor(target, *floor);

        if (!isSampledFloorValid(target, *floor))
            continue;

        outT = target;

        if (mInfo.mIsSurfaceBound) {
            PSVECNormalize(floor->mNormal, mInfo.mSurfaceNormal);

            TVec3f up = fabsf(mInfo.mSurfaceNormal.y) < 0.999f ? TVec3f::up() : TVec3f::forward();

            Mtx mtx;
            Matrix::normalToRotation(floor->mNormal, up, mtx);

            TVec3f t, s;
            Matrix::decompose(mtx, t, outR, s);

            outR.x += mInfo.mAdjustRotation.x;
            outR.y += mInfo.mAdjustRotation.y;
            outR.z += mInfo.mAdjustRotation.z;
        } else {
            getRandomizedRotation(outR, mInfo);
        }

        if (Randomizer::isRandomScale()) {
            getRandomizedScale(outS, mInfo);
        }
        break;
    }
}

BETTER_SMS_FOR_CALLBACK void resetExStageGlobals(TMarDirector *director) {
    sPrevTheta           = 0.0f;
    sPrevPhi             = 0.0f;
    sPrevUmaibouDistBias = 1.0f;
    for (int i = 0; i < 256; i++) {
        sExItems[i] = nullptr;
    }
    sExItemsLoaded = 0;
    sLastItemPos   = TVec3f::zero();
    sRedCoinsPlaced = 0;
}

BETTER_SMS_FOR_CALLBACK void removeExStageStickyWalls(TMarDirector *director) {
    TMapCollisionData::mIllegalCheckData.mFlags &= ~0x10;
}

static TGraphWeb *selectGraphWeb(const char *name) {
    if (SMS_isExMap__Fv()) {
        return gpConductor->getGraphByName("(null)");
    }
    return gpConductor->getGraphByName(name);
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
        theta = 2.0f * M_PI * randLerp();  // Random angle around the sphere
        phi =
            acosf(2.0f * randLerp() - 1.0f);  // Random polar angle for uniform sampling on a sphere

        while (fabsf(sPrevPhi - phi) > M_PI * 0.5f) {
            phi = acosf(2.0f * randLerp() - 1.0f);
        }

        phi = clamp<f32>(phi, M_PI * 0.3f, M_PI * 0.7f);

        // Check if the angle difference is too acute (folding in on itself)
        while (fabsf(sPrevTheta - theta) > M_PI * 0.5f) {
            theta = 2.0f * M_PI * randLerp();
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
        adjustY = verticalRadius * distBias * cosf(phi);
        const f32 adjustZ = horizontalRadius * distBias * sinPhi * sinf(theta);

        outT.x = prevObjPos.x + adjustX;
        outT.y = prevObjPos.y + adjustY;
        outT.z = prevObjPos.z + adjustZ;

        boundsX =
            gpMapCollisionData->mAreaSizeX - (1024.0f * 2 + 1024.0f * 2 * sinf(theta) * outS.z);
        boundsZ =
            gpMapCollisionData->mAreaSizeZ - (1024.0f * 2 + 1024.0f * 2 * cosf(theta) * outS.z);

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
    } while (fabsf(outT.x) >= boundsX || fabsf(outT.z) >= boundsZ || outT.y < Max(100.0f, -adjustY) * 1.5f);

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

    sSecretCourseLastPos    = sSecretCourseCurrentPos;
    sSecretCourseCurrentPos = outT + dir_scaled;

    sSecretCourseLastRot    = sSecretCourseCurrentRot;
    sSecretCourseCurrentRot = outR;
}

static TMapObjBase *sRedCoinSwitch = nullptr;

void SMSSolver::solveExStageObject(TMapCollisionData &collision) {
    TVec3f outT = mActor->mTranslation;
    TVec3f outR = mActor->mRotation;
    TVec3f outS = mActor->mScale;

    if (STR_EQUAL(mInfo.mObjectType, "RedCoinSwitch")) {
        sRedCoinSwitch = reinterpret_cast<TMapObjBase *>(mActor);
    }

    if (Randomizer::isRandomScale()) {
        getRandomizedScale(outS, mInfo);
    } else {
        outS = mActor->mScale;
    }

    // Handle these later.
    if (mInfo.mIsItemObj) {
        sExItems[sExItemsLoaded++] = reinterpret_cast<TItem *>(mActor);
        return;
    }

    if (mInfo.mIsSwitchObj) {
        switch (sSecretCourseSwitchesLoaded) {
        default:
        case 0:
            outT = sSecretCourseStartPlatform;
            outT.x += 500.0f;
            break;
        case 1:
            outT = sSecretCourseStartPlatform;
            outT.x -= 500.0f;
            break;
        case 2:
            outT = sSecretCourseStartPlatform;
            outT.z += 500.0f;
            break;
        case 3:
            outT = sSecretCourseStartPlatform;
            outT.z -= 500.0f;
            break;
        }

        mActor->mTranslation = outT;
        mActor->mRotation    = outR;
        mActor->mScale       = outS;
        return;
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

    //if (mInfo.mIsRailObj) {
    //    TRailMapObj *obj = reinterpret_cast<TRailMapObj *>(mActor);
    //    if (obj->mGraphTracer) {
    //        obj->mGraphTracer->mGraph = (TGraphWeb *)0x803afb48;
    //    }
    //    //obj->mGraphTracer = nullptr;
    //}

    if (mInfo.mIsExLinear || true) {
        Mtx rot;

        f32 boundsX = 0.0f;
        f32 boundsZ = 0.0f;
        f32 theta   = 0.0f;
        f32 phi     = 0.0f;

        size_t repeatCalc = 0;

        do {
            float xzDistance = 0;

            theta = 2.0f * M_PI * randLerp();
            phi   = acosf(2.0f * randLerp() - 1.0f);

            while (fabsf(sPrevPhi - phi) > M_PI * 0.5f) {
                phi = acosf(2.0f * randLerp() - 1.0f);
            }

            // Check if the angle difference is too acute (folding in on itself)
            while (fabsf(sPrevTheta - theta) > M_PI * 0.5f) {
                theta = 2.0f * M_PI * randLerp();
            }

            f32 horizontalRadius;
            f32 verticalRadius;
            if (sSecretCourseFluddless) {
                horizontalRadius = sSecretCourseVertical ? 800.0f : 1000.0f;
                verticalRadius   = sSecretCourseVertical ? 400.0f : 200.0f;
            } else {
                horizontalRadius = sSecretCourseVertical ? 1200.0f : 1500.0f;
                verticalRadius   = sSecretCourseVertical ? 600.0f : 400.0f;
            }
            horizontalRadius *= mInfo.mExSpacialScale;
            verticalRadius *= mInfo.mExSpacialScale;

            const f32 adjustX = horizontalRadius * outS.x * sinf(theta) * cosf(phi);
            const f32 adjustY = verticalRadius * outS.y * randLerp();
            const f32 adjustZ = horizontalRadius * outS.z * cosf(theta);

            outT.x = prevObjPos.x + adjustX;
            outT.y = prevObjPos.y + adjustY;
            outT.z = prevObjPos.z + adjustZ;

            xzDistance = sqrtf(adjustX * adjustX + adjustZ * adjustZ);

            sPrevPhi   = phi;
            sPrevTheta = theta;

            boundsX =
                gpMapCollisionData->mAreaSizeX - (1024.0f * 2 + 1024.0f * 2 * sinf(theta) * outS.z);
            boundsZ =
                gpMapCollisionData->mAreaSizeZ - (1024.0f * 2 + 1024.0f * 2 * cosf(theta) * outS.z);


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
        } while (fabsf(outT.x) >= boundsX || fabsf(outT.z) >= boundsZ || outT.y < 200.0f);

        TVec3f dir = outT - sSecretCourseLastPos;

        TVec3f dir_n;
        PSVECNormalize(dir, dir_n);

        TVec3f up = fabsf(dir_n.y) < 0.999f ? TVec3f::up() : TVec3f::forward();

        Mtx mtx;
        Matrix::normalToRotation(dir, TVec3f::up(), mtx);

        TVec3f t, s;
        Matrix::decompose(mtx, t, outR, s);

        sSecretCourseLastPos    = sSecretCourseCurrentPos;
        sSecretCourseCurrentPos = outT;

        sSecretCourseLastRot    = sSecretCourseCurrentRot;
        sSecretCourseCurrentRot = outR;
    } else {
        outT.y = 0.0f;

        f32 theta = 0.0f;
        f32 phi     = 0.0f;
        f32 boundsX = 0.0f;
        f32 boundsZ = 0.0f;

        f32 horizontalRadius;
        f32 verticalRadius;
        if (sSecretCourseFluddless) {
            horizontalRadius = sSecretCourseVertical ? 600.0f : 800.0f;
            verticalRadius   = sSecretCourseVertical ? 400.0f : 200.0f;
        } else {
            horizontalRadius = sSecretCourseVertical ? 900.0f : 1200.0f;
            verticalRadius   = sSecretCourseVertical ? 600.0f : 300.0f;
        }

        do {
            theta = 2.0f * M_PI * randLerp();
            phi   = acosf(2.0f * randLerp() - 1.0f);

            while (fabsf(sPrevPhi - phi) > M_PI * 0.5f) {
                phi = acosf(2.0f * randLerp() - 1.0f);
            }

            // Check if the angle difference is too acute (folding in on itself)
            while (fabsf(sPrevTheta - theta) > M_PI * 0.3f) {
                theta = 2.0f * M_PI * randLerp();
            }

            const f32 adjustX =
                horizontalRadius * outS.x * sinf(theta) * cosf(phi) * mInfo.mExSpacialScale;
            const f32 adjustY =
                verticalRadius * outS.y * sinf(theta) * sinf(phi) * mInfo.mExSpacialScale +
                (sSecretCourseVertical ? 50.0f : 0.0f);
            const f32 adjustZ = horizontalRadius * outS.z * cosf(theta) * mInfo.mExSpacialScale;

            outT.x = sSecretCourseCurrentPos.x + adjustX;
            outT.y = sSecretCourseCurrentPos.y + adjustY;
            outT.z = sSecretCourseCurrentPos.z + adjustZ;

            if (outT.y < 500.0f) {
                outT.y = 500.0f + (500.0f - outT.y);
            }

            boundsX =
                gpMapCollisionData->mAreaSizeX - (1024.0f * 2 + 1024.0f * 2 * sinf(theta) * outS.z);
            boundsZ =
                gpMapCollisionData->mAreaSizeZ - (1024.0f * 2 + 1024.0f * 2 * cosf(theta) * outS.z);

            sPrevTheta = theta;
        } while (fabsf(outT.x) >= boundsX || fabsf(outT.z) >= boundsZ);

        sSecretCourseLastPos    = sSecretCourseCurrentPos;
        sSecretCourseCurrentPos = outT;

        OSReport("obj: %s\n", mInfo.mObjectType);

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
    }

    mActor->mTranslation = outT;
    mActor->mRotation    = outR;
    mActor->mScale       = outS;
}

void Randomizer::SMSSolver::solveExItemInPost(TItem *item, TMapCollisionData &collision) {
    HitActorInfo &actorInfo = getRandomizerInfo(item);

    if (actorInfo.mIsShineObj) {
        if (sSecretCourseHasEndPlatform) {
            f32 sampleY = sSecretCourseShinePos.y;
            f32 roofY;
            while (true) {
                const TBGCheckData *sample;
                sampleY = collision.checkGround(sSecretCourseShinePos.x, 1000000.0f,
                                                sSecretCourseShinePos.z, 0, &sample);

                if (sample == &TMapCollisionData::mIllegalCheckData ||
                    (sample->mType == 1536 || sample->mType == 2048)) {
                    sampleY = sSecretCourseCurrentPos.y;
                    break;
                }

                break;
            }

            item->mTranslation.x = sSecretCourseShinePos.x;
            item->mTranslation.y = sampleY + 300.0f;
            item->mTranslation.z = sSecretCourseShinePos.z;
        } else {
            item->mTranslation = sSecretCourseCurrentPos;
            item->mTranslation.y += 200.0f;
        }

        if (Randomizer::isRandomScale()) {
            getRandomizedScale(item->mScale, actorInfo);
        }

        sLastItemPos = item->mTranslation;

        return;
    }

    OSReport("Item: %s\n", actorInfo.mObjectType);

    if (STR_EQUAL(actorInfo.mObjectType, "CoinRed") || false) {
        if (sRedCoinsPlaced >= 8) {
            return;
        }

        size_t objCount;
        THitActor **objs = getHitActors(objCount);

        OSReport("(Red Coin) objCount: %d\n", objCount);

        for (size_t i = (objCount / 8) * sRedCoinsPlaced; i < objCount; i++) {
            THitActor *actor = objs[i];
            HitActorInfo &actorInfo = getRandomizerInfo(actor);

            if (!actorInfo.mIsBaseObj) {
                continue;
            }

            item->mTranslation = actor->mTranslation;

            if (STR_EQUAL(actorInfo.mObjectType, "Umaibou")) {
                item->mTranslation.y += 600.0f;
            }
            else {
                item->mTranslation.y += 400.0f;
            }
            break;
        }

        sRedCoinsPlaced++;
        return;
    }

    do {
        sampleRandomFloor(collision, item->mTranslation, item->mRotation, item->mScale);
        item->mTranslation.y += 400.0f;
    } while (PSVECDistance(item->mTranslation, sLastItemPos) < 5000.0f);

    sLastItemPos = item->mTranslation;
}

void Randomizer::SMSSolver::solveExPost(TMapCollisionData &collision) {
    if (sRedCoinSwitch) {
        *(u32 *)((u8 *)sRedCoinSwitch + 0x138) *= 2;
    }

    for (int i = 0; i < sExItemsLoaded; i++) {
        TItem *item = sExItems[i];
        if (item == nullptr) {
            continue;
        }

        solveExItemInPost(item, collision);
    }
}
