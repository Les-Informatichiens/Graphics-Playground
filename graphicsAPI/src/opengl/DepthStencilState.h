//
// Created by Jonathan Richard on 2024-02-08.
//

#pragma once

#include "graphicsAPI/common/DepthStencilState.h"
#include "graphicsAPI/opengl/Context.h"

namespace opengl
{

class DepthStencilState: public IDepthStencilState, public WithContext
{
public:
    explicit DepthStencilState(Context& context, const DepthStencilStateDesc& desc_);

    void bind();

    [[nodiscard]] const DepthStencilStateDesc& getDepthStencilStateDesc() const;
private:
    DepthStencilStateDesc desc;
};

}// namespace opengl
