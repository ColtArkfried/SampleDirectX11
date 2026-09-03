#include "Build.h"
#include "Common.h"
#include "Utility.h"
#include "Display.h"
#include "Device.h"
#include "Scene.h"

// ------------------------------------------------------------------
// *メイン
int main(void)
{
    DUMP("=============================== START ===============================");

    // メモリーリークの監視(NEWで確保したメモリーのみ有効)
    LEAKCHECK();

    // リソースを登録
    PathRegister::set(L"ground_tex", L"Data/ground.jpg");
    PathRegister::set(L"ground_fbx", L"Data/ground.fbx");
    PathRegister::set(L"ground_hlsl", L"Shader/ground.hlsl");

    //PathRegister::set(L"ground_fbx", L"Data/chest.fbx");
    //PathRegister::set(L"ground_hlsl", L"Shader/FBXObject.hlsl");

    // 画面の初期化とセットアップ
    //Window::setup(WindowSize(1920,1080));
    Window::setup();
    DirectXManager::setup(Window::getHandle(),Window::getWidth(),Window::getHeight());

    // カメラを生成
    Camera* camera = ObjectManager::createObject<Camera>();

    // 板を生成
    Plane* obj = ObjectManager::createObject<Plane>();
    if(obj) obj->setModelKey(L"ground");

    // 各オブジェクトのセットアップ
    ObjectManager::setupAll();

    // メインループ
    MSG msg = { 0 };
    while(WM_QUIT != msg.message)
    {
        if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            FrameController::beginFrame(60.0f);

            Input::update();

            ObjectManager::updateAll();

            DirectXManager::draw();

            FrameController::endFrame();
        }
    }

    // 終了処理
    ObjectManager::cleanupAll();
    Window::cleanup();
    DirectXManager::cleanup();

    return EXIT_SUCCESS;
}


