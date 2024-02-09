//
// Created by Jonathan Richard on 2024-02-08.
//

#include "graphicsAPI/common/Texture.h"

uint32_t TextureDesc::calcNumMipLevels(size_t width, size_t height) {
    if (width == 0 || height == 0) {
        return 0;
    }

    uint32_t levels = 1;

    while ((width | height) >> levels) {
        levels++;
    }

    return levels;
}

bool TextureDesc::operator==(const TextureDesc& rhs) const {
    return (type == rhs.type) && (format == rhs.format) && (width == rhs.width) &&
           (height == rhs.height) && (depth == rhs.depth) && (numLayers == rhs.numLayers) &&
           (numSamples == rhs.numSamples) && (usage == rhs.usage) &&
           (numMipLevels == rhs.numMipLevels) && (storage == rhs.storage);
}

bool TextureDesc::operator!=(const TextureDesc& rhs) const {
    return !operator==(rhs);
}
