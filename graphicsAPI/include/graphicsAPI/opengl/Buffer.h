//
// Created by Jonathan Richard on 2024-01-31.
//

#pragma once

#include "Context.h"
#include "graphicsAPI/common/Buffer.h"

namespace opengl
{
    class Buffer : public IBuffer, public WithContext
    {
    public:
        explicit Buffer(Context& context, const BufferDesc& desc);

        virtual void initialize() = 0;

        void map(const void* data, unsigned int size) const override;
        void mapRange(const void* data, unsigned int size, unsigned int offset) const override;
        void unmap() const override;

    private:
        uint32_t id;
    };
}