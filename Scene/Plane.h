#pragma once
#include <wrl.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// 継承元
#include "Model.h"
#include "../Device/DirectXManager.h"

/*-------------------------------------------------------------------
// namespace Ark::Scene;
-------------------------------------------------------------------*/
namespace Ark
{

    namespace Scene
    {
        /*-------------------------------------------------------------------
        // 描画用抽象化クラス
        -------------------------------------------------------------------*/
        class Plane : public Model
        {
        public:

            Plane(void);
            virtual ~Plane(void) override;

            virtual void setup(void) override;
            virtual void update(void) override;
            virtual void draw(const RenderContext& rc) override;
            virtual void cleanup(void) override;

            void setModelKey(const WString& key) { mModelKey = key; }

            // 頂点フォーマット
            struct VERTEX
            {
                DirectX::XMFLOAT3 position; // 位置座標
                DirectX::XMFLOAT3 normal;   // 法線
                DirectX::XMFLOAT2 uv;       // テクスチャ座標
            };

            // シェーダに送る頂点情報
            static constexpr D3D11_INPUT_ELEMENT_DESC VERTEX_INFO[]
            {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };

        private:

            // 識別するキー
            WString mModelKey;

            // メッシュ群へのポインタ
            std::vector<Ark::Device::MESH_RESOURCE>* mModelMeshes = nullptr;
        };
    }
}