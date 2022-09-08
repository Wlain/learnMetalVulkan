//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_SHADERVK_H
#define LEARNMETALVULKAN_SHADERVK_H
#include "shader.h"
class ShaderVk : public Shader
{
public:
    using Shader::Shader;
    ~ShaderVk() override = default;
};

#endif // LEARNMETALVULKAN_SHADERVK_H
