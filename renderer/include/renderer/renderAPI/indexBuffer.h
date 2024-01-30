//
// Created by Jean on 1/30/2024.
//

#pragma once

#include <cstdint>

class indexBuffer {
public:
    indexBuffer(const uint32_t *indices, uint32_t count);

    ~indexBuffer();

    void bind() const;

    void unBind() const;

};