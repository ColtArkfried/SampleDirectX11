#pragma once

// WindowsAPIヘッダーのインクルード最適化(VC_EXTRALEAN)が有効な場合にインクルード
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#include <windows.h>

/*-------------------------------------------------------------------
*namespace Ark::Display;
-------------------------------------------------------------------*/
namespace Ark
{
    namespace Display
    {
        // テンプレートの名称化
        typedef Ark::Utility::Point2d<uint> WindowSize;
        typedef Ark::Utility::Rect<uint>    WindowRect;

        // 定数
        constexpr wchar_t WINDOW_CLASS_NAME[] = L"DirectX11";

        const WindowSize DEFAULT_WINDOW_SIZE = WindowSize(600, 400);
        const WindowSize MINIMUM_WINDOW_SIZE = WindowSize(600, 400);

        constexpr wchar_t DEFAULT_WINDOW_TITLE[] = L"DirectX11 - シェーダーによる描画";
        constexpr uint    MAXIMUM_WINDOW_TITLE_LENGTH = 64;

        /*-------------------------------------------------------------------
        // ウィンドウ制御クラス
        -------------------------------------------------------------------*/
        class Window
        {
        public:

            // ウィンドウハンドルを内部生成する
            static const HWND setup(const WindowSize& size = WindowSize(Display::DEFAULT_WINDOW_SIZE.x, Display::DEFAULT_WINDOW_SIZE.y),
                                    const WString&   title = Display::DEFAULT_WINDOW_TITLE);
            // リソースの開放と初期化
            static void  cleanup(void);

            static const HWND& getHandle(void);
            static const WString& getTitle(void);
            static const UINT     getWidth(void);
            static const UINT     getHeight(void);
            static const bool& isActive(void);

            static void updateTitle(const WString& title);
            static void updateWindowSize(const UINT& width, const UINT& height);

        private:

            // クラスをインスタンス化せず関数を呼び出す設計のため削除
            Window(void) = delete;
            Window(const Window&) = delete;
            ~Window(void) = delete;
            Window& operator=(const Window&) = delete;

            // ウィンドウハンドラ
            static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

            // 保持する変数郡
            static HWND       mHandle;
            static WNDCLASSEX mWindowClass;
            static HANDLE     mMutex;

            static WString    mWindowTitle;
            static WindowRect mDesktopRect;
            static WindowRect mWindowRect;
            static bool       mActive;
        };
    }
}