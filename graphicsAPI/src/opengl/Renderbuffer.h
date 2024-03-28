//
// Created by Jonathan Richard on 2024-02-07.
//

#pragma once

#include "Texture.h"
#include "graphicsAPI/opengl/Context.h"

namespace opengl
{

class Renderbuffer : public Texture
{
public:
    explicit Renderbuffer(Context& context, TextureFormat format);
    ~Renderbuffer() override;

    void create(const TextureDesc& desc, bool hasStorageAlready) override;

    void bind() override;
    void bindImage(size_t unit) override;
    void unbind() override;

    [[nodiscard]] TextureType getType() const override;
    [[nodiscard]] size_t getUsage() const override;

    [[nodiscard]] BufferType getBufferType() const override;
    [[nodiscard]] GLuint getHandle() const override;

private:
    GLuint handle = 0;
};

}// namespace opengl