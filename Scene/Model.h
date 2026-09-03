#pragma once
#include <wrl.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// 継承元
#include "BaseObject.h"

/*-------------------------------------------------------------------
*namespace Ark::Scene;
-------------------------------------------------------------------*/
namespace Ark
{   
    // 前方宣言
    namespace Device
    {
        struct SHADER_RESOURCE;
    }

    namespace Scene
    {
        /*-------------------------------------------------------------------
        // 描画に必要なパラメータの構造体
        -------------------------------------------------------------------*/
        struct RenderContext
        {
            ID3D11DeviceContext* pContext;
            DirectX::XMMATRIX viewMatrix;
            DirectX::XMMATRIX projectionMatrix;
        };

        /*-------------------------------------------------------------------
        // 描画用抽象化クラス
        -------------------------------------------------------------------*/
        class Model : public BaseObject
        {
        public:

            Model(void);
            virtual ~Model(void) override;

            virtual void setup(void) override;
            virtual void update(void) override;
            virtual void draw(const RenderContext& rc) = 0;
            virtual void cleanup(void) override;

            const DirectX::XMFLOAT3& getRotation(void) const { return mRotation; }
            void setRotation(const DirectX::XMFLOAT3& rot) { mRotation = rot; mIsDirty = true; }
            void setRotation(float vx, float vy, float vz) { mRotation.x = vx; mRotation.y = vy; mRotation.z = vz; mIsDirty = true; }

            const DirectX::XMFLOAT3& getScale(void) const { return mScale; }
            void setScale(const DirectX::XMFLOAT3& scale)  { mScale = scale; mIsDirty = true; }
            void setScale(float vx, float vy, float vz) { mScale.x = vx; mScale.y = vy; mScale.z = vz; mIsDirty = true; }

            const DirectX::XMMATRIX& getWorldMatrix() const { return mWorldMatrix; }

        protected:

            void updateWorldMatrix(void);
            void createBuffers(ID3D11Device* device);

            DirectX::XMFLOAT3 mPosition = {0.0f, 0.0f, 0.0f};
            DirectX::XMFLOAT3 mRotation = {0.0f, 0.0f, 0.0f};
            DirectX::XMFLOAT3 mScale    = {1.0f, 1.0f, 1.0f};

            // キャッシュ用ワールド行列
            DirectX::XMMATRIX mWorldMatrix = DirectX::XMMatrixIdentity();

            // バッファ
            Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;   // 頂点バッファ
            Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;    // インデックスバッファ
            Microsoft::WRL::ComPtr<ID3D11Buffer> mConstantBuffer; // カメラのWVP行列をシェーダーへ送るバッファ

            // シェーダーインスタンス
            const Ark::Device::SHADER_RESOURCE* mShader;

        };
    }
}