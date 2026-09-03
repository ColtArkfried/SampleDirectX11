#include "../Utility.h"
#include "../Device.h"
#include "Window.h"
using namespace Ark::Display;

/*-------------------------------------------------------------------
*スタティックメンバー
-------------------------------------------------------------------*/

// 保持する変数郡
HWND        Window::mHandle = nullptr;
HANDLE      Window::mMutex = nullptr;
WNDCLASSEX  Window::mWindowClass = {0};
bool        Window::mActive = false;

WString    Window::mWindowTitle;
WindowRect Window::mDesktopRect;
WindowRect Window::mWindowRect;


/*-------------------------------------------------------------------
*ウィンドウハンドルを内部生成する
-------------------------------------------------------------------*/
const HWND Window::setup(const WindowSize& size, const WString& title)
{
    /*-------------------------------------------------------------------
    // 多重生成を防止する処理
    -------------------------------------------------------------------*/
    try
    {
        // この関数が多重に呼び出しかを判定
        if(Window::mHandle != nullptr || Window::mMutex != nullptr || Window::mWindowClass.lpfnWndProc == Window::WndProc)
        {
            throw("エラー:既にセットアップは完了しています");
        }

        // 既にウィンドウが存在しているかを確認
        if(FindWindow(Display::WINDOW_CLASS_NAME, 0))
        {
            throw("エラー:既にウィンドウが作成されています");
        }
    }
    catch(const char* msg)
    {
        // デバッグ時にメッセージを出す
        DEBUGBREAK(msg);
        return nullptr;
    }

    /*-------------------------------------------------------------------
    // ウィンドウサイズの調整
    -------------------------------------------------------------------*/
    try
    {
        // デスクトップ解像度を取得
        RECT rct = {0};
        if(!SystemParametersInfoW(SPI_GETWORKAREA, 0, &rct, 0))
        {
            throw("エラー:デスクトップ解像度の取得に失敗しました");
        }
        Window::mDesktopRect.set(rct.left, rct.top, rct.right, rct.bottom);

        // デスクトップの解像度が最小ウィンドウサイズの定義より小さい場合をエラーとする
        if(Display::MINIMUM_WINDOW_SIZE.x > Window::mDesktopRect.getRight() ||
           Display::MINIMUM_WINDOW_SIZE.y > Window::mDesktopRect.getBottom())
        {
            throw("エラー:デスクトップ解像度が最小ウィンドウサイズより小さいため起動できません");
        }

        // クランプ処理
        uint wx = min(max(Display::MINIMUM_WINDOW_SIZE.x, size.x), Window::mDesktopRect.getRight());
        uint wy = min(max(Display::MINIMUM_WINDOW_SIZE.y, size.y), Window::mDesktopRect.getBottom());

        // ウィンドウの初期位置と範囲を算出
        uint left   = (Window::mDesktopRect.getRight() - wx) / 4;
        uint top    = (Window::mDesktopRect.getBottom() - wy) / 4;
        uint right  = left + wx;
        uint bottom = top + wy;

        Window::mWindowRect.set(left, top, right, bottom);
    }
    catch(const char* msg)
    {
        // デバッグ時にメッセージを出す
        DEBUGBREAK(msg);

        // メンバ変数を初期状態に戻す
        Window::cleanup();
        return nullptr;
    }

    /*-------------------------------------------------------------------
    // ウィンドウクラスの登録と生成
    -------------------------------------------------------------------*/

    // いずれか一つが失敗した場合にcloseが実行される
    try
    {
        // モジュールハンドルを取得する
        HINSTANCE hInstance = GetModuleHandleA(nullptr);
        if(hInstance == nullptr)
        {
            throw("エラー:モジュールハンドルの取得に失敗しました");
        }

        // 同じウィンドウクラスが登録されているかを確認
        Window::mWindowClass = {0};
        if(GetClassInfoEx(hInstance, Display::WINDOW_CLASS_NAME, &Window::mWindowClass))
        {
            throw("エラー:既にウィンドウクラスが登録されています");
        }


        // ウィンドウクラス設定と登録
        Window::mWindowClass = {0};
        Window::mWindowClass.cbSize = sizeof(WNDCLASSEXA);
        Window::mWindowClass.style = (CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_OWNDC);
        Window::mWindowClass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        Window::mWindowClass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
        Window::mWindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        Window::mWindowClass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
        Window::mWindowClass.lpszClassName = Display::WINDOW_CLASS_NAME;
        Window::mWindowClass.lpfnWndProc = Window::WndProc;
        Window::mWindowClass.hInstance = hInstance;

        // ウィンドウクラスの登録
        if(!RegisterClassEx(&Window::mWindowClass))
        {
            throw("エラー:ウィンドウクラスの登録に失敗しました");
        }

        // ミューテクスを使った多重起動防止
        // ミューテクスがすでに存在している場合をエラーとする
        Window::mMutex = CreateMutex(nullptr, TRUE, Display::WINDOW_CLASS_NAME);
        if(GetLastError() == ERROR_ALREADY_EXISTS || Window::mMutex == nullptr)
        {
            throw("エラー:既にウィンドウが作成されています");
        }


        // ウィンドウタイトルを設定
        if(!title.empty() && title.length() <= Display::MAXIMUM_WINDOW_TITLE_LENGTH)
        {
            Window::mWindowTitle = title;
        }

        // ウィンドウハンドル作成
        ulong style = (WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        Window::mHandle = CreateWindowEx(0, Window::mWindowClass.lpszClassName, Window::mWindowTitle.c_str(),
                                         style,
                                         Window::mWindowRect.getLeft(), Window::mWindowRect.getTop(),
                                         Window::mWindowRect.getRight(), Window::mWindowRect.getBottom(),
                                         nullptr, nullptr, Window::mWindowClass.hInstance, nullptr
        );
        if(Window::mHandle == nullptr)
        {
            throw("エラー:ウィンドウの作成に失敗しました");
        }
    }
    catch(const char* msg)
    {
        // デバッグ時にメッセージを出す
        DEBUGBREAK(msg);

        // まとめてリソースを開放する
        Window::cleanup();
        return nullptr;
    }


    ShowWindow(Window::mHandle, true);

    return Window::mHandle;
}

/*-------------------------------------------------------------------
*リソースの開放と初期化
-------------------------------------------------------------------*/
void Window::cleanup(void)
{
    // ミューテックスの所有権があれば破棄する
    if(Window::mMutex != nullptr)
    {
        ReleaseMutex(Window::mMutex);
        CloseHandle(Window::mMutex);

    }

    // ウィンドウインスタンスがあればウィンドウクラス登録状況を確認する
    if(Window::mWindowClass.hInstance != nullptr)
    {
        // ウィンドウクラスが登録されているなら破棄する
        if(GetClassInfoEx(Window::mWindowClass.hInstance, Display::WINDOW_CLASS_NAME, &Window::mWindowClass))
        {
            UnregisterClass(Display::WINDOW_CLASS_NAME, Window::mWindowClass.hInstance);
        }
    }

    // ウィンドウハンドルがあれば破棄する
    if(Window::mHandle != nullptr)
    {
        ShowWindow(Window::mHandle, SW_HIDE);
        ShowCursor(TRUE);
        DestroyWindow(Window::mHandle);
    }

    // メンバ変数を初期化
    Window::mMutex = nullptr;
    Window::mWindowClass = WNDCLASSEX();
    Window::mHandle = nullptr;

    Window::mWindowTitle.clear();
    Window::mDesktopRect = WindowRect(0);
    Window::mWindowRect = WindowRect(0, 0, Display::DEFAULT_WINDOW_SIZE.x, Display::DEFAULT_WINDOW_SIZE.y);

    return;
}

/*-------------------------------------------------------------------
*ウィンドウハンドルを返す
-------------------------------------------------------------------*/
const HWND& Window::getHandle()
{
    return Window::mHandle;
}

/*-------------------------------------------------------------------
*ウィンドウタイトルを返す(マルチバイト)
-------------------------------------------------------------------*/
const WString& Window::getTitle(void)
{
    return mWindowTitle;
}

/*-------------------------------------------------------------------
*ウィンドウの横幅を返す
-------------------------------------------------------------------*/
const UINT Window::getWidth(void)
{
    return mWindowRect.getWidth();
}

/*-------------------------------------------------------------------
*ウィンドウの縦幅を返す
-------------------------------------------------------------------*/
const UINT Window::getHeight(void)
{
    return mWindowRect.getHeight();
}

/*-------------------------------------------------------------------
*ウィンドウのアクティブフラグを返す
-------------------------------------------------------------------*/
const bool& Window::isActive(void)
{
    return mActive;
}


/*-------------------------------------------------------------------
*ウィンドウタイトルを変更
-------------------------------------------------------------------*/
void Window::updateTitle(const WString& title)
{

    const HWND hwnd = Window::getHandle();
    if(hwnd == nullptr) return;

    if(title.empty() || title.length() > Display::MAXIMUM_WINDOW_TITLE_LENGTH)
    {
        // デバッグ時にメッセージを出す
        DEBUGBREAK("タイトルを設定する文字列が正しくありません");
        return;
    }

    Window::mWindowTitle = title;

    SetWindowText(hwnd, mWindowTitle.c_str());
    UpdateWindow(hwnd);

    return;
}

/*-------------------------------------------------------------------
*ウィンドウサイズを変更
-------------------------------------------------------------------*/
void Window::updateWindowSize(const UINT& width, const UINT& height)
{
    const HWND hwnd = Window::getHandle();
    if(hwnd == nullptr) return;

    // 最小ウィンドウサイズ以下なら弾く
    if(width  < Display::MINIMUM_WINDOW_SIZE.x) return;
    if(height < Display::MINIMUM_WINDOW_SIZE.y) return;

    // デスクトップサイズより大きければ弾く
    if(Window::mDesktopRect.getRight()  < width ) return;
    if(Window::mDesktopRect.getBottom() < height) return;

    // 最小・最大サイズ内に収める（クランプ）
    uint wx = min(max(Display::MINIMUM_WINDOW_SIZE.x, width), Window::mDesktopRect.getRight());
    uint wy = min(max(Display::MINIMUM_WINDOW_SIZE.y, height), Window::mDesktopRect.getBottom());

    // 現在のウィンドウの左上座標（left, top）を取得して、新しい right, bottom を求める
    RECT rect;
    GetWindowRect(hwnd, &rect);

    uint left = rect.left;
    uint top = rect.top;
    uint right = left + wx;
    uint bottom = top + wy;

    // 内部の矩形情報を更新！
    Window::mWindowRect.set(left, top, right, bottom);
}

/*-------------------------------------------------------------------
*ウィンドウプロシジャ
-------------------------------------------------------------------*/
LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        // ウィンドウのメッセージ
        case WM_ACTIVATE:
        if(wParam == WA_INACTIVE) Window::mActive = false;
        else                      Window::mActive = true;
        break;

        case WM_SIZE:
        switch(wParam)
        {
            case SIZE_MAXHIDE:
            case SIZE_MINIMIZED:
            Window::mActive = false;
            break;

            case SIZE_MAXIMIZED:
            Window::mActive = true;
            break;

            case SIZE_RESTORED:
            Window::mActive = true;
            Window::updateWindowSize(LOWORD(lParam), HIWORD(lParam));
            break;
        }
        break;

        case WM_SYSCOMMAND:
        if(wParam == SC_KEYMENU) return 0;
        break;

        case WM_DESTROY:
        PostQuitMessage(0);
        break;


        // キーボードとマウスのメッセージ
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MOUSEWHEEL:
        if(Input::handleMessage(hWnd, uMsg, wParam, lParam))
        {
            return 0;
        }
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}