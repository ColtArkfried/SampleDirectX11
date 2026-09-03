#pragma once

#include <chrono>
#include <thread>

/*-------------------------------------------------------------------
*namespace Ark::Display;
-------------------------------------------------------------------*/
namespace Ark
{
    namespace Display
    {
        // 定数
        constexpr float MINIMUM_FPS = 30.0f;

        /*-------------------------------------------------------------------
        // フレーム制御
        -------------------------------------------------------------------*/
        class FrameController
        {
        public:

            // フレームの開始時に呼ぶ
            static void beginFrame(float targetFps);

            // フレームの終了時に呼び出し 指定時間を超えるまで待機する
            static void endFrame(void);

            // 前回のフレームからの経過時間（秒）を取得
            static const float getDeltaTime(void) { return mDeltaTime; }

            // 現在の計測FPSを取得
            static const  float getCurrentFps(void) { return mCurrentFps; }

        private:

            // クラスをインスタンス化せず関数を呼び出す設計のため削除
            FrameController(void) = delete;
            FrameController(const FrameController&) = delete;
            ~FrameController(void) = delete;
            FrameController& operator=(const FrameController&) = delete;

            static float mTargetFps;
            static double mTargetFrameTimeMs;

            static float mDeltaTime;
            static float mCurrentFps;

            static Timer mFrameTimer;
        };
    }
}