#include "../Common.h"
#include "../Utility.h"
#include "../Device.h"
#include "Plane.h"
using namespace Microsoft::WRL;
using namespace DirectX;
using namespace Ark::Scene;
using namespace Ark::Device;

//-------------------------------------------------------------------
// コンストラクタ・デストラクタ
Plane::Plane(void)
{
}

Plane::~Plane(void)
{
}

void Plane::setup(void)
{
    try
    {
        // DirectXデバイスを取得
        ComPtr<ID3D11Device> device = DirectXManager::getDevice();
        if(device == nullptr)
        {
            throw("エラー:DirectXデバイスの取得に失敗しました");
        }

        D3D11_BUFFER_DESC cbd = {};
        cbd.Usage = D3D11_USAGE_DEFAULT;
        cbd.ByteWidth = sizeof(Ark::Device::DirectXManager::CONSTANT_BUFFER);
        cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        if( FAILED(device->CreateBuffer(&cbd, nullptr, mConstantBuffer.ReleaseAndGetAddressOf())) )
        {
            throw("エラー:定数バッファの作成に失敗しました");
        }
 
        mModelMeshes = DirectXManager::getModel<Plane::VERTEX>(mModelKey);
        if(mModelMeshes == nullptr)
        {
            throw("エラー:モデルのインスタンス化に失敗しました");
        }

        // 地面のテクスチャ
        WString path = PathRegister::get(L"ground_tex");
        mModelMeshes->at(0).pTextures[0] = DirectXManager::getTexture(path);

        // シェーダーをコンパイルしてインスタンス化
        mShader = DirectXManager::getShader(mModelKey, VERTEX_INFO, ARRAYSIZE(VERTEX_INFO));
        if(mShader == nullptr)
        {
            throw("エラー:シェーダーのインスタンス化に失敗しました");
        }
    }
    catch(const char* msg)
    {
        // デバッグ時にメッセージを出す
        DEBUGBREAK(msg);

        cleanup();
        return;
    }
}

void Plane::update(void)
{
    Model::update();
}

void Plane::draw(const RenderContext& rc)
{

    if(!mModelMeshes || !mShader) return;

    // デバイスコンテキストの取得
    ID3D11DeviceContext* context = rc.pContext;
    if(!context) return;

    auto sampler = DirectXManager::getSamplerState();
    context->PSSetSamplers(0, 1, sampler.GetAddressOf());

    // シェーダーをバインド
    context->VSSetShader(mShader->pVS.Get(), nullptr, 0);
    context->PSSetShader(mShader->pPS.Get(), nullptr, 0);
    context->IASetInputLayout(mShader->pLayout.Get());

    // トポロジーを設定
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // シェーダーの定数バッファ構造体に合わせてデータを準備
    Ark::Device::DirectXManager::CONSTANT_BUFFER cbData = {};

    // 各行列を転置して格納
    DirectX::XMStoreFloat4x4(&cbData.world, DirectX::XMMatrixTranspose(mWorldMatrix));
    DirectX::XMStoreFloat4x4(&cbData.view, DirectX::XMMatrixTranspose(rc.viewMatrix));
    DirectX::XMStoreFloat4x4(&cbData.projection, DirectX::XMMatrixTranspose(rc.projectionMatrix));

    // 定数バッファへ構造体の転送
    context->UpdateSubresource(mConstantBuffer.Get(), 0, nullptr, &cbData, 0, 0);
    context->VSSetConstantBuffers(0, 1, mConstantBuffer.GetAddressOf());

    UINT stride = sizeof(Ark::Scene::Plane::VERTEX);
    UINT offset = 0;

    for(const auto& mesh : *mModelMeshes)
    {
        // 頂点バッファとインデックスバッファをセット
        context->IASetVertexBuffers(0, 1, mesh.pVertex.GetAddressOf(), &stride, &offset);
        context->IASetIndexBuffer(mesh.pIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

        // 複数テクスチャをスロット0〜に一括セット
        ID3D11ShaderResourceView* views[TEX_MAX] = {};
        for(int t = 0; t < TEX_MAX; ++t) {
            views[t] = mesh.pTextures[t].Get();
        }
        context->PSSetShaderResources(0, TEX_MAX, views);

        // 描画実行
        context->DrawIndexed(mesh.indexCount, 0, 0);
    }
}

void Plane::cleanup(void)
{

}