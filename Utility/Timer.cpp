#include "../Common.h"
#include "Timer.h"
using namespace std::chrono;
using namespace Ark::Utility;

Timer::Timer(void)
{
    mTime = getTimeSec();
}

Timer::~Timer(void)
{
}

inline double Timer::getTimeSec(void)
{
    return static_cast<double>(duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count()) / 1000000000;
}

const double Timer::getTime(void)
{
    mTime = getTimeSec();
    return mTime;
}

const double Timer::getTickTime(double& time)
{
    return (getTimeSec() - time);
}
