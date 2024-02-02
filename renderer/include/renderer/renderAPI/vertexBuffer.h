//
// Created by Jean on 1/30/2024.
//

#pragma once

#include <cstdint>

class vertexBuffer {
public:
    vertexBuffer(const void *data, uint32_t size);

    ~vertexBuffer();

    void bind() const;

    void unBind() const;

};