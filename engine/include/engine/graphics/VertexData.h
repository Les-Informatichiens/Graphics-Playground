//
// Created by Jonathan Richard on 2024-03-07.
//

#pragma once

#include "VertexDataLayout.h"
#include "graphicsAPI/common/Device.h"

namespace graphics
{

class IVertexData {
public:
    virtual ~IVertexData() = default;

    virtual void preparePipelineDesc(GraphicsPipelineDesc& desc) const = 0;
    virtual void allocateVertexBuffer(IDevice& device, uint32_t vertexCount_) = 0;
    virtual void allocateIndexBuffer(IDevice& device, uint32_t indexCount_) = 0;
    [[nodiscard]] virtual std::shared_ptr<IVertexInputState> getVertexInputState() const = 0;
    [[nodiscard]] virtual std::shared_ptr<IBuffer> getVertexBuffer() const = 0;
    [[nodiscard]] virtual std::shared_ptr<IBuffer> getIndexBuffer() const = 0;
    [[nodiscard]] virtual uint32_t getVertexCount() const = 0;
    [[nodiscard]] virtual uint32_t getIndexCount() const = 0;
    [[nodiscard]] virtual uint32_t getVertexSize() const = 0;
    [[nodiscard]] virtual uint32_t getIndexSize() const = 0;

    [[nodiscard]] constexpr IndexFormat getIndexFormat() const
    {
        if (getIndexSize() == 2)
        {
            return IndexFormat::UInt16;
        }
        else if (getIndexSize() == 4)
        {
            return IndexFormat::UInt32;
        }
        else
        {
            throw std::runtime_error("Invalid index size");
        }
    }
};

class VertexData : public IVertexData
{
public:
    VertexData(IDevice& device, const VertexDataLayout& layout, IndexFormat indexFormat_, uint32_t vertexCount_ = 0, uint32_t indexCount_ = 0)
        : vertexCount(vertexCount_), indexCount(indexCount_), vertexBufferHead(0), indexBufferHead(0), indexFormat(indexFormat_)
    {
        VertexInputStateDescBuilder builder;

        builder.beginBinding(0);
        for (const auto& attribute : layout.getAttributes())
        {
            builder.addVertexAttribute(attribute.format, attribute.name, attribute.location);
        }
        builder.endBinding();
        auto desc = builder.build();

        vertexInputState = device.createVertexInputState(desc);

        VERTEX_SIZE = layout.getStride();
        INDEX_SIZE = indexFormat == IndexFormat::UInt16 ? 2 : 4;

        if (vertexCount > 0)
        {
            allocateVertexBuffer(device, vertexCount);
        }

        if (indexCount > 0)
        {
            allocateIndexBuffer(device, indexCount);
        }
    }

    void preparePipelineDesc(GraphicsPipelineDesc &desc) const override
    {
        desc.vertexInputState = vertexInputState;
    }

    void allocateVertexBuffer(IDevice& device, uint32_t vertexCount_) override
    {
        vertexCount = vertexCount_;

        BufferDesc vertexBufferDesc;
        vertexBufferDesc.size = vertexCount * VERTEX_SIZE;
        vertexBufferDesc.data = nullptr;
        vertexBufferDesc.type = BufferDesc::BufferTypeBits::Vertex;
        vertexBufferDesc.storage = ResourceStorage::Shared;
        vertexBuffer = device.createBuffer(vertexBufferDesc);

        vertexBufferHead = 0;
    }

    void allocateIndexBuffer(IDevice& device, uint32_t indexCount_) override
    {
        indexCount = indexCount_;

        BufferDesc indexBufferDesc;
        indexBufferDesc.size = indexCount * INDEX_SIZE;
        indexBufferDesc.data = nullptr;
        indexBufferDesc.type = BufferDesc::BufferTypeBits::Index;
        indexBufferDesc.storage = ResourceStorage::Shared;
        indexBuffer = device.createBuffer(indexBufferDesc);

        indexBufferHead = 0;
    }

    template<typename V>
    void pushVertex(const V& vertex)
    {
        vertexBuffer->data(&vertex, VERTEX_SIZE, vertexBufferHead);
        vertexBufferHead += VERTEX_SIZE;
    }

    template<typename I>
    void pushIndex(I index)
    {
        indexBuffer->data(&index, INDEX_SIZE, indexBufferHead);
        indexBufferHead += INDEX_SIZE;
    }

    template<typename V>
    void pushVertices(const V* vertices, uint32_t count)
    {
        vertexBuffer->data(vertices, VERTEX_SIZE * count, vertexBufferHead);
        vertexBufferHead += VERTEX_SIZE * count;
    }

    template<typename I>
    void pushIndices(const I* indices, uint32_t count)
    {
        indexBuffer->data(indices, INDEX_SIZE * count, indexBufferHead);
        indexBufferHead += INDEX_SIZE * count;
    }

    template<typename V>
    void pushVertices(const std::vector<V>& vertices)
    {
        vertexBuffer->data(vertices.data(), VERTEX_SIZE * vertices.size(), vertexBufferHead);
        vertexBufferHead += VERTEX_SIZE * vertices.size();
    }

    template<typename I>
    void pushIndices(const std::vector<I>& indices)
    {
        indexBuffer->data(indices.data(), INDEX_SIZE * indices.size(), indexBufferHead);
        indexBufferHead += INDEX_SIZE * indices.size();
    }

    [[nodiscard]] std::shared_ptr<IVertexInputState> getVertexInputState() const override { return vertexInputState; }
    [[nodiscard]] std::shared_ptr<IBuffer> getVertexBuffer() const override { return vertexBuffer; }
    [[nodiscard]] std::shared_ptr<IBuffer> getIndexBuffer() const override { return indexBuffer; }
    [[nodiscard]] uint32_t getVertexCount() const override { return vertexCount; }
    [[nodiscard]] uint32_t getIndexCount() const override { return indexCount; }
    [[nodiscard]] uint32_t getVertexSize() const { return VERTEX_SIZE; }
    [[nodiscard]] uint32_t getIndexSize() const { return INDEX_SIZE; }


private:
    std::shared_ptr<IVertexInputState> vertexInputState;
    std::shared_ptr<IBuffer> vertexBuffer;
    std::shared_ptr<IBuffer> indexBuffer;
    uint32_t vertexCount{};
    uint32_t indexCount{};
    uint32_t vertexBufferHead{};
    uint32_t indexBufferHead{};

    IndexFormat indexFormat;

    uint32_t VERTEX_SIZE;
    uint32_t INDEX_SIZE;
};

} // namespace graphics
