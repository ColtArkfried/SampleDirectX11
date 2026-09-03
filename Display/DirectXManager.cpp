#include "../Utility.h"
#include "../Render.h"
#include "Window.h"
#include "DirectXManager.h"

using namespace ark::render;

// ----------------------------------------------------------------------------
// ■スタティックメンバー
static DevicePtr           mpDevice = nullptr;
static DeviceContextPtr    mpContext = nullptr;
static SwapChainPtr        mpSwapChain = nullptr;
static RenderTargetViewPtr mpRenderTargetView = nullptr;
static D3D11_VIEWPORT      mViewport = {0,0,0,0,0,0};

bool DirectXManager::init(void)
{
    try
    {
        if((mpDevice != NULL) && (mD3DBase != NULL))
        {
            EXCEPTION("エラー:既に初期化されています");
            return false;
        }

        // インターフェース作成
        DirectX::createDirect3D();

        // デバイス作成
        DirectX::createDevice();

        // ハードウェア確認
        DirectX::hardwareCheck();
    }
    catch(Exception e)
    {
        close();
    }

    return true;
}

void DirectX::close(void)
{
    RELEASE(mD3DBase);
    RELEASE(mD3DDevice);
}

const LPDIRECT3DDEVICE9& DirectX::getDevice(void)
{
    return mD3DDevice;
}

const LPDIRECT3D9& DirectX::getDirect3D(void)
{
    return mD3DBase;
}

D3DCAPS9* DirectX::getCaps(void)
{
    return &mD3DCaps;
}

bool DirectX::createDirect3D(void)
{
    mD3DBase = Direct3DCreate9(D3D_SDK_VERSION);
    if(!mD3DBase)
    {
        EXCEPTION("エラー:Direct3Dインターフェイスの作成に失敗");
        return false;
    }

    return true;
}

bool DirectX::createDevice(void)
{
    HWND hwnd = Window::getWindowHandle();
    if(!hwnd)
    {
        EXCEPTION("エラー:ウィンドウハンドルが取得できません");
        return false;
    }

    if(!mD3DBase)
    {
        EXCEPTION("エラー:Direct3Dインターフェイスが作成されていません");
        return false;
    }

    //ディスプレイフォーマット取得
    D3DDISPLAYMODE displayMode;
    mD3DBase->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);

    //Direct3DDevice作成
    ZeroMemory(&mD3DParams, sizeof(D3DPRESENT_PARAMETERS));
    mD3DParams.hDeviceWindow = hwnd;
    mD3DParams.BackBufferWidth = Window::getWindowWidth();
    mD3DParams.BackBufferHeight = Window::getWindowHeight();
    mD3DParams.BackBufferCount = 1;
    mD3DParams.MultiSampleType = D3DMULTISAMPLE_NONE;
    mD3DParams.BackBufferFormat = displayMode.Format;
    mD3DParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
    mD3DParams.Windowed = Window::isWindowed();
    mD3DParams.EnableAutoDepthStencil = TRUE;
    mD3DParams.AutoDepthStencilFormat = D3DFMT_D16;
    mD3DParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;


    //Direct3DDeviceの作成
    HRESULT hr = mD3DBase->CreateDevice(
        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
        hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &mD3DParams, &mD3DDevice
        );

    if(hr != S_OK || !mD3DDevice)
    {
        EXCEPTION("エラー:Direct3Dデバイスの作成に失敗");
        return false;
    }

    return true;
}

bool DirectX::hardwareCheck(void)
{
    if(!mD3DDevice || !mD3DBase)
    {
        EXCEPTION("エラー:初期化が行われていません");
        return false;
    }

    // デバイス能力を取得
    D3DDEVICE_CREATION_PARAMETERS	deviceParams;
    mD3DDevice->GetCreationParameters(&deviceParams);
    mD3DBase->GetDeviceCaps(D3DADAPTER_DEFAULT, deviceParams.DeviceType, &mD3DCaps);

    // グラフィックボード情報取得
    D3DADAPTER_IDENTIFIER9 adapter;
    mD3DBase->GetAdapterIdentifier(0, 0, &adapter);

    // メインメモリーの情報取得
    MEMORYSTATUSEX status = {sizeof(MEMORYSTATUSEX)};
    GlobalMemoryStatusEx(&status);

    // 物理メモリの搭載容量
    uint memorySize = (uint)(status.ullTotalPhys / 1024 / 1024);
    // 物理メモリの使用容量
    uint memoryUsed = (uint)(status.ullTotalPhys - status.ullAvailPhys) / 1024 / 1024;
    // 物理メモリの空き容量
    uint memoryFree = (uint)(status.ullAvailPhys / 1024 / 1024);


#ifdef _DEBUG
    Logger::write("Memory Size:%d MB", memorySize);
    Logger::write("Memory Used:%d MB", memoryUsed);
    Logger::write("Memory Free:%d MB", memoryFree);
    Logger::write("Graphics Driver:%s", adapter.Description);
    Logger::write("Driver Version:%08X%08X", adapter.DriverVersion.HighPart, adapter.DriverVersion.LowPart);
    Logger::write("Available Video Memory:%d MB", (mD3DDevice->GetAvailableTextureMem() / (1024 * 1024)));
    Logger::write("Max Active Lights:%d", mD3DCaps.MaxActiveLights);
    Logger::write("Max Vertex Index:%d", mD3DCaps.MaxVertexIndex);
    Logger::write("Max Primitive Count:%d", mD3DCaps.MaxPrimitiveCount);
    Logger::write("Max Anisotropy:%d", mD3DCaps.MaxAnisotropy);
#endif

    if((mD3DDevice->GetAvailableTextureMem() / (1024 * 1024)) < 128)
    {
        EXCEPTION("エラー:ビデオメモリーが足りません");
        return false;
    }

    if(mD3DCaps.VertexShaderVersion < D3DVS_VERSION(2, 0) || mD3DCaps.PixelShaderVersion < D3DPS_VERSION(2, 0))
    {
        EXCEPTION("エラー:シェーダー機能がサポートされていません");
        return false;
    }

    if(mD3DCaps.MaxActiveLights < 3 || mD3DCaps.MaxVertexIndex < 0x0000FFFF)
    {
        EXCEPTION("エラー:処理能力が足りません");
        return false;
    }

    return false;
}

const HRESULT DirectX::deviceReset(void)
{
    return mD3DDevice->Reset(&mD3DParams);
}