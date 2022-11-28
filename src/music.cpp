#include <Dolphin/types.h>

#include <SMS/MSound/MSBGM.hxx>
#include <SMS/MSound/MSoundSESystem.hxx>
#include <SMS/raw_fn.hxx>

#include <BetterSMS/libs/container.hxx>
#include <BetterSMS/libs/constmath.hxx>
#include <BetterSMS/module.hxx>
#include <BetterSMS/music.hxx>

#include "seed.hxx"
#include "settings.hxx"

// clang-format off
static const u8 sValidAreaIDs[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9,
	13, 14, 15, 16,
	20, 21, 22, 23, 24,
	29, 30, 31, 32, 33,
	40, 41, 42,
	44,
	46, 47, 48,
	50, 51, 52,
	55, 56, 57, 58, 59, 60
};
// clang-format on

static u32 sMusicChoices[] = {
    MSStageInfo::BGM_AIRPORT,      MSStageInfo::BGM_BIANCO,      MSStageInfo::BGM_CASINO,
    MSStageInfo::BGM_CORONA,       MSStageInfo::BGM_DELFINO,     MSStageInfo::BGM_DOLPIC,
    MSStageInfo::BGM_EVENT,        MSStageInfo::BGM_EXTRA,       MSStageInfo::BGM_MAMMA,
    MSStageInfo::BGM_MARE_SEA,     MSStageInfo::BGM_MAREVILLAGE, MSStageInfo::BGM_MERRY_GO_ROUND,
    MSStageInfo::BGM_MONTE_NIGHT,  MSStageInfo::BGM_MONTE_ONSEN, MSStageInfo::BGM_MONTE_RESCUE,
    MSStageInfo::BGM_MONTEVILLAGE, MSStageInfo::BGM_PINNAPACO,   MSStageInfo::BGM_PINNAPACO_SEA,
    MSStageInfo::BGM_RICCO,        MSStageInfo::BGM_SHILENA,     MSStageInfo::BGM_SKY_AND_SEA};

 //static void setRandomMSoundEnterStage(u8 area, u8 episode) {
 //    srand32((area * 0x41C64E6D + 0x3039) * getGameSeed());
 //    area = sMusicChoices[lerp<u32>(0, 38, randLerp())];
 //    setMSoundEnterStage__10MSMainProcFUcUc(area, episode);
 //}
 //SMS_PATCH_BL(SMS_PORT_REGION(0x802B7A4C, 0, 0, 0), setRandomMSoundEnterStage);

static void setRandomMSoundEnterStage(u32 musicID) {
    if (gRandomizeMusicSetting.getBool()) {
        srand32((gpMarDirector->mAreaID * 0x41C64E6D + 0x3039) * getGameSeed());
        MSBgm::startBGM(sMusicChoices[lerp<u32>(0, 20, randLerp())]);
    }
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802BB760, 0, 0, 0), setRandomMSoundEnterStage);
SMS_PATCH_BL(SMS_PORT_REGION(0x802BB780, 0, 0, 0), setRandomMSoundEnterStage);
SMS_PATCH_BL(SMS_PORT_REGION(0x802BB7B0, 0, 0, 0), setRandomMSoundEnterStage);