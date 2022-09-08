//
// Created by william on 2022/5/22.
//

#ifndef GRAPHICRENDERENGINE_SINGLETON_H
#define GRAPHICRENDERENGINE_SINGLETON_H
#include <type_traits>

template <typename T>
class Singleton
{
protected:
    Singleton() = default;

public:
    static T& getInstance() noexcept(std::is_nothrow_constructible<T>::value)
    {
        static T instance;
        return instance;
    }
    virtual ~Singleton() noexcept = default;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};
#endif // GRAPHICRENDERENGINE_SINGLETON_H
