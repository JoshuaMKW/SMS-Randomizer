#include <Dolphin/GX.h>
#include <Dolphin/types.h>

#include <JSystem/JDrama/JDRActor.hxx>
#include <JSystem/JDrama/JDRNameRefGen.hxx>

#include <SMS/rand.h>
#include <SMS/raw_fn.hxx>
#include <SMS/macros.h>
#include <SMS/Strategic/Strategy.hxx>
#include <SMS/Map/Map.hxx>
#include <SMS/Map/MapCollisionData.hxx>
#include <SMS/Map/MapCollisionStatic.hxx>
#include <SMS/System/MarDirector.hxx>

#include <BetterSMS/libs/constmath.hxx>
#include <BetterSMS/module.hxx>

#include "actorinfo.hxx"
#include "seed.hxx"
#include "settings.hxx"

extern u32 getColorSeed();

#define _SHIFTL(v, s, w)((u32)(((u32)(v) & ((0x01 << (w)) - 1)) << (s)))  //mask the first w bits of v before lshifting

static void randomizeGXTevColor(u8 tevstage, GXColor *color) {
    if (Randomizer::isRandomColors()) {
        const u32 seed = Randomizer::getGameSeed() * 0x41C64E6D + 0x3039;
        *(u32 *)color ^= (seed * 0x41C64E6D) & 0xFFFFFF00;
    }

    u32 reg;

    reg = (_SHIFTL((0xE0 + (tevstage << 1)), 24, 8) | (_SHIFTL(color->a, 12, 8)) | (color->r & 0xFF));
    GX_LOAD_BP_REG(reg);

    reg = (_SHIFTL((0xE1 + (tevstage << 1)), 24, 8) | (_SHIFTL(color->g, 12, 8)) | (color->b & 0xFF));
    GX_LOAD_BP_REG(reg);

    GX_LOAD_BP_REG(reg);
    GX_LOAD_BP_REG(reg);
}
SMS_PATCH_B(SMS_PORT_REGION(0x80361510, 0, 0, 0), randomizeGXTevColor);

static void randomizeTevBlock2ColorS10(J3DTevBlock2 *block, u8 tevstage, GXColorS10 *color) {
    if (Randomizer::isRandomColors()) {
        const u32 seed = getColorSeed();
        ((u32 *)color)[0] ^= (seed * 0x41C64E6D);
        ((u32 *)color)[1] ^= (seed * 0x51E3102A) & 0xFFFFFF00;
    }

    reinterpret_cast<GXColorS10 *>(((u8 *)block) + 0x10)[tevstage] = *color;
}
SMS_PATCH_B(SMS_PORT_REGION(0x802DC35C, 0, 0, 0), randomizeTevBlock2ColorS10);
SMS_PATCH_B(SMS_PORT_REGION(0x802DC378, 0, 0, 0), randomizeTevBlock2ColorS10);

static void randomizeTevBlock4ColorS10(J3DTevBlock4 *block, u8 tevstage, GXColorS10 *color) {
    if (Randomizer::isRandomColors()) {
        const u32 seed = getColorSeed();
        ((u32 *)color)[0] ^= (seed * 0x41C64E6D);
        ((u32 *)color)[1] ^= (seed * 0x51E3102A) & 0xFFFFFF00;
    }

    reinterpret_cast<GXColorS10 *>(((u8 *)block) + 0x3E)[tevstage] = *color;
}
SMS_PATCH_B(SMS_PORT_REGION(0x802DBF88, 0, 0, 0), randomizeTevBlock4ColorS10);
SMS_PATCH_B(SMS_PORT_REGION(0x802DBFA4, 0, 0, 0), randomizeTevBlock4ColorS10);

static void randomizeTevBlock16ColorS10(J3DTevBlock16 *block, u8 tevstage, GXColorS10 *color) {
    if (Randomizer::isRandomColors()) {
        const u32 seed = getColorSeed();
        ((u32 *)color)[0] ^= (seed * 0x41C64E6D);
        ((u32 *)color)[1] ^= (seed * 0x51E3102A) & 0xFFFFFF00;
    }

    reinterpret_cast<GXColorS10 *>(((u8 *)block) + 0xD6)[tevstage] = *color;
}
SMS_PATCH_B(SMS_PORT_REGION(0x802DBBB4, 0, 0, 0), randomizeTevBlock16ColorS10);
SMS_PATCH_B(SMS_PORT_REGION(0x802DBBD0, 0, 0, 0), randomizeTevBlock16ColorS10);

static void randomizeTevBlock2KColorS10(J3DTevBlock2 *block, u8 tevstage, GXColor *color) {
    if (Randomizer::isRandomColors()) {
        const u32 seed = getColorSeed();
        *(u32 *)color ^= (seed * 0x41C64E6D) & 0xFFFFFF00;
    }

    reinterpret_cast<GXColor *>(((u8 *)block) + 0x41)[tevstage] = *color;
}
SMS_PATCH_B(SMS_PORT_REGION(0x802DC3A8, 0, 0, 0), randomizeTevBlock2KColorS10);
SMS_PATCH_B(SMS_PORT_REGION(0x802DC3BC, 0, 0, 0), randomizeTevBlock2KColorS10);

static void randomizeTevBlock4KColorS10(J3DTevBlock4 *block, u8 tevstage, GXColor *color) {
    if (Randomizer::isRandomColors()) {
        const u32 seed = getColorSeed();
        *(u32 *)color ^= (seed * 0x41C64E6D) & 0xFFFFFF00;
    }

    reinterpret_cast<GXColor *>(((u8 *)block) + 0x5E)[tevstage] = *color;
}
SMS_PATCH_B(SMS_PORT_REGION(0x802DBFD4, 0, 0, 0), randomizeTevBlock4KColorS10);
SMS_PATCH_B(SMS_PORT_REGION(0x802DBFE8, 0, 0, 0), randomizeTevBlock4KColorS10);

static void randomizeTevBlock16KColorS10(J3DTevBlock16 *block, u8 tevstage, GXColor *color) {
    if (Randomizer::isRandomColors()) {
        const u32 seed = getColorSeed();
        *(u32 *)color ^= (seed * 0x41C64E6D) & 0xFFFFFF00;
    }

    reinterpret_cast<GXColor *>(((u8 *)block) + 0xF6)[tevstage] = *color;
}
SMS_PATCH_B(SMS_PORT_REGION(0x802DBC00, 0, 0, 0), randomizeTevBlock16KColorS10);
SMS_PATCH_B(SMS_PORT_REGION(0x802DBC14, 0, 0, 0), randomizeTevBlock16KColorS10);

#undef _SHIFTL