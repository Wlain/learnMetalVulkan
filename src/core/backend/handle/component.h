//
// Created by cwb on 2022/9/22.
//

#ifndef LEARNMETALVULKAN_COMPONENT_H
#define LEARNMETALVULKAN_COMPONENT_H
#include <string>
#include <typeindex>
// 所有类的基类，方便后续结点化
class Component
{
public:
    Component();
    virtual ~Component();
    explicit Component(const std::string& name);
    Component(Component&& other) noexcept ;
    [[nodiscard]] const std::string& name() const;
    virtual std::type_index type();

private:
    std::string m_name;
};

#endif // LEARNMETALVULKAN_COMPONENT_H
