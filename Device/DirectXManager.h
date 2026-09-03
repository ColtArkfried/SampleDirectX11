#pragma once

// COMポインタとDirectX関連のインクルード
#include <wrl.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <unordered_map>

#include <assimp/Importer.hpp>   // メインの読み込みクラス
#include <assimp/scene.h>        // 読み込んだデータ構造 (aiScene, aiMesh など)
#include <assimp/postprocess.h>  // 読み込み時のフラグ (aiProcess_...)

/*-------------------------------------------------------------------
// namespace Ark::Device;
-------------------------------------------------------------------*/
namespace Ark
{
    namespace Device
    {
        // シェーダーセット（VS + PS + InputLayout）をまとめる構造体
        struct SHADER_RESOURCE
        {
            Microsoft::WRL::ComPtr<ID3D11VertexShader> pVS;
            Microsoft::WRL::ComPtr<ID3D11PixelShader>  pPS;
            Microsoft::WRL::ComPtr<ID3D11InputLayout>  pLayout;
        };

        // テクスチャのスロット定義
        enum TEXTURE_TYPE
        {
            TEX_DIFFUSE  = 0, // Slot 0: メインカラー
            TEX_DIFFUSE2 = 1, // Slot 1: サブカラー/バンプ用カラー
            TEX_NORMAL   = 2, // Slot 2: 法線マップ
            TEX_SPECULAR = 3, // Slot 3: スペキュラマップ
            TEX_MAX
        };

        // メッシュ
        struct MESH_RESOURCE
        {
            Microsoft::WRL::ComPtr<ID3D11Buffer> pVertex;
            Microsoft::WRL::ComPtr<ID3D11Buffer> pIndex;
            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextures[TEX_MAX];
            UINT indexCount = 0;
        };

        // 頂点のデータ構造
        struct VERTEX
        {
            DirectX::XMFLOAT3 pos;      // 座標
            DirectX::XMFLOAT3 normal;   // 法線
            DirectX::XMFLOAT2 texCoord; // UV座標
        };



        /*-------------------------------------------------------------------
        // DirectX制御クラス
        -------------------------------------------------------------------*/
        class DirectXManager
        {
        public:

            struct CONSTANT_BUFFER
            {
                DirectX::XMFLOAT4X4 world;
                DirectX::XMFLOAT4X4 view;
                DirectX::XMFLOAT4X4 projection;
                DirectX::XMFLOAT4   light;
            };

            // DirectXデバイスを生成する
            static void setup(const HWND& hwnd, const UINT& width, const UINT& height);

            // リソースの開放
            static void  cleanup(void);

            static void draw(void);

            static Microsoft::WRL::ComPtr<ID3D11Device>           getDevice(void);
            static Microsoft::WRL::ComPtr<ID3D11DeviceContext>    getContext(void);
            static Microsoft::WRL::ComPtr<IDXGISwapChain>         getSwapChain(void);
            static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> getRenderTargetView(void);
            static Microsoft::WRL::ComPtr<ID3D11DepthStencilView> getDepthStencilView(void);

            static Microsoft::WRL::ComPtr<ID3D11Buffer> getConstantBuffer(void);
            static const D3D11_VIEWPORT& getViewPort(void);
            static Microsoft::WRL::ComPtr<ID3D11SamplerState> getSamplerState(void);

            static bool setViewPort(const float& width, const float& height);
            static bool setWVPMatrices(const DirectX::XMVECTOR& eye, const DirectX::XMVECTOR& up, const float& fov = 75.0f, const float& nearZ = 0.1f, const float& farZ = 100.0f);

            static const SHADER_RESOURCE* getShader(const WString& key, const D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElements);
            static HRESULT compileShader(const WString& path, const String& entryPoint, const String& target, ID3D10Blob** blobOut);

            template <typename T> static std::vector<MESH_RESOURCE>* getModel(const WString& key);

            static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> getTexture(const WString& filePath);

        private:

            // クラスをインスタンス化せず関数を呼び出す設計のため削除
            DirectXManager(void) = delete;
            DirectXManager(const DirectXManager&) = delete;
            ~DirectXManager(void) = delete;
            DirectXManager& operator=(const DirectXManager&) = delete;

            static bool createDevice(void);
            static bool createSwapChain(const HWND& hwnd, const UINT& width, const UINT& height);
            static bool createRenderTarget(void);
            static bool createDepthStencilBuffer(const UINT& width, const UINT& height);
            static bool createConstantBuffer(void);
            static bool createSamplerState(void);

            static Microsoft::WRL::ComPtr<ID3D11Device>           mDevice;
            static Microsoft::WRL::ComPtr<ID3D11DeviceContext>    mContext;
            static Microsoft::WRL::ComPtr<IDXGISwapChain>         mSwapChain;
            static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRenderTargetView;
            static Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;
            static Microsoft::WRL::ComPtr<ID3D11SamplerState>     mSamplerState;

            static Microsoft::WRL::ComPtr<ID3D11Buffer>           mConstantBuffer;
            static Microsoft::WRL::ComPtr<ID3D11Texture2D>        mDepthStencilTexture;

            static D3D_DRIVER_TYPE   mDriverType;
            static D3D_FEATURE_LEVEL mFeatureLevel;
            static DXGI_FORMAT       mBufferFormat;

            static D3D11_VIEWPORT    mViewport;

            static std::unordered_map<WString, SHADER_RESOURCE>            mShaderCache;
            static std::unordered_map<WString, std::vector<MESH_RESOURCE>> mMeshCache;
            static std::unordered_map<WString, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> > mTextureCache;
        };
    }
}