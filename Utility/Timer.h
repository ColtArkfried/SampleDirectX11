#pragma once

#include <chrono>

/*-------------------------------------------------------------------
*namespace Ark::Utility;
-------------------------------------------------------------------*/
namespace Ark
{
    namespace Utility
    {
        /*-------------------------------------------------------------------
        // 汎用タイマー
        -------------------------------------------------------------------*/
        class Timer
        {
        public:
            Timer(void) = default;

            // 指定したミリ秒後にタイムアウトするようにタイマーを開始
            void start(double milliseconds)
            {
                mStartTime = std::chrono::high_resolution_clock::now();
                mTargetDuration = std::chrono::duration<double, std::milli>(milliseconds);
                mIsRunning = true;
            }

            // 指定した時間が経過したかを判定
            bool isTimeUp() const
            {
                if(!mIsRunning)
                {
                    return false;
                }

                auto now = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> elapsed = now - mStartTime;

                return elapsed >= mTargetDuration;
            }

            // タイマーの停止
            void stop()
            {
                mIsRunning = false;
            }

            // タイマーが動作中かどうかの取得
            bool isRunning() const
            {
                return mIsRunning;
            }

        private:
            std::chrono::high_resolution_clock::time_point mStartTime;
            std::chrono::duration<double, std::milli> mTargetDuration{0.0};
            bool mIsRunning = false;
        };
    }
}