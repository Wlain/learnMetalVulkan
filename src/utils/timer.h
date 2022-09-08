//
// Created by william on 2022/5/23.
//

#ifndef GRAPHICRENDERENGINE_TIMER_H
#define GRAPHICRENDERENGINE_TIMER_H

#include <chrono>

class Timer
{
public:
    Timer() :
        m_begin(std::chrono::high_resolution_clock::now()) {}
    ~Timer() = default;

    void reset()
    {
        m_begin = std::chrono::high_resolution_clock::now();
    }

    template <typename Duration = std::chrono::milliseconds>
    int64_t elapsed() const
    {
        return std::chrono::duration_cast<Duration>(std::chrono::high_resolution_clock::now() - m_begin).count();
    }

    int64_t elapsedMicro() const
    {
        return elapsed<std::chrono::milliseconds>();
    }

    int64_t elapsedNano() const
    {
        return elapsed<std::chrono::nanoseconds>();
    }

    int64_t elapsedSecond() const
    {
        return elapsed<std::chrono::seconds>();
    }

    int64_t elapsedMinutes() const
    {
        return elapsed<std::chrono::minutes>();
    }

    int64_t elapsedHours() const
    {
        return elapsed<std::chrono::hours>();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_begin;
};
#endif // GRAPHICRENDERENGINE_TIMER_H
