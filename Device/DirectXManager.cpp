#include "../Common.h"
#include "../Utility.h"
#include "../Scene.h"
#include "DirectXManager.h"
#include <WICTextureLoader.h>
using namespace Microsoft::WRL;
using namespace DirectX;
using namespace Ark::Device;

/*-------------------------------------------------------------------
// スタティックメンバー
-------------------------------------------------------------------*/
ComPtr<ID3D11Device>           DirectXManager::mDevice = nullptr;
ComPtr<ID3D11DeviceContext>    DirectXManager::mContext = nullptr;
ComPtr<IDXGISwapChain>         DirectXManager::mSwapChain = nullptr;
ComPtr<ID3D11RenderTargetView> DirectXManager::mRenderTargetView = nullptr;
ComPtr<ID3D11DepthStencilView> DirectXManager::mDepthStencilView = nullptr;
ComPtr<ID3D11Buffer>           DirectXManager::mConstantBuffer = nullptr;
ComPtr<ID3D11Texture2D>        DirectXManager::mDepthStencilTexture = nullptr;
ComPtr<ID3D11SamplerState>     DirectXManager::mSamplerState = nullptr;

D3D_DRIVER_TYPE   DirectXManager::mDriverType   = D3D_DRIVER_TYPE_HARDWARE;
D3D_FEATURE_LEVEL DirectXManager::mFeatureLevel = D3D_FEATURE_LEVEL_11_0;
DXGI_FORMAT       DirectXManager::mBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

D3D11_VIEWPORT DirectXManager::mViewport = {0};

std::unordered_map<WString, SHADER_RESOURCE>            DirectXManager::mShaderCache;
std::unordered_map<WString, std::vector<MESH_RESOURCE>> DirectXManager::mMeshCache;
std::unordered_map<WString, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> > DirectXManager::mTextureCache;

/*-------------------------------------------------------------------
// DirectX11の初期化
-------------------------------------------------------------------*/
void DirectXManager::setup(const HWND& hwnd, const UINT& width, const UINT& height)
{
    try
    {
        // デバイスとデバイスコンテキストの作成
        DirectXManager::createDevice();

        // スワップチェインの作成
        DirectXManager::createSwapChain(hwnd, width, height);

        // レンダーターゲットの作成
        DirectXManager::createRenderTarget();

        // ディプス・ステンシルバッファの作成
        DirectXManager::createDepthStencilBuffer(width, height);

        // サンプラーステートの作成
        DirectXManager::createSamplerState();

        // ビューポートを作成
        const float fWidth = static_cast<float>(width);
        const float fHeight = static_cast<float>(height);
        DirectXManager::setViewPort(fWidth, fHeight);
    }
    catch(const char* msg)
    {
        // 失敗時にリソース開放
        DirectXManager::cleanup();

        // デバッグ時にメッセージを出す
        DEBUGBREAK(msg);
    }
}

/*-------------------------------------------------------------------
// リソースの破棄
-------------------------------------------------------------------*/
void DirectXManager::cleanup(void)
{
    if(DirectXManager::mContext) {
        DirectXManager::mContext->ClearState();
    }

    DirectXManager::mConstantBuffer.Reset();
    DirectXManager::mRenderTargetView.Reset();
    DirectXManager::mContext.Reset();
    DirectXManager::mSwapChain.Reset();
    DirectXManager::mDevice.Reset();
}

/*-------------------------------------------------------------------
// ビューポートの設定
-------------------------------------------------------------------*/
bool DirectXManager::setViewPort(const float& width, const float& height)
{
    try
    {
        if(DirectXManager::mDevice == nullptr || DirectXManager::mContext == nullptr)
        {
            throw("エラー:デバイスの取得に失敗しました");
        }

        ZeroMemory(&DirectXManager::mViewport, sizeof(D3D11_VIEWPORT));
        DirectXManager::mViewport.Width    = width;
        DirectXManager::mViewport.Height   = height;
        DirectXManager::mViewport.MinDepth = 0.0f;
        DirectXManager::mViewport.MaxDepth = 1.0f;
        DirectXManager::mViewport.TopLeftX = 0.0f;
        DirectXManager::mViewport.TopLeftY = 0.0f;

        DirectXManager::mContext->RSSetViewports(1, &DirectXManager::mViewport);
    }
    catch(const char* msg)
    {
        // 失敗時にリソース開放
        DirectXManager::cleanup();

        // デバッグ時にメッセージを出す
        DEBUGBREAK(msg);

        return false;
    }

    return true;
}

/*-------------------------------------------------------------------
// デバイスとデバイスコンテキストを作成
-------------------------------------------------------------------*/
bool DirectXManager::createDevice(void)
{
    try
    {
        if((DirectXManager::mDevice != nullptr) || (DirectXManager::mContext != nullptr))
        {
            throw("エラー:既に初期化されています");
        }

        // デバッグ時にエラー詳細を有効化する
        UINT creationFlags = 0;
    #ifdef _DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,	// Direct3D 11.1  ShaderModel 5
            D3D_FEATURE_LEVEL_11_0,	// Direct3D 11    ShaderModel 5
            D3D_FEATURE_LEVEL_10_1,	// Direct3D 10.1  ShaderModel 4
            D3D_FEATURE_LEVEL_10_0,	// Direct3D 10.0  ShaderModel 4
            D3D_FEATURE_LEVEL_9_3,	// Direct3D 9.3   ShaderModel 3
            D3D_FEATURE_LEVEL_9_2,	// Direct3D 9.2   ShaderModel 3
            D3D_FEATURE_LEVEL_9_1,	// Direct3D 9.1   ShaderModel 3
        };

        // デバイスとデバイスコンテキストを作成
        if(FAILED(D3D11CreateDevice(
            nullptr,
            DirectXManager::mDriverType,
            nullptr,
            creationFlags,
            featureLevels,
            _countof(featureLevels),
            D3D11_SDK_VERSION,
            &DirectXManager::mDevice,
            &DirectXManager::mFeatureLevel,
            &DirectXManager::mContext)))
        {
            throw("エラー:DirectX11デバイス生成に失敗しました");
        }
    }
    catch(const char* msg)
    {
        // 失敗時にリソース開放
        DirectXManager::cleanup();

        // デバッグ時にメッセージを出す
        DEBUGBREAK(msg);

        return false;
    }

    return true;
}

/*-------------------------------------------------------------------
// スワップチェインの作成
-------------------------------------------------------------------*/
bool DirectXManager::createSwapChain(const HWND& hwnd, const UINT& width, const UINT& height)
{
    try
    {
        if(DirectXManager::mSwapChain != nullptr)
        {
            throw("エラー:既に初期化されています");
        }

        if(DirectXManager::mDevice == nullptr || DirectXManager::mContext == nullptr)
        {
            throw("エラー:デバイスの取得に失敗しました");
        }

        // ウィンドウハンドルを取得
        if(hwnd == nullptr)
        {
            throw("エラー:ウィンドウハンドルの取得に失敗しました");
        }

        // スワップチェインの設定
        DXGI_SWAP_CHAIN_DESC scDesc = {};

        // ウィンドウ関連
        scDesc.BufferDesc.Width = width;   // 画面の幅
        scDesc.BufferDesc.Height = height;  // 画面の高さ
        scDesc.OutputWindow = hwnd;    // ウィンドウハンドル
        scDesc.Windowed = TRUE;    // ウィンドウモード

        // バッファ関連
        scDesc.BufferDesc.Format = DirectXManager::mBufferFormat;             // フォーマット
        scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // バッファの使用方法
        scDesc.BufferCount = 2;                                    // バッファ数

        // 固有機能関連
        scDesc.BufferDesc.RefreshRate.Numerator = 0; // フレームを制御しない
        scDesc.BufferDesc.RefreshRate.Denominator = 1;
        scDesc.SampleDesc.Count = 1; // MSAAを使用しない
        scDesc.SampleDesc.Quality = 0;
        scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


        ComPtr<IDXGIFactory> factory;
        if(FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
        {
            throw("エラー:ファクトリクラスの作成に失敗しました");
        }

        // スワップチェインの作成
        if(FAILED(factory->CreateSwapChain(DirectXManager::mDevice.Get(), &scDesc, &DirectXManager::mSwapChain)))
        {
            throw("エラー:スワップチェインの作成に失敗しました");
        }
    }
    catch(const char* msg)
    {
        // 失敗時にリソース開放
        DirectXManager::cleanup();

        // デバッグ時にメッセージを出す
        DEBUGBREAK(msg);

        return false;
    }

    return true;
}

/*-------------------------------------------------------------------
// レンダーターゲットの作成
-------------------------------------------------------------------*/
bool DirectXManager::createRenderTarget(void)
{
    try
    {
        if(DirectXManager::mRenderTargetView != nullptr)
        {
            throw("エラー:既に初期化されています");
        }

        if(DirectXManager::mDevice == nullptr || DirectXManager::mContext == nullptr)
        {
            throw("エラー:デバイスの取得に失敗しました");
        }

        if(DirectXManager::mSwapChain == nullptr)
        {
            throw("エラー:スワップチェインの取得に失敗しました");
        }

        // バックバッファを取得
        ComPtr<ID3D11Texture2D> pBackBuffer;
        if(FAILED(DirectXManager::mSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
        {
            throw("エラー:バックバッファの取得に失敗しました");
        }

        // バックバッファに対応するレンダーターゲットを作成
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.Format = DirectXManager::mBufferFormat;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        if(FAILED(DirectXManager::mDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, &DirectXManager::mRenderTargetView)))
        {
            throw("エラー:レンダーターゲットの作成に失敗しました");
        }

        // バックバッファにレンダーターゲットを設定
        DirectXManager::mContext->OMSetRenderTargets(1, DirectXManager::mRenderTargetView.GetAddressOf(), nullptr);

    }
    catch(const char* msg)
    {
        // 失敗時にリソース開放
        DirectXManager::cleanup();

        // デバッグ時にメッセージを出す
        DEBUGBREAK(msg);

        return false;
    }

    return true;
}

/*-------------------------------------------------------------------
// ディプス・ステンシルバッファの作成
-------------------------------------------------------------------*/
bool DirectXManager::createDepthStencilBuffer(const UINT& width, const UINT& height)
{
    try
    {
        D3D11_TEXTURE2D_DESC txDesc;
        ZeroMemory(&txDesc, sizeof(txDesc));
        txDesc.Width  = width;
        txDesc.Height = height;
        txDesc.MipLevels = 1;
        txDesc.ArraySize = 1;
        txDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        txDesc.SampleDesc.Count = 1;
        txDesc.SampleDesc.Quality = 0;
        txDesc.Usage = D3D11_USAGE_DEFAULT;
        txDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        txDesc.CPUAccessFlags = 0;
        txDesc.MiscFlags = 0;

        if(FAILED(DirectXManager::mDevice->CreateTexture2D(&txDesc, NULL, &DirectXManager::mDepthStencilTexture)))
        {
            throw("エラー:ディブス・ステンシルバッファの作成に失敗しました");
        }


        D3D11_DEPTH_STENCIL_VIEW_DESC dsDesc;
        ZeroMemory(&dsDesc, sizeof(dsDesc));
        dsDesc.Format = txDesc.Format;
        dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        dsDesc.Texture2D.MipSlice = 0;
        if(FAILED(DirectXManager::mDevice->CreateDepthStencilView(
            DirectXManager::mDepthStencilTexture.Get(),
            &dsDesc,
            &DirectXManager::mDepthStencilView)))
        {
            throw("エラー:ディブス・ステンシルビューの作成に失敗しました");
        }


    }
    catch(const char* msg)
    {
        // 失敗時にリソース開放
        DirectXManager::cleanup();

        // デバッグ時にメッセージを出す
        DEBUGBREAK(msg);

        return false;
    }

    return true;
}

/*-------------------------------------------------------------------
// コンスタントバッファの作成
-------------------------------------------------------------------*/
bool DirectXManager::createConstantBuffer(void)
{
    try
    {
        if(mConstantBuffer == nullptr)
        {
            D3D11_BUFFER_DESC cbDesc{};
            cbDesc.ByteWidth = sizeof(CONSTANT_BUFFER);
            cbDesc.Usage = D3D11_USAGE_DEFAULT;
            cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbDesc.CPUAccessFlags = 0;
            cbDesc.MiscFlags = 0;
            cbDesc.StructureByteStride = 0;

            if(FAILED(DirectXManager::mDevice->CreateBuffer(&cbDesc, nullptr, &DirectXManager::mConstantBuffer)))
            {
                throw("エラー:コンスタントバッファの作成に失敗しました");
            }
        }
    }
    catch(const char* msg)
    {
        // 失敗時にリソース開放
        DirectXManager::cleanup();

        // デバッグ時にメッセージを出す
        DEBUGBREAK(msg);

        return false;
    }

    return true;
}
/*-------------------------------------------------------------------
// サンプラーステートの作成
-------------------------------------------------------------------*/
bool DirectXManager::createSamplerState(void)
{
    try
    {
        D3D11_SAMPLER_DESC sampDesc = {};
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // 滑らかに補間
        sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;    // 横方向リピート
        sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;    // 縦方向リピート
        sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampDesc.MinLOD = 0;
        sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

        if(FAILED(mDevice->CreateSamplerState(&sampDesc, &mSamplerState)))
        {
            throw("エラー:サンプラーステートの作成に失敗しました");
        }

    }
    catch(const char* msg)
    {
        // 失敗時にリソース開放
        DirectXManager::cleanup();

        // デバッグ時にメッセージを出す
        DEBUGBREAK(msg);

        return false;
    }

    return true;
}

/*-------------------------------------------------------------------
// シェーダーのインスタンス化と登録
-------------------------------------------------------------------*/
const SHADER_RESOURCE* DirectXManager::getShader(const WString& key, const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements)
{
    // キャッシュに存在すればそのポインタを返す
    auto it = mShaderCache.find(key);
    if(it != mShaderCache.end())
    {
        return &(it->second);
    }

    // キャッシュになければ新規コンパイル＆作成
    SHADER_RESOURCE newShader;

    WString hlsl_key = key + L"_hlsl";
    WString path = PathRegister::get(hlsl_key.c_str());
    if(path.empty())
    {
        DEBUGBREAK("エラー: シェーダーのファイルパスが未登録です");
        return nullptr;
    }

    // 頂点シェーダーのコンパイル＆作成
    Microsoft::WRL::ComPtr<ID3D10Blob> vsBlob;
    if(FAILED(compileShader(path.c_str(), "VS_Main", "vs_5_0", vsBlob.GetAddressOf())))
    {
        DEBUGBREAK("エラー: 頂点シェーダーのコンパイルに失敗しました");
        return nullptr;
    }
    mDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, newShader.pVS.GetAddressOf());

    // ピクセルシェーダーのコンパイル＆作成
    Microsoft::WRL::ComPtr<ID3D10Blob> psBlob;
    if(FAILED(compileShader(path.c_str(), "PS_Main", "ps_5_0", psBlob.GetAddressOf())))
    {
        DEBUGBREAK("エラー: ピクセルシェーダーのコンパイルに失敗しました");
        return nullptr;
    }
    mDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, newShader.pPS.GetAddressOf());

    // インプットレイアウトの作成
    if(layoutDesc == nullptr || numElements == 0)
    {
        DEBUGBREAK("エラー: インプットレイアウトの指定が不十分です");
        return nullptr;
    }

    if( FAILED(mDevice->CreateInputLayout(layoutDesc, numElements, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), newShader.pLayout.GetAddressOf())) )
    {
        DEBUGBREAK("エラー: インプットレイアウトの作成に失敗しました");
        return nullptr;
    }
    // キャッシュに登録して参照を返す
    mShaderCache[key] = std::move(newShader);
    return &mShaderCache[key];
}

/*-------------------------------------------------------------------
// 汎用シェーダーコンパイル
-------------------------------------------------------------------*/
HRESULT DirectXManager::compileShader(const WString& path, const String& entryPoint, const String& target, ID3D10Blob** blobOut)
{
    DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    if(path.empty())
    {
        throw("エラー:シェーダーのファイルパスが正しくありません");
    }

    Microsoft::WRL::ComPtr<ID3D10Blob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        path.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.c_str(),
        target.c_str(),
        shaderFlags,
        0,
        blobOut,
        errorBlob.GetAddressOf()
    );

    // コンパイルエラーの原因をログに残す
    if(FAILED(hr) && errorBlob)
    {
        OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
    }

    return hr;
}

/*-------------------------------------------------------------------
// モデルのインスタンス化と登録
-------------------------------------------------------------------*/
template <typename T> std::vector<MESH_RESOURCE>* DirectXManager::getModel(const WString& key)
{

    // キャッシュに存在すればそのポインタを返す
    auto it = mMeshCache.find(key);
    if(it != mMeshCache.end())
    {
        return &(it->second);
    }

    WString fbx_key = key + L"_fbx";
    WString path = PathRegister::get(fbx_key.c_str());
    if(path.empty())
    {
        DEBUGBREAK("エラー:3Dモデルのファイルパスが未登録です");
        return nullptr;
    }

    char tempPath[256] = {0};
    if(path.size() > sizeof(tempPath))
    {
        DEBUGBREAK("エラー:ファイルパスが長すぎます");
        return nullptr;
    }
    // 文字列の変換
    WideCharToMultiByte(CP_ACP, 0, path.c_str(), -1, tempPath, (int)path.size(), NULL, NULL);

    Assimp::Importer importer;

    // 1. Assimpでファイルを読み込む
    const aiScene* scene = importer.ReadFile(
        tempPath,
        aiProcess_Triangulate |           // ポリゴンをすべて三角形に分割
        aiProcess_ConvertToLeftHanded |   // DirectX用の左手系座標に変換
        aiProcess_GenNormals |            // 法線がなければ自動生成
        aiProcess_CalcTangentSpace        // 接線計算
    );

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        DEBUGBREAK("エラー:3Dモデルの読み込みに失敗しました");
        return nullptr;
    }

    std::vector<MESH_RESOURCE> newMeshs;

    // 2. シーンに含まれるすべてのメッシュ（パーツ）を処理
    for(unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        aiMesh* aiMesh = scene->mMeshes[i];

        std::vector<T> vertices;
        std::vector<uint32_t> indices;

        // --- 頂点データの抽出 ---
        for(unsigned int v = 0; v < aiMesh->mNumVertices; ++v)
        {
            T vertex;

            // 位置
            vertex.position = {aiMesh->mVertices[v].x, aiMesh->mVertices[v].y, aiMesh->mVertices[v].z};

            // 法線
            if(aiMesh->HasNormals()) {
                vertex.normal = {aiMesh->mNormals[v].x, aiMesh->mNormals[v].y, aiMesh->mNormals[v].z};
            }
            else {
                vertex.normal = {0.0f, 1.0f, 0.0f};
            }

            // UV座標 (0番目)
            if(aiMesh->mTextureCoords[0]) {
                vertex.uv = {aiMesh->mTextureCoords[0][v].x, aiMesh->mTextureCoords[0][v].y};
            }
            else {
                vertex.uv = {0.0f, 0.0f};
            }

            // タンジェント
            if constexpr(requires(T t) { t.tangent; })
            {
                if(aiMesh->HasTangentsAndBitangents()) {
                    vertex.tangent = {aiMesh->mTangents[v].x, aiMesh->mTangents[v].y, aiMesh->mTangents[v].z};
                }
                else {
                    vertex.tangent = {1.0f, 0.0f, 0.0f};
                }
            }

            vertices.push_back(vertex);
        }


        // --- インデックスデータの抽出 ---
        for(unsigned int f = 0; f < aiMesh->mNumFaces; ++f)
        {
            aiFace face = aiMesh->mFaces[f];
            for(unsigned int j = 0; j < face.mNumIndices; ++j)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        MESH_RESOURCE meshRes = {};
        meshRes.indexCount = static_cast<UINT>(indices.size());

        // 頂点バッファ作成
        D3D11_BUFFER_DESC vbd = {};
        vbd.Usage = D3D11_USAGE_DEFAULT;
        vbd.ByteWidth = static_cast<UINT>(sizeof(T) * vertices.size());
        vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vInitData = {};
        vInitData.pSysMem = vertices.data();

        HRESULT hr = mDevice->CreateBuffer(&vbd, &vInitData, &meshRes.pVertex);
        if(FAILED(hr))
        {
            DEBUGBREAK("エラー:バーテックスバッファの作成に失敗しました");
            return nullptr;
        }

        // インデックスバッファ作成
        D3D11_BUFFER_DESC ibd = {};
        ibd.Usage = D3D11_USAGE_DEFAULT;
        ibd.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indices.size());
        ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA iInitData = {};
        iInitData.pSysMem = indices.data();

        hr = mDevice->CreateBuffer(&ibd, &iInitData, &meshRes.pIndex);
        if(FAILED(hr))
        {
            DEBUGBREAK("エラー:インデックスバッファの作成に失敗しました");
            return nullptr;
        }

        // 読み込み用ヘルパー
        auto loadTex = [&](aiMaterial* mat, aiTextureType type, UINT index, TEXTURE_TYPE slot)
        {
            aiString path;

            // 引数の type と index を使ってテクスチャを取得
            if(mat->GetTexture(type, index, &path) == AI_SUCCESS)
            {
                WString wTexPath = WString(path.C_Str(), path.C_Str() + path.length);
                size_t pos = wTexPath.find_last_of(L"\\/");
                if(pos != WString::npos) {
                    wTexPath = L"Data/" + wTexPath.substr(pos + 1);
                }
                else {
                    wTexPath = L"Data/" + wTexPath;
                }

                // 引数で指定されたスロットへ渡す
                meshRes.pTextures[slot] = getTexture(wTexPath);

                char buf[256];
                sprintf_s(buf, "テクスチャタイプ[%d]スロット[%d] にロード: %s", type, slot, path.C_Str());
                DUMP(buf);
            }
            else {
                // テクスチャパスが一切入っていない場合
                DUMP("このメッシュにはテクスチャが設定されていません");
            }
        };

        // テクスチャをファイルから読み込み
        if(aiMesh->mMaterialIndex < scene->mNumMaterials)
        {
            aiMaterial* material = scene->mMaterials[aiMesh->mMaterialIndex];

            DUMP("\n\n=== MESH MATERIAL DEBUG START ===");
            for(unsigned int p = 0; p < material->mNumProperties; ++p)
            {
                const aiMaterialProperty* prop = material->mProperties[p];

                // プロパティキー名（$?mat.name, $tex.file など）
                std::string keyName = prop->mKey.C_Str();

                // テクスチャファイルパスなどの文字列データの場合
                if(prop->mType == aiPTI_String)
                {
                    aiString str;
                    material->Get(keyName.c_str(), prop->mSemantic, prop->mIndex, str);

                    // 出力ウィンドウで確認できるように出力
                    char debugBuf[512];
                    sprintf_s(debugBuf, "[MatProp] Key: %s | Semantic(TextureType): %d | Path: %s\n",
                              keyName.c_str(), prop->mSemantic, str.C_Str());
                    OutputDebugStringA(debugBuf);
                }
            }
            DUMP("=== MESH MATERIAL DEBUG END ===\n\n");

            // 1. メインカラー (Semantic: 1 -> aiTextureType_DIFFUSE)
            loadTex(material, aiTextureType_DIFFUSE, 0, TEX_DIFFUSE);

            // 2. スペキュラ (Semantic: 2 -> aiTextureType_SPECULAR)
            loadTex(material, aiTextureType_SPECULAR, 0, TEX_SPECULAR);

            // 3. 法線/バンプ (Semantic: 6 -> aiTextureType_NORMALS)
            loadTex(material, aiTextureType_NORMALS, 0, TEX_NORMAL);
        }

        if(FAILED(hr))
        {
            // 画像読み込み失敗時のデバッグ出力
            DEBUGBREAK("テクスチャの読み込みに失敗しました");
            return nullptr;
        }

        // 完成したメッシュリソースをリストに追加
        newMeshs.push_back(meshRes);
    }

    mMeshCache[key] = std::move(newMeshs);

    return &mMeshCache[key];
}

// スタティック関数のため宣言を予めセットする。
template std::vector<MESH_RESOURCE>* DirectXManager::getModel<FBXObject::VERTEX>(const WString& key);
template std::vector<MESH_RESOURCE>* DirectXManager::getModel<Plane::VERTEX>(const WString& key);


// テクスチャをキャッシュ経由で取得/ロードする関数
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DirectXManager::getTexture(const WString& filePath)
{
    // 1. キャッシュにあればそれを返す
    auto it = mTextureCache.find(filePath);
    if(it != mTextureCache.end())
    {
        return it->second;
    }

    // 2. キャッシュになければファイルから作成
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTexture;
    HRESULT hr = DirectX::CreateWICTextureFromFile(
        mDevice.Get(),
        filePath.c_str(),
        nullptr,
        pTexture.GetAddressOf()
    );

    if(FAILED(hr))
    {
        DEBUGBREAK("テクスチャのロードに失敗しました");
        return nullptr;
    }

    // 3. キャッシュに登録して返す
    mTextureCache[filePath] = pTexture;
    return pTexture;
}

/*-------------------------------------------------------------------
// 描画
-------------------------------------------------------------------*/
void DirectXManager::draw(void)
{
    // RenderTargetView と DepthStencilView をバインド
    ID3D11RenderTargetView* rtv = DirectXManager::mRenderTargetView.Get();
    ID3D11DepthStencilView* dsv = DirectXManager::mDepthStencilView.Get();
    DirectXManager::mContext->OMSetRenderTargets(1, &rtv, dsv);

    // 指定の色で塗りつぶす
    static const float tmpColor[4] = {0.2f, 0.2f, 0.2f, 1.0f};
    DirectXManager::mContext->ClearRenderTargetView(rtv, tmpColor);

    if(dsv)
    {
        DirectXManager::mContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }

    // 各オブジェクトの描画
    ObjectManager::drawAll();

    // バックバッファの内容を画面に表示
    DirectXManager::mSwapChain->Present(1, 0);
}

/*-------------------------------------------------------------------
// デバイスポインタを返す
-------------------------------------------------------------------*/
ComPtr<ID3D11Device> DirectXManager::getDevice(void)
{
    return DirectXManager::mDevice;
}

/*-------------------------------------------------------------------
// デバイスコンテキストポインタを返す
-------------------------------------------------------------------*/
ComPtr<ID3D11DeviceContext> DirectXManager::getContext(void)
{
    return DirectXManager::mContext;
}

/*-------------------------------------------------------------------
// スワップチェインポインタを返す
-------------------------------------------------------------------*/
ComPtr<IDXGISwapChain> DirectXManager::getSwapChain(void)
{
    return DirectXManager::mSwapChain;
}

/*-------------------------------------------------------------------
// レンダーターゲットヴューポインタを返す
-------------------------------------------------------------------*/
ComPtr<ID3D11RenderTargetView> DirectXManager::getRenderTargetView(void)
{
    return DirectXManager::mRenderTargetView;
}

/*-------------------------------------------------------------------
// ディプス・ステンシルビューポインタを返す
-------------------------------------------------------------------*/
ComPtr<ID3D11DepthStencilView> DirectXManager::getDepthStencilView(void)
{
    return DirectXManager::mDepthStencilView;
}

/*-------------------------------------------------------------------
// コンスタントバッファポインタを返す
-------------------------------------------------------------------*/
ComPtr<ID3D11Buffer> DirectXManager::getConstantBuffer(void)
{
    return DirectXManager::mConstantBuffer;
}
/*-------------------------------------------------------------------
// ヴューポートの参照を返す
-------------------------------------------------------------------*/
const D3D11_VIEWPORT& DirectXManager::getViewPort(void)
{
    return DirectXManager::mViewport;
}
/*-------------------------------------------------------------------
// サンプラーステートのポインタを返す
-------------------------------------------------------------------*/
ComPtr<ID3D11SamplerState> DirectXManager::getSamplerState(void)
{
    return DirectXManager::mSamplerState;
}
