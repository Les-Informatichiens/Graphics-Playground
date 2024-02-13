/*
* Copyright (c) Meta Platforms, Inc. and affiliates.
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "graphicsAPI/common/SamplerState.h"
#include "graphicsAPI/common/Common.h"


bool SamplerStateDesc::operator==(const SamplerStateDesc& rhs) const
{
    return (minFilter == rhs.minFilter) && (magFilter == rhs.magFilter) &&
           (mipFilter == rhs.mipFilter) && (addressModeU == rhs.addressModeU) &&
           (addressModeV == rhs.addressModeV) && (addressModeW == rhs.addressModeW) &&
           (depthCompareFunction == rhs.depthCompareFunction) && (mipLodMin == rhs.mipLodMin) &&
           (mipLodMax == rhs.mipLodMax) && (maxAnisotropic == rhs.maxAnisotropic) &&
           (depthCompareEnabled == rhs.depthCompareEnabled);
}

bool SamplerStateDesc::operator!=(const SamplerStateDesc& rhs) const
{
    return !operator==(rhs);
}

size_t std::hash<SamplerStateDesc>::operator()(SamplerStateDesc const& key) const
{
    // clang-format off
 size_t hash = EnumToValue(key.minFilter) |         // 0,1: 1 bit field
               EnumToValue(key.magFilter) << 1 |    // 0,1: 1 bit field
               EnumToValue(key.mipFilter) << 2 |    // 0,1: 1 bit field
               EnumToValue(key.addressModeU) << 4 | // 0,1,2: 2 bit field
               EnumToValue(key.addressModeV) << 6 | // 0,1,2: 2 bit field
               EnumToValue(key.addressModeW) << 8 | // 0,1,2: 2 bit field
               (key.maxAnisotropic - 1) << 10 | // subtract 1 so it fits 4 bits
               key.mipLodMin << 14 |                     // [0, 15]: 4 bit field
               key.mipLodMax << 18 |                     // [0, 15]: 4 bit field
               EnumToValue(key.depthCompareFunction) << 22 | // [0, 7]: 3 bit field
               key.depthCompareEnabled << 25;            // 0,1: 1 bit field
    // clang-format on

    return hash;
}
