//
// Created by Jonathan Richard on 2024-01-31.
//

#pragma once

struct CommandBufferDesc
{
};

class ICommandBuffer
{
public:
    virtual void begin() = 0;
    virtual void end() = 0;
};
