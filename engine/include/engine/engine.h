//
// Created by Jonathan Richard on 2024-01-29.
//
#pragma once
#include "graphicsAPI/common/Device.h"
#include <memory>
#include <utility>

class engine
{
public:

    explicit engine(IDevice& graphicsDevice);;

    void update(float dt) const;
    void render() const;

private:

    IDevice& device;
};