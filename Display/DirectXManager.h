#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>

typedef Microsoft::WRL::ComPtr<ID3D11Device> DevicePtr;
typedef Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContextPtr;
typedef Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChainPtr;
typedef Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RenderTargetViewPtr;
typedef Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBuffer,VertexBuffer,IndexBuffer,StructuredBuffer;
typedef Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
typedef Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
typedef Microsoft::WRL::ComPtr<ID3D11GeometryShader> GeometryShader;
typedef Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
typedef Microsoft::WRL::ComPtr<ID3D11ComputeShader> ComputeShader;
typedef Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture2D;
typedef Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderTexture;
typedef Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> ComputeOutputView;

/*-------------------------------------------------------------------
Å°namespace ark::render;
-------------------------------------------------------------------*/
namespace ark
{
    namespace render
    {
        /*-------------------------------------------------------------------
        Å°ï\é¶êßå‰ÉNÉâÉX
        -------------------------------------------------------------------*/
        class DirectXManager
        {
        public:

            static bool init(void);
            static void close(void);

            static DevicePtr        getDevice(void);
            static DeviceContextPtr getDeviceContext(void);

        private:

            DirectXManager(void);
            DirectXManager(const DirectXManager& copy);
            ~DirectXManager(void);

            DirectXManager& operator=(const DirectXManager& copy);

            static bool createDirect3D(void);
            static bool createDevice(void);
            static bool hardwareCheck(void);

            static DevicePtr           mpDevice;
            static DeviceContextPtr    mpContext;
            static SwapChainPtr        mpSwapChain;
            static RenderTargetViewPtr mpRenderTargetView;
            static D3D11_VIEWPORT      mViewport;
        };
    }
}