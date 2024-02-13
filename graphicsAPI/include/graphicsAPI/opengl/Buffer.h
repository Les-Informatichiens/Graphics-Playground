//
// Created by Jonathan Richard on 2024-01-31.
//

#pragma once

#include "Context.h"
#include "graphicsAPI/common/Buffer.h"

namespace opengl {
class Buffer : public IBuffer, public WithContext
{
public:
    enum class Type : uint8_t
    {
        Attribute,
        Index,
        Uniform,
        Storage
    };

    explicit Buffer(Context& context) : WithContext(context){};

    [[nodiscard]] virtual Type getType() const noexcept = 0;

    virtual void initialize(const BufferDesc& desc) = 0;
};

class ArrayBuffer : public Buffer
{
public:
    explicit ArrayBuffer(Context& context);
    ~ArrayBuffer() override;

    void initialize(const BufferDesc& desc) override;

    void bind() const noexcept;
    void bindBase(uint32_t index) const noexcept;
    void bindRange(uint32_t index, uint32_t offset) const noexcept;
    void unbind() const noexcept;

    void data(const void* data, uint32_t size, uint32_t offset) const override;
    void* map(uint32_t size, uint32_t offset) const override;
    void unmap() const override;

    [[nodiscard]] size_t getSize() const override;

    [[nodiscard]] GLuint getId() const noexcept { return id_; }
    [[nodiscard]] Type getType() const noexcept override { return type_; }
    [[nodiscard]] GLenum getTarget() const noexcept { return target_; }

private:
    void savePreviousBuffer() const;
    void restorePreviousBuffer() const;

private:
    GLuint id_;
    GLenum target_;
    Type type_;
    bool isDynamic_;
    uint32_t size_;

    mutable GLint lastBoundBuffer_ = 0;
};

}// namespace opengl
