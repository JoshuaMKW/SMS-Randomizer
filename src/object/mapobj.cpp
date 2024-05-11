#include <BetterSMS/libs/constmath.hxx>
#include <System/MarNameRefGen.hxx>
#include <Dolphin/types.h>
#include <JSystem/JSupport/JSUMemoryStream.hxx>

#include <SMS/Manager/LiveManager.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/MapObj/MapObjGeneral.hxx>
#include <SMS/MoveBG/Item.hxx>
#include <SMS/MarioUtil/DrawUtil.hxx>
#include <SMS/macros.h>
#include <SMS/raw_fn.hxx>
#include <SMS/rand.h>

#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"
#include "seed.hxx"
#include "settings.hxx"
#include "solver.hxx"

#define STR_EQUAL(a, b) strcmp(a, b) == 0

#pragma region MapObjBehaviors

static void forceMTXUpdate(MActor *actor) {
    TLiveActor *obj;
    SMS_FROM_GPR(27, obj);

    J3DModel *model = actor->mModel;
    if (!model)
		return;
    
    if (model->mModelData->mDrawMtxData.mJointCount != 0) {
        /*MsMtxSetTRS__FPA4_ffffffffff(model->mJointArray[*model->mModelData->mCurrentJointIndex],
            obj->mTranslation.x, obj->mTranslation.y, obj->mTranslation.z, obj->mRotation.x,
            obj->mRotation.y, obj->mRotation.z, obj->mScale.x, obj->mScale.y, obj->mScale.z);*/
        /*J3DMTXConcatArrayIndexedSrc__FPA4_CfPA3_A4_CfPCUsPA3_A4_fUl(
            (u32 *)0x804045DC, model->mJointArray, model->mModelData->mCurrentJointIndex,
            ((u32 *)model->_64)[model->_7C], model->mModelData->mDrawMtxData.mJointCount);*/
    }
    model->viewCalc();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80217f90, 0, 0, 0), forceMTXUpdate);

void initializeDefaultActorInfo(const TMarDirector& director, HitActorInfo& actorInfo) {
    const char *objectType = actorInfo.mObjectType;
    const char *objectKey = actorInfo.mObjectKey;

    actorInfo.mShouldRandomize = Randomizer::isRandomObjects();
    actorInfo.mIsWaterValid    = false;

    if (STR_EQUAL(objectType, "MapObjSmoke")) {  // Delfino Plaza
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "MapObjFlag")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "MapObjGrass")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "MapObjGrassGroup")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "MapObjChangeStage")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "MapObjStartDemoLoad")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "MapObjStartDemo")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "MapObjWaterSpray")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "MapObjSoundGroup")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "MapObjFloatOnSea")) {
        actorInfo.mIsGroundValid     = false;
        actorInfo.mIsWaterValid      = true;
        actorInfo.mIsUnderwaterValid = false;
    } else if (STR_EQUAL(objectType, "OrangeSeal")) {
        actorInfo.mShouldRandomize   = true;
        actorInfo.mIsGroundValid     = true;
        actorInfo.mIsWallValid       = true;
        actorInfo.mIsSurfaceBound    = true;
        actorInfo.mIsWaterValid      = false;
        actorInfo.mIsUnderwaterValid = false;
        actorInfo.mAdjustRotation    = {90, 0, 0};
    } else if (STR_EQUAL(objectType, "MapStaticObj")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "WoodBox")) {
        actorInfo.mShouldRandomize     = true;
        actorInfo.mShouldResizeUniform = true;
    } else if (STR_EQUAL(objectType, "JellyGate")) {
        actorInfo.mFromSurfaceDist = 5;
        actorInfo.mIsRoofValid     = true;
        actorInfo.mIsWallValid     = true;
        actorInfo.mIsSurfaceBound  = true;
    } else if (STR_EQUAL(objectType, "GateToRicco")) {
        actorInfo.mFromSurfaceDist = 100;
        actorInfo.mIsRoofValid     = true;
        actorInfo.mIsWallValid     = true;
        actorInfo.mIsSurfaceBound  = true;
    } else if (STR_EQUAL(objectType, "Billboard")) {
        actorInfo.mShouldRandomize   = true;
        actorInfo.mAdjustRotation    = {0, 90, 0};
        actorInfo.mIsGroundValid     = false;
        actorInfo.mIsWallValid       = true;
        actorInfo.mIsSurfaceBound    = true;
    } else if (STR_EQUAL(objectType, "MonumentShine")) {
        actorInfo.mShouldRandomize   = true;
        actorInfo.mIsGroundValid     = false;
        actorInfo.mIsWaterValid      = false;
        actorInfo.mIsUnderwaterValid = false;
        actorInfo.mIsRoofValid       = true;
        actorInfo.mIsWallValid       = false;
        actorInfo.mFromSurfaceDist   = 300;
    } else if (STR_EQUAL(objectType, "BellDolpicTV")) {
        actorInfo.mShouldRandomize   = true;
        actorInfo.mIsGroundValid     = false;
        actorInfo.mIsWaterValid      = false;
        actorInfo.mIsUnderwaterValid = false;
        actorInfo.mIsRoofValid       = true;
        actorInfo.mIsWallValid       = false;
        // actorInfo.mFromSurfaceDist = 800;
    } else if (STR_EQUAL(objectType, "BellDolpicPolice")) {
        actorInfo.mShouldRandomize   = true;
        actorInfo.mIsGroundValid     = false;
        actorInfo.mIsWaterValid      = false;
        actorInfo.mIsUnderwaterValid = false;
        actorInfo.mIsRoofValid       = true;
        actorInfo.mIsWallValid       = false;
        // actorInfo.mFromSurfaceDist = 800;
    } else if (STR_EQUAL(objectType, "DemoCannon")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "DokanGate")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "DptCoronaFence")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "MareGate")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "WaterRecoverObj")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "WaterHitHideObj")) {
        //actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectKey, "PalmLeaf 0")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectKey, "PalmLeaf 1")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectKey, "PalmLeaf 2")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectKey, "PalmLeaf 3")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectKey, "PalmLeaf 4")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "FruitBasketEvent")) {
        actorInfo.mShouldResizeXZ = false;
        actorInfo.mShouldResizeY = false;
        if (STR_EQUAL(objectKey, "\x83\x74\x83\x8B\x81\x5B\x83\x63\x82\xA9\x82\xB2\x82\x63\xC5\x0F"
                                 "\xBF\xFE\x44")) {  // Durian Basket
            actorInfo.mIsSurfaceBound  = true;
            actorInfo.mFromSurfaceDist = -100.0f;
            actorInfo.mAdjustRotation  = {0.0f, 0.0f, 90.0f};
        }
    } else if (STR_EQUAL(objectType, "WindmillRoof")) {  // Bianco Hills
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "BiaBridge")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "Amenbo")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "HanaSambo")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "BellWatermill")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "WireTrap")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "CraneRotY")) {  // Ricco Harbor
        actorInfo.mShouldResizeXZ  = false;
        actorInfo.mShouldResizeY  = false;
    } else if (STR_EQUAL(objectType, "RiccoLog")) {
        actorInfo.mIsGroundValid = false;
        actorInfo.mIsWaterValid = true;
    } else if (STR_EQUAL(objectKey, "\x89\xBA\x90\x85\x8D\xF2\x20\x30")) {  // Grille 0
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectKey, "\x89\xBA\x90\x85\x8D\xF2\x20\x31")) {  // Grille 1
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectKey, "submarine")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "SandEgg")) {  // Gelato Beach
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "SandBombBase")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "SandLeafBase")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "coral00")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "coral01")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "LeanMirror")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "FruitHitHideObj")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectKey, "TeethOfJuicer")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "bosshanachan0")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "Shining")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectKey, "fenceRevolve 3")) {  // Pinna
        if (director.mAreaID == 13 && director.mEpisodeID == 2)
            actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "Merrygoround")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "FerrisWheel")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "Viking")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "ShellCup")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "AmiKing")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "Rocket")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "PinnaDoor")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "BalloonKoopaJr")) {  // TODO: Attempt to randomize
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "SirenaShop")) {  // Sirena Beach
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "ChestRevolve")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "Closet")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "PosterTeresa")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "Door")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "PanelRevolve")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "PanelBreak")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "PictureTeresa")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "casinoRoulette")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "ItemSlotDrum")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "SlotDrum")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "SirenaCasinoRoof")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "Donchou")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "SakuCasino")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "RailFence")) {  // Pianta Village
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "FireWanwan")) {  // TODO: Find a way to randomize
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "BossWanwan")) {  // TODO: Find a way to randomize
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "MareEventBumpyWall")) {  // Noki
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "MareEventPoint")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "MareFall")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "CoinFish")) {  // TODO: Find a way to randomize
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "SakuCasino")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "SakuCasino")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectKey, "\x8A\xCF\x97\x97\x8E\xD4\x81\x69\x89\x93\x8C\x69\x81\x6A")) {  // FerrisLOD
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "WaterRecoverObj")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "WaterHitPictureHideObj")) {
        actorInfo.mShouldRandomize = Randomizer::isRandomCollectibles();
        actorInfo.mFromSurfaceDist = 0;
        actorInfo.mAdjustRotation  = {0, 0, 0};
        actorInfo.mShouldRotateXZ  = true;
        actorInfo.mShouldRotateY   = true;
        actorInfo.mIsGroundValid   = true;
        actorInfo.mIsRoofValid     = true;
        actorInfo.mIsWallValid     = true;
        actorInfo.mIsSurfaceBound  = true;
        actorInfo.mIsSprayableObj  = true;
    } else if (STR_EQUAL(objectType, "HideObjPictureTwin")) {
        actorInfo.mShouldRandomize = Randomizer::isRandomCollectibles();
        actorInfo.mFromSurfaceDist = 0;
        actorInfo.mAdjustRotation  = {0, 0, 0};
        actorInfo.mShouldRotateXZ  = true;
        actorInfo.mShouldRotateY   = true;
        actorInfo.mIsGroundValid   = true;
        actorInfo.mIsRoofValid     = true;
        actorInfo.mIsWallValid     = true;
        actorInfo.mIsSurfaceBound  = true;
        actorInfo.mIsSprayableObj  = true;
    } else if (STR_EQUAL(objectType, "SunModel")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "SunsetModel")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "GoalFlag")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "EMario")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "Manhole")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "AnimalMew")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "FishoidA")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "FishoidB")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "FishoidC")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "FishoidD")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "EffectPinnaFunsui")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "FruitsBoat")) {
        actorInfo.mShouldRandomize   = true;
        actorInfo.mIsGroundValid     = false;
        actorInfo.mIsWaterValid      = true;
        actorInfo.mIsUnderwaterValid = false;
        actorInfo.mFromSurfaceDist   = 0;
    } else if (STR_EQUAL(objectType, "FruitsBoatB")) {
        actorInfo.mShouldRandomize   = true;
        actorInfo.mIsGroundValid     = false;
        actorInfo.mIsWaterValid      = true;
        actorInfo.mIsUnderwaterValid = false;
        actorInfo.mFromSurfaceDist   = 0;
    } else if (STR_EQUAL(objectType, "MuddyBoat")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "Umaibou")) {
        actorInfo.mExSpacialScale = 4.0f;
        actorInfo.mIsExLinear = true;
    } else if (STR_EQUAL(objectType, "RedCoinSwitch")) {
        actorInfo.mIsSurfaceBound  = true;
        actorInfo.mIsSwitchObj     = true;
    } else if (STR_EQUAL(objectType, "MapObjSwitch")) {
        actorInfo.mIsSurfaceBound  = true;
        actorInfo.mIsSwitchObj     = true;
    } else if (STR_EQUAL(objectType, "TurboNozzleDoor")) {
        actorInfo.mShouldRandomize = false;
    } else if (STR_EQUAL(objectType, "NozzleBox")) {
        actorInfo.mShouldRandomize     = true;
        actorInfo.mIsGroundValid       = true;
        actorInfo.mIsWaterValid        = false;
        actorInfo.mIsUnderwaterValid   = false;
        actorInfo.mShouldResizeUniform = false;
        actorInfo.mShouldResizeXZ      = false;
        actorInfo.mShouldResizeY       = false;
        actorInfo.mShouldRotateXZ      = false;
        actorInfo.mShouldRotateY       = true;
    } else if (STR_EQUAL(objectType, "Coin")) {
        actorInfo.mShouldRandomize     = true;
        actorInfo.mIsGroundValid       = true;
        actorInfo.mIsWaterValid        = true;
        actorInfo.mIsUnderwaterValid   = true;
        actorInfo.mShouldResizeXZ      = false;
        actorInfo.mShouldResizeY       = false;
        actorInfo.mShouldRotateXZ      = false;
        actorInfo.mShouldRotateY       = false;
    } else if (STR_EQUAL(objectType, "CoinBlue")) {
        actorInfo.mShouldRandomize   = true;
        actorInfo.mIsGroundValid     = true;
        actorInfo.mIsWaterValid      = true;
        actorInfo.mIsUnderwaterValid = true;
        actorInfo.mShouldResizeXZ    = false;
        actorInfo.mShouldResizeY     = false;
        actorInfo.mShouldRotateXZ    = false;
        actorInfo.mShouldRotateY     = false;
    } else if (STR_EQUAL(objectType, "CoinRed")) {
        actorInfo.mShouldRandomize   = true;
        actorInfo.mIsGroundValid     = true;
        actorInfo.mIsWaterValid      = true;
        actorInfo.mIsUnderwaterValid = true;
        actorInfo.mShouldResizeXZ    = false;
        actorInfo.mShouldResizeY     = false;
        actorInfo.mShouldRotateXZ    = false;
        actorInfo.mShouldRotateY     = false;
    } else if (STR_EQUAL(objectType, "NPCPeach")) {
        if (director.mAreaID == 1 && director.mEpisodeID == 1)
            actorInfo.mShouldRandomize     = false;
    } else if (STR_EQUAL(objectType, "Mario")) {
        actorInfo.mShouldRandomize     = !SMS_isExMap__Fv();
        actorInfo.mIsGroundValid       = true;
        actorInfo.mIsWaterValid        = true;
        actorInfo.mIsUnderwaterValid   = true;
        actorInfo.mShouldResizeUniform = true;
        actorInfo.mShouldResizeXZ      = false;
        actorInfo.mShouldResizeY       = false;
        actorInfo.mShouldRotateXZ      = false;
        actorInfo.mShouldRotateY       = true;
        actorInfo.mIsPlayer            = true;
    }
}

#pragma endregion
#pragma region WarpHandlers

static u16 *sStageWarps[32]{};
static u16 sStageWarpsCollected = 0;

void resetStageWarpInfo() { sStageWarpsCollected = 0; }

static void collectStageWarpIDs(u16 *stageWarp, JSUMemoryInputStream *in) {
    load__11TMapObjBaseFR20JSUMemoryInputStream(stageWarp, in);
    sStageWarps[sStageWarpsCollected++] = stageWarp;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x801C173C, 0, 0, 0), collectStageWarpIDs);

static u32 applyRandomStageWarps() {
    auto warpSetting = Randomizer::getRandomWarpsState();
    if (warpSetting == RandomWarpSetting::OFF)
        return SMSGetGameVideoHeight__Fv();

    Randomizer::srand32(Randomizer::levelScramble(Randomizer::getGameSeed(), 0x51324217, false));

    if (warpSetting == RandomWarpSetting::LOCAL) {
        u16 stageWarpIDs[64];
        u32 warpSetFlags = 0;

    #define GET_FLAG(bits, idx) static_cast<bool>((bits & (1 << (idx))) >> idx)
    #define SET_FLAG(bits, idx, flag) static_cast<bool>(bits |= ((1 & flag) << (idx)))

        for (int i = 0; i < sStageWarpsCollected; ++i) {
            while (true) {
                u32 n = lerp<f32>(0.0f, static_cast<f32>(sStageWarpsCollected) - 0.01f,
                                   Randomizer::randLerp());
                if (!GET_FLAG(warpSetFlags, n)) {
                    stageWarpIDs[n] = sStageWarps[i][0x138 / 2];
                    SET_FLAG(warpSetFlags, n, true);
                    break;
                }
            }
        }

        for (int i = 0; i < sStageWarpsCollected; ++i) {
            sStageWarps[i][0x138 / 2] = stageWarpIDs[i];
        }

    #undef GET_FLAG
    #undef SET_FLAG

        return SMSGetGameVideoHeight__Fv();
    }

    // Global warps

    const auto &warpIDs = Randomizer::getWarpIDWhiteList();
    for (int i = 0; i < sStageWarpsCollected; ++i) {
        u32 n = lerp<f32>(0.0f, static_cast<f32>(warpIDs.size()) - 0.01f, Randomizer::randLerp());
        sStageWarps[i][0x138 / 2] = warpIDs.at(n);
    }

    return SMSGetGameVideoHeight__Fv();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802B8B44, 0, 0, 0), applyRandomStageWarps);

static THitActor *collectObjectTypes(const TMarNameRefGen *gen, const char *name) {
    auto *actor = reinterpret_cast<THitActor *>(gen->getNameRef(name));

    HitActorInfo &actorInfo     = getRandomizerInfo(actor);
    actorInfo.mObjectType      = name;

    return actor;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802FA628, 0, 0, 0), collectObjectTypes);

/* Allocate extra memory for a larger capacity of ObjHitChecks */
/* Fixes crashes in Bianco Hills due to too many objs near each other at once */
SMS_WRITE_32(SMS_PORT_REGION(0x8021B340, 0, 0, 0), 0x60638004);  // Allocate about 1.8x memory
SMS_WRITE_32(SMS_PORT_REGION(0x8021B354, 0, 0, 0), 0x38E03000);  // Allocate about 1.8x array slices

#pragma endregion

#undef STR_EQUAL