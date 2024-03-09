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

size_t SamplerStateDescHash::operator()(const SamplerStateDesc& samplerStateDesc) const
{
    // Use the existing hash calculation logic
    size_t hash = EnumToValue(samplerStateDesc.minFilter) |
                  EnumToValue(samplerStateDesc.magFilter) << 1 |
                  EnumToValue(samplerStateDesc.mipFilter) << 2 |
                  EnumToValue(samplerStateDesc.addressModeU) << 4 |
                  EnumToValue(samplerStateDesc.addressModeV) << 6 |
                  EnumToValue(samplerStateDesc.addressModeW) << 8 |
                  (samplerStateDesc.maxAnisotropic - 1) << 10 |
                  samplerStateDesc.mipLodMin << 14 |
                  samplerStateDesc.mipLodMax << 18 |
                  EnumToValue(samplerStateDesc.depthCompareFunction) << 22 |
                  samplerStateDesc.depthCompareEnabled << 25;

    return hash;
}
