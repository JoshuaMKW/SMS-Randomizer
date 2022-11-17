#include <Dolphin/types.h>

#include <SMS/Player/Mario.hxx>
#include <BetterSMS/libs/constmath.hxx>
#include <BetterSMS/module.hxx>

#include "seed.hxx"
#include "settings.hxx"

void setPlayerInitialHealth(TMario* player, bool isMario) {
    if (!isMario || !gRandomizeHPDamageSetting.getBool())
        return;

    player->mHealth = lerp<u16>(1, 8, randLerp());
}

static void randomizeDecHP(TMario *player, int loss) {
    if (gRandomizeHPDamageSetting.getBool()) {
        loss = lerp<int>(1, 7, randLerp());
    }
    player->decHP(loss);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80240200, 0, 0, 0), randomizeDecHP);
SMS_PATCH_BL(SMS_PORT_REGION(0x80242B10, 0, 0, 0), randomizeDecHP);
SMS_PATCH_BL(SMS_PORT_REGION(0x8024EE90, 0, 0, 0), randomizeDecHP);
SMS_PATCH_BL(SMS_PORT_REGION(0x8024FA34, 0, 0, 0), randomizeDecHP);
SMS_PATCH_BL(SMS_PORT_REGION(0x8025A340, 0, 0, 0), randomizeDecHP);

//static void randomizeIncHP(TMario *player, int loss) {
//    if (gRandomizeHPDamageSetting.getBool()) {
//        loss = lerp<int>(0, 2, randLerp());
//    }
//    player->decHP(loss);
//}
//SMS_PATCH_BL(SMS_PORT_REGION(0x80240200, 0, 0, 0), randomizeIncHP);
//SMS_PATCH_BL(SMS_PORT_REGION(0x80242B10, 0, 0, 0), randomizeIncHP);
//SMS_PATCH_BL(SMS_PORT_REGION(0x8024EE90, 0, 0, 0), randomizeIncHP);
//SMS_PATCH_BL(SMS_PORT_REGION(0x8024FA34, 0, 0, 0), randomizeIncHP);
//SMS_PATCH_BL(SMS_PORT_REGION(0x8025A340, 0, 0, 0), randomizeIncHP);