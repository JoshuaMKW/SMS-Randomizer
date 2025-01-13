#pragma once

#include <SMS/Map/MapCollisionStatic.hxx>

#include <BetterSMS/libs/triangle.hxx>
#include <BetterSMS/libs/constmath.hxx>
#include <BetterSMS/module.hxx>

namespace Surface {

    inline bool isDeathRelated(const TBGCheckData &data) {
        auto type = data.mType & 0xFFF;
        return type == 1536 || type == 2048;
    }

    inline bool isExitRelated(const TBGCheckData &data) {
        auto type = data.mType & 0xFFF;
        return type == 768 || type == 1536 || type == 2048;
    }

    inline bool isWaterRelated(const TBGCheckData &data) {
        auto type = data.mType & 0xFFF;
        return data.isWaterSurface() || type == 267 || type == 33035;
    }

}  // namespace Surface