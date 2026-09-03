#pragma once

// DirectXのコンテナ用
#include <DirectXMath.h>

// WindowsAPIヘッダーのインクルード最適化(VC_EXTRALEAN)が有効な場合にインクルード
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#include <windows.h>

/*-------------------------------------------------------------------
*namespace Ark::Device;
-------------------------------------------------------------------*/
namespace Ark
{
    namespace Device
    {

        // マウスボタン識別用
        enum class MouseButton
        {
            eLeft = 0,
            eRight,
            eMiddle,
            eCount
        };

        /*-------------------------------------------------------------------
        // キーボード・マウス入力クラス
        -------------------------------------------------------------------*/
        class Input
        {
        public:
            // インスタンス化を禁止
            Input() = delete;

            // --- 初期化・更新系 ---
            // ウィンドウプロシージャ(WndProc)からのメッセージを処理する
            static bool handleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

            // 毎フレームの最初に呼ぶ（前フレーム状態の更新と移動量の計算）
            static void update();

            // --- キーボード入力 ---
            // 押し続けられているか (VK_SPACE や 'W' など Windowsの仮想キーコードを指定)
            static bool isKeyDown(int vKey);

            // 押された瞬間か
            static bool isKeyTrigger(int vKey);

            // 離された瞬間か
            static bool isKeyRelease(int vKey);

            // --- マウスボタン入力 ---
            // 押し続けられているか
            static bool isMouseDown(MouseButton button);

            // 押された瞬間か
            static bool isMouseTrigger(MouseButton button);

            // 離された瞬間か
            static bool isMouseRelease(MouseButton button);

            // --- マウス座標 ---
            // 現在のマウス座標（スクリーン座標／クライアント座標）
            static DirectX::XMFLOAT2 getMousePosition(void);

            // 前フレームからのマウス移動量（FPSカメラなどの視点移動用）
            static DirectX::XMFLOAT2 getMouseDelta(void);

            // マウスホイールの回転量（今フレームでの変化量）
            static float getMouseWheelDelta(void);

        private:
            static constexpr int KEY_SIZE = 256;

            // キーボードの状態（現フレーム / 前フレーム）
            static std::array<bool, KEY_SIZE> mCurrentKeys;
            static std::array<bool, KEY_SIZE> mPreviousKeys;

            // マウスボタンの状態（現フレーム / 前フレーム）
            static std::array<bool, static_cast<size_t>(MouseButton::eCount)> mCurrentMouseButtons;
            static std::array<bool, static_cast<size_t>(MouseButton::eCount)> mPreviousMouseButtons;

            // マウス位置データ
            static DirectX::XMFLOAT2 mMousePos;
            static DirectX::XMFLOAT2 mPreviousMousePos;
            static DirectX::XMFLOAT2 mMouseDelta;
            static float mMouseWheelDelta;
            static float mTempWheelDelta; // メッセージハンドラからの蓄積用
        };
    }
}