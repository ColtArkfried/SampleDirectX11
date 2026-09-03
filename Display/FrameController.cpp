#include "../Common.h"
#include "../Utility.h"
#include "FrameController.h"
using namespace Ark::Display;

/*-------------------------------------------------------------------
*スタティックメンバー
-------------------------------------------------------------------*/
float  FrameController::mTargetFps;
double FrameController::mTargetFrameTimeMs;
float FrameController::mDeltaTime;
float FrameController::mCurrentFps;
Timer FrameController::mFrameTimer;

void FrameController::beginFrame(float targetFps)
{
    if(targetFps <= 0.0f)
    {
        targetFps = MINIMUM_FPS;
    }
    mTargetFps = targetFps;
    mTargetFrameTimeMs = 1000.0 / static_cast<double>(mTargetFps);
    mFrameTimer.start(mTargetFrameTimeMs);
}

void FrameController::endFrame()
{
    // Timerで指定時間（16.66msなど）が経過するまで待機
    while(!mFrameTimer.isTimeUp())
    {
        // CPUを占有しないようにスリープ
        std::this_thread::yield();
    }

    // デルタタイム（秒）の計算（目標時間を秒に換算）
    mDeltaTime = static_cast<float>(mTargetFrameTimeMs / 1000.0);

    // 実際のFPSの計算
    if(mDeltaTime > 0.0f)
    {
        mCurrentFps = 1.0f / mDeltaTime;
    }
}