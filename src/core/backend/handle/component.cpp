//
// Created by cwb on 2022/9/22.
//

#include "component.h"
Component::Component() = default;
Component::~Component() = default;
Component::Component(const std::string& name)
{
}
Component::Component(Component&& other)  noexcept = default;

const std::string& Component::name() const
{
    return m_name;
}

std::type_index Component::type()
{
    return typeid(void);
}
