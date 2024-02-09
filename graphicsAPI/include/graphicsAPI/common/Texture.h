//
// Created by Jonathan Richard on 2024-02-02.
//

#pragma once

#include "Common.h"
#include "TextureStructures.h"

#include <cstdint>

enum class TextureType : uint8_t {
    Invalid,
    Texture2D,
    Texture2DArray,
    Texture3D,
    TextureCube
};

/**
 * @brief Descriptor for internal texture creation methods used in IGL
 *
 *  width              - width of the texture
 *  height             - height of the texture
 *  depth              - depth of the texture
 *  numLayers          - Number of layers for array texture
 *  numSamples         - Number of samples for multisampling
 *  usage              - Bitwise flag for containing a mask of TextureUsageBits
 *  options            - Bitwise flag for containing other options
 *  numMipLevels       - Number of mipmaps to generate
 *  format             - Internal texture format type
 *  storage            - Internal resource storage type
 */
struct TextureDesc {
    /**
   * @brief Bitwise flags for texture usage
   *
   *  Sampled - Can be used as read-only texture in vertex/fragment shaders
   *  Storage - Can be used as read/write storage texture in vertex/fragment/compute shaders
   *  Attachment - Can be bound for render target
   */
    enum TextureUsageBits : uint8_t {
        Sampled = 1 << 0,
        Storage = 1 << 1,
        Attachment = 1 << 2,
    };

    using TextureUsage = uint8_t;

    size_t width = 1;
    size_t height = 1;
    size_t depth = 1;
    size_t numLayers = 1;
    size_t numSamples = 1;
    TextureUsage usage = 0;
    size_t numMipLevels = 1;
    TextureType type = TextureType::Invalid;
    TextureFormat format = TextureFormat::Invalid;
    ResourceStorage storage = ResourceStorage::Invalid;

    bool operator==(const TextureDesc& rhs) const;
    bool operator!=(const TextureDesc& rhs) const;

    /**
   * @brief Utility to create a new 2D texture
   *
   * @param format The format of the texture
   * @param width  The width of the texture
   * @param height The height of the texture
   * @param usage A combination of TextureUsage flags
   * @param debugName An optional debug name
   * @return TextureDesc
   */
    static TextureDesc new2D(TextureFormat format,
                             size_t width,
                             size_t height,
                             TextureUsage usage) {
        return TextureDesc{width,
                           height,
                           1,
                           1,
                           1,
                           usage,
                           1,
                           TextureType::Texture2D,
                           format,
                           ResourceStorage::Invalid};
    }


    /**
   * @brief Utility to calculate maximum mipmap level support
   *
   * @param width  The width of the texture
   * @param height The height of the texture
   * @return uint32_t
   */
    static uint32_t calcNumMipLevels(size_t width, size_t height);
};

enum class TextureCubeFace : uint8_t { PosX = 0, NegX, PosY, NegY, PosZ, NegZ };

class ITexture
{
public:
    virtual ~ITexture() = default;

    virtual void upload(const void* data, const TextureRangeDesc& range, size_t bytesPerRow) const = 0;
    virtual void uploadCube(const void* data, TextureCubeFace face, const TextureRangeDesc& range, size_t bytesPerRow) const = 0;

    [[nodiscard]] virtual float getAspectRatio() const = 0;
    [[nodiscard]] virtual size_t getWidth() const = 0;
    [[nodiscard]] virtual size_t getHeight() const = 0;
    [[nodiscard]] virtual size_t getDepth() const = 0;

    [[nodiscard]] virtual size_t getNumLayers() const = 0;
    [[nodiscard]] virtual size_t getSamples() const = 0;
    [[nodiscard]] virtual size_t getNumMipLevels() const = 0;
    [[nodiscard]] virtual bool isRequiredGenerateMipmap() const = 0;
    virtual void generateMipmap() const = 0;

    [[nodiscard]] virtual TextureFormatProperties getProperties() const = 0;
    [[nodiscard]] virtual size_t getUsage() const = 0;

    [[nodiscard]] virtual TextureType getType() const = 0;
    [[nodiscard]] virtual TextureFormat getFormat() const = 0;

    [[nodiscard]] virtual TextureRangeDesc getFullRange(size_t mipLevel, size_t numMipLevels) const = 0;
    [[nodiscard]] virtual std::pair<bool, bool> validateRange(const TextureRangeDesc& range) const = 0;
};
