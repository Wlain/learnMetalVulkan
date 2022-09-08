//
// Created by william on 2022/5/22.
//

#ifndef GRAPHICRENDERENGINE_COMMONMACRO_H
#define GRAPHICRENDERENGINE_COMMONMACRO_H

#include "logger.h"

#include <chrono>
#include <string>
#include <thread>

#define LOG_HELPER(level, ...) \
    Logger::getInstance().log(level, "[re]in File:" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + " in func:" + std::string(__FUNCTION__) + "]:" + __VA_ARGS__)

#define LOG_DEBUG(...) LOG_HELPER(Logger::Level::Debug, __VA_ARGS__)
#define LOG_INFO(...) LOG_HELPER(Logger::Level::Info, __VA_ARGS__)
#define LOG_WARN(...) LOG_HELPER(Logger::Level::Warn, __VA_ARGS__)
#define LOG_ERROR(...) LOG_HELPER(Logger::Level::Error, __VA_ARGS__)
#define LOG_FATAL(...) LOG_HELPER(Logger::Level::Fatal, __VA_ARGS__)

/// 安卓平台下导出动态库
#ifdef __cplusplus
    #define EXPORT extern "C" __attribute__((visibility("default")))
#else
    #define EXPORT extern __attribute__((visibility("default")))
#endif
/// __attribute__((visibility("default")))  // 默认，设置为：default之后就可以让外面的类看见了。
/// __attribute__((visibility("hideen")))  // 隐藏

#if defined(NDEBUG) && defined(__GNUC__)
    #define ASSERT_ONLY __attribute__((unused))
#else
    #define ASSERT_ONLY
#endif

#ifdef NDEBUG
    #define ASSERT(statement)
#else
    #define ASSERT(statement) assert(statement)
#endif

#define TO_STRING(s) #s
#define CONNECTION(x, y) x##y

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))
#define VECTOR_SIZE(x) (sizeof(x[0]) * x.size())

#define ENUM_ALIAS(name, member) \
    constexpr auto name##_##member = name::member

#define NO_REF_TYPE(t) std::remove_reference<decltype(*t)>::type
#define MAKE_SHARED(t, ...) std::make_shared<typename NO_REF_TYPE(t)>(__VA_ARGS__)
#define MAKE_UNIQUE(t, ...) std::make_unique<typename NO_REF_TYPE(t)>(__VA_ARGS__)

/// math macros
#define MATH_PI 3.1415926f
#define TWO_MATH_PI (MATH_PI * 2.0f)
#define MATH_DEG_TO_RAD(x) ((x)*0.0174532925f)
#define MATH_PIOVER2 1.57079632679489661923f
#define MATH_RANDOM_MINUS1_1() ((2.0f * ((float)rand() / RAND_MAX)) - 1.0f) // 返回一个【-1，,1】之间的随机值
#define MATH_RANDOM_0_1() ((float)rand() / RAND_MAX)                        // 返回一个【0，,1】之间的随机值

// 代码废弃
#define DEPRECATED(X) [[deprecated(X)]]
// 取消字节对齐
#define PACK(__Declaration__) __Declaration__ __attribute__((__packed__))

#endif // GRAPHICRENDERENGINE_COMMONMACRO_H
