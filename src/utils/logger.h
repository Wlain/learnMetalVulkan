//
// Created by william on 2022/5/22.
//

#ifndef GRAPHICRENDERENGINE_LOGGER_H
#define GRAPHICRENDERENGINE_LOGGER_H
#include "singleton.h"

#include <cstdio>
#include <cstdlib>
#include <spdlog/spdlog.h>

class Logger final : public Singleton<Logger>
{
public:
    enum class Level : uint8_t
    {
        Info = 0,
        Warn,
        Error,
        Debug,
        Fatal
    };

public:
    Logger();
    ~Logger() override;
    //    void log(Level level, const char* message, ...);
    template <typename... T>
    void log(Level level, T&&... args)
    {
        switch (level)
        {
        case Level::Info:
            m_logger->info(std::forward<T>(args)...);
            break;
        case Level::Warn:
            m_logger->warn(std::forward<T>(args)...);
            break;
        case Level::Error:
            m_logger->error(std::forward<T>(args)...);
            break;
        case Level::Debug:
            m_logger->debug(std::forward<T>(args)...);
            break;
        case Level::Fatal:
            m_logger->critical(std::forward<T>(args)...);
            fatalCallback(std::forward<T>(args)...);
            break;
        default:
            break;
        }
    }

    template <typename... T>
    void fatalCallback(T&&... args)
    {
        const std::string format_str = fmt::format(std::forward<T>(args)...);
        throw std::runtime_error(format_str);
    }

private:
    std::shared_ptr<spdlog::logger> m_logger;
};
#endif // GRAPHICRENDERENGINE_LOGGER_H
