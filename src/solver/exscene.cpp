#include "p_common.hxx"

using namespace Randomizer;

//--------------------------------------------------------------------------
// ExScene Solver
//--------------------------------------------------------------------------

static void solveUmaibou(const JDrama::TActor *actor, const HitActorInfo &info, TVec3f &outT,
                         TVec3f &outR, TVec3f &outS) {
    if (Randomizer::isRandomScale()) {
        getRandomizedScale(outS, info);
    } else {
        outS = actor->mScale;
    }

    if (info.mIsShineObj) {
        if (sSecretCourseHasEndPlatform) {
            f32 sampleY = sSecretCourseShinePos.y;
            f32 roofY;
            while (true) {
                const TBGCheckData *roof;
                roofY = gpMapCollisionData->checkRoof(sSecretCourseShinePos.x, sampleY,
                                                      sSecretCourseShinePos.z, 0, &roof);
                const TBGCheckData *sample;
                sampleY = gpMapCollisionData->checkGround(sSecretCourseShinePos.x, roofY,
                                                          sSecretCourseShinePos.z, 0, &sample);

                if (sample == &TMapCollisionData::mIllegalCheckData ||
                    (sample->mType == 1536 || sample->mType == 2048)) {
                    sampleY = sSecretCourseCurrentPos.y + 200.0f;
                    break;
                }

                if (roofY - sampleY > 300.0f || roof->isWaterSurface()) {
                    sampleY = sSecretCourseShinePos.y;
                    break;
                }

                sampleY = roofY + 10.0f;
            }

            outT.x = sSecretCourseShinePos.x;
            outT.y = sampleY;
            outT.z = sSecretCourseShinePos.z;
        } else {
            outT = sSecretCourseCurrentPos;
            outT.y += 200.0f;
        }

        getRandomizedRotation(outR, info);
        return;
    }

    if (info.mIsSwitchObj) {
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
        return;
    }

    TVec3f pprevObjPos = sSecretCourseLastPos;
    TVec3f prevObjPos  = sSecretCourseCurrentPos;

    TVec3f pprevObjRot = sSecretCourseLastRot;
    TVec3f prevObjRot  = sSecretCourseCurrentRot;

    Mtx rot;

    float xzDistance = 0;
    float minXZDistance;
    if (sSecretCourseVertical) {
        minXZDistance = sSecretCourseFluddless ? 300.0f : 600.0f;
    } else {
        minXZDistance = sSecretCourseFluddless ? 400.0f : 800.0f;
    }
    minXZDistance *= scaleLinearAtAnchor(info.mExSpacialScale, 0.75f, 1.0f);

    const f32 theta = 2.0f * M_PI * randLerp();  // Random angle around the sphere
    const f32 phi =
        acosf(2.0f * randLerp() - 1.0f);  // Random polar angle for uniform sampling on a sphere

    f32 horizontalRadius = sSecretCourseVertical ? 600.0f : 800.0f;
    f32 verticalRadius   = sSecretCourseVertical ? 400.0f : 200.0f;

    horizontalRadius *= info.mExSpacialScale;
    verticalRadius *= info.mExSpacialScale;

    // Calculate the point on the sphere surface
    const f32 sinPhi  = sinf(phi);
    const f32 adjustX = horizontalRadius * outS.x * sinPhi * cosf(theta);
    const f32 adjustY = verticalRadius * outS.y * cosf(phi);
    const f32 adjustZ = horizontalRadius * outS.z * sinPhi * sinf(theta);

    outT.x = prevObjPos.x + adjustX;
    outT.y = prevObjPos.y + adjustY;
    outT.z = prevObjPos.z + adjustZ;

    sSecretCourseLastPos    = sSecretCourseCurrentPos;
    sSecretCourseCurrentPos = outT;

    sSecretCourseLastRot    = sSecretCourseCurrentRot;
    sSecretCourseCurrentRot = outR;

    // For the spinning pillar blocks in secret courses like Ricco Harbor
    // we want to have future objects orient at the poles
    TVec3f dir = outT - prevObjPos;

    TVec3f dir_n;
    PSVECNormalize(dir, dir_n);

    TVec3f dir_scaled = dir_n;
    dir_scaled.scale(300.0f * info.mExSpacialScale * ((actor->mScale.x + actor->mScale.z) * 0.5f));

    sSecretCourseCurrentPos += dir_scaled;

    TVec3f up = fabsf(dir_n.y) < 0.999f ? TVec3f::up() : TVec3f::forward();

    Mtx mtx;
    Matrix::normalToRotation(dir, TVec3f::up(), mtx);

    TVec3f t, s;
    Matrix::decompose(mtx, t, outR, s);
}

void SMSSolver::solveExStageObject(TMapCollisionData &collision) {
    TVec3f outT;
    TVec3f outR;
    TVec3f outS;

    if (Randomizer::isRandomScale()) {
        getRandomizedScale(outS, mInfo);
    } else {
        outS = mActor->mScale;
    }

    if (mInfo.mIsShineObj) {
        if (sSecretCourseHasEndPlatform) {
            f32 sampleY = sSecretCourseShinePos.y;
            f32 roofY;
            while (true) {
                const TBGCheckData *roof;
                roofY = collision.checkRoof(sSecretCourseShinePos.x, sampleY,
                                            sSecretCourseShinePos.z, 0, &roof);
                const TBGCheckData *sample;
                sampleY = collision.checkGround(sSecretCourseShinePos.x, roofY,
                                                sSecretCourseShinePos.z, 0, &sample);

                if (sample == &TMapCollisionData::mIllegalCheckData ||
                    (sample->mType == 1536 || sample->mType == 2048)) {
                    sampleY = sSecretCourseCurrentPos.y + 200.0f;
                    break;
                }

                if (roofY - sampleY > 300.0f || roof->isWaterSurface()) {
                    sampleY = sSecretCourseShinePos.y;
                    break;
                }

                sampleY = roofY + 10.0f;
            }

            outT.x = sSecretCourseShinePos.x;
            outT.y = sampleY;
            outT.z = sSecretCourseShinePos.z;
        } else {
            outT = sSecretCourseCurrentPos;
            outT.y += 200.0f;
        }

        getRandomizedRotation(outR, mInfo);

        mActor->mTranslation = outT;
        mActor->mRotation    = outR;
        mActor->mScale       = outS;
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

    if (mInfo.mIsExLinear) {
        Mtx rot;

        do {
            float xzDistance = 0;
            float minXZDistance;
            if (sSecretCourseVertical) {
                minXZDistance = sSecretCourseFluddless ? 300.0f : 600.0f;
            } else {
                minXZDistance = sSecretCourseFluddless ? 400.0f : 800.0f;
            }
            minXZDistance *= scaleLinearAtAnchor(mInfo.mExSpacialScale, 0.75f, 1.0f);

            do {
                const f32 theta = 2.0f * M_PI * randLerp();
                const f32 phi   = acosf(2.0f * randLerp() - 1.0f);

                f32 horizontalRadius;
                f32 verticalRadius;
                if (sSecretCourseFluddless) {
                    horizontalRadius = sSecretCourseVertical ? 600.0f : 800.0f;
                    verticalRadius   = sSecretCourseVertical ? 400.0f : 200.0f;
                } else {
                    horizontalRadius = sSecretCourseVertical ? 1100.0f : 1400.0f;
                    verticalRadius   = sSecretCourseVertical ? 800.0f : 500.0f;
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
            } while (xzDistance < minXZDistance);

            if (outT.y < 500.0f) {
                outT.y = 500.0f + (500.0f - outT.y);
            }

        } while (PSVECDistance(prevObjPos, outT) > PSVECDistance(pprevObjPos, outT) + 500.0f);

        sSecretCourseLastPos    = sSecretCourseCurrentPos;
        sSecretCourseCurrentPos = outT;

        sSecretCourseLastRot    = sSecretCourseCurrentRot;
        sSecretCourseCurrentRot = outR;

        TVec3f dir = outT - prevObjPos;

        TVec3f dir_n;
        PSVECNormalize(dir, dir_n);

        TVec3f up = fabsf(dir_n.y) < 0.999f ? TVec3f::up() : TVec3f::forward();

        Mtx mtx;
        Matrix::normalToRotation(dir, TVec3f::up(), mtx);

        TVec3f t, s;
        Matrix::decompose(mtx, t, outR, s);
    } else {
        outT.y = 0.0f;

        const f32 theta = 2.0f * M_PI * randLerp();
        const f32 phi   = acosf(2.0f * randLerp() - 1.0f);

        f32 horizontalRadius;
        f32 verticalRadius;
        if (sSecretCourseFluddless) {
            horizontalRadius = sSecretCourseVertical ? 600.0f : 800.0f;
            verticalRadius   = sSecretCourseVertical ? 400.0f : 200.0f;
        } else {
            horizontalRadius = sSecretCourseVertical ? 1100.0f : 1400.0f;
            verticalRadius   = sSecretCourseVertical ? 800.0f : 500.0f;
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
    }

    mActor->mTranslation = outT;
    mActor->mRotation    = outR;
    mActor->mScale       = outS;
}