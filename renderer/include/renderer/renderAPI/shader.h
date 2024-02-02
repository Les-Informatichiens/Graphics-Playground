//
// Created by Jean on 1/30/2024.
//

#pragma once

#include <string>

class shader {
public:
    shader(const std::string &vertexSrc, const std::string &fragmentSrc);

    ~shader();

    void bind() const;

    void unBind() const;


};
