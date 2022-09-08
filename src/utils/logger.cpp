//
// Created by william on 2022/5/22.
//
#include "logger.h"

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <vector>
Logger::Logger()
{
    auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto rotatingSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("mylog.txt", 1024 * 1024 * 10, 3);
    std::vector<spdlog::sink_ptr> sinks{ stdoutSink, rotatingSink };
    spdlog::init_thread_pool(8192, 1);
    m_logger = std::make_shared<spdlog::async_logger>("Logger", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    spdlog::register_logger(m_logger);
}

Logger::~Logger()
{
    m_logger->flush();
    spdlog::drop_all();
}