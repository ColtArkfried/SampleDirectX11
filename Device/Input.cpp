#include "../Common.h"
#include "../Utility.h"
#include "Input.h"
using namespace Ark::Device;

/*-------------------------------------------------------------------
*スタティックメンバー
-------------------------------------------------------------------*/
std::array<bool, Input::KEY_SIZE> Input::mCurrentKeys = {false};
std::array<bool, Input::KEY_SIZE> Input::mPreviousKeys = {false};

std::array<bool, static_cast<size_t>(MouseButton::eCount)> Input::mCurrentMouseButtons = {false};
std::array<bool, static_cast<size_t>(MouseButton::eCount)> Input::mPreviousMouseButtons = {false};

DirectX::XMFLOAT2 Input::mMousePos = {0.0f, 0.0f};
DirectX::XMFLOAT2 Input::mPreviousMousePos = {0.0f, 0.0f};
DirectX::XMFLOAT2 Input::mMouseDelta = {0.0f, 0.0f};
float Input::mMouseWheelDelta = 0.0f;
float Input::mTempWheelDelta = 0.0f;

bool Input::handleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        // キーボードメッセージ
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        if(wParam < KEY_SIZE)
        {
            Input::mCurrentKeys[wParam] = true;
        }
        return true;

        case WM_KEYUP:
        case WM_SYSKEYUP:
        if(wParam < KEY_SIZE)
        {
            Input::mCurrentKeys[wParam] = false;
        }
        return true;

        // マウス移動メッセージ
        case WM_MOUSEMOVE:
        {
            // クライアント領域の左上原点の座標を取得
            float x = static_cast<float>(static_cast<short>(LOWORD(lParam)));
            float y = static_cast<float>(static_cast<short>(HIWORD(lParam)));
            Input::mMousePos = DirectX::XMFLOAT2(x, y);
            return true;
        }

        // マウスボタンメッセージ
        case WM_LBUTTONDOWN:
        SetCapture(hWnd);
        Input::mCurrentMouseButtons[static_cast<size_t>(MouseButton::eLeft)] = true;
        return true;
        case WM_LBUTTONUP:
        ReleaseCapture();
        Input::mCurrentMouseButtons[static_cast<size_t>(MouseButton::eLeft)] = false;
        return true;

        case WM_RBUTTONDOWN:
        SetCapture(hWnd);
        Input::mCurrentMouseButtons[static_cast<size_t>(MouseButton::eRight)] = true;
        return true;
        case WM_RBUTTONUP:
        ReleaseCapture();
        Input::mCurrentMouseButtons[static_cast<size_t>(MouseButton::eRight)] = false;
        return true;

        case WM_MBUTTONDOWN:
        SetCapture(hWnd);
        Input::mCurrentMouseButtons[static_cast<size_t>(MouseButton::eMiddle)] = true;
        return true;
        case WM_MBUTTONUP:
        ReleaseCapture();
        Input::mCurrentMouseButtons[static_cast<size_t>(MouseButton::eMiddle)] = false;
        return true;

        // マウスホイール
        case WM_MOUSEWHEEL:
        Input::mTempWheelDelta += static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;
        return true;

        default:
        break;
    }

    return false;
}

void Input::update(void)
{
    // 移動量の計算
    Input::mMouseDelta.x = Input::mMousePos.x - Input::mPreviousMousePos.x;
    Input::mMouseDelta.y = Input::mMousePos.y - Input::mPreviousMousePos.y;
    Input::mPreviousMousePos = Input::mMousePos;

    // ホイール回転量の適用とクリア
    Input::mMouseWheelDelta = Input::mTempWheelDelta;
    Input::mTempWheelDelta = 0.0f;

    // 次フレーム判定用に「現フレームの状態」を「前フレームの状態」にコピー
    Input::mPreviousKeys = Input::mCurrentKeys;
    Input::mPreviousMouseButtons = Input::mCurrentMouseButtons;
}

// キーボード判定
bool Input::isKeyDown(int vKey)
{
    if(vKey < 0 || vKey >= KEY_SIZE) return false;
    return Input::mCurrentKeys[vKey];
}

bool Input::isKeyTrigger(int vKey)
{
    if(vKey < 0 || vKey >= KEY_SIZE) return false;
    // 「今フレーム押されていて」かつ「前フレームは押されていなかった」
    return Input::mCurrentKeys[vKey] && !Input::mPreviousKeys[vKey];
}

bool Input::isKeyRelease(int vKey)
{
    if(vKey < 0 || vKey >= KEY_SIZE) return false;
    // 「今フレーム押されていなくて」かつ「前フレームは押されていた」
    return !Input::mCurrentKeys[vKey] && Input::mPreviousKeys[vKey];
}

// マウスボタン判定
bool Input::isMouseDown(MouseButton button)
{
    size_t idx = static_cast<size_t>(button);
    return Input::mCurrentMouseButtons[idx];
}

bool Input::isMouseTrigger(MouseButton button)
{
    size_t idx = static_cast<size_t>(button);
    return Input::mCurrentMouseButtons[idx] && !Input::mPreviousMouseButtons[idx];
}

bool Input::isMouseRelease(MouseButton button)
{
    size_t idx = static_cast<size_t>(button);
    return !Input::mCurrentMouseButtons[idx] && Input::mPreviousMouseButtons[idx];
}

// マウス座標取得
DirectX::XMFLOAT2 Input::getMousePosition(void)
{
    return Input::mMousePos;
}

DirectX::XMFLOAT2 Input::getMouseDelta(void)
{
    return Input::mMouseDelta;
}

float Input::getMouseWheelDelta(void)
{
    return Input::mMouseWheelDelta;
}