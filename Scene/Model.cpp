#include "../Utility.h"
#include "../Display.h"
#include "Model.h"
using namespace Microsoft::WRL;
using namespace DirectX;
using namespace Ark::Scene;

//-------------------------------------------------------------------
// *コンストラクタ・デストラクタ
Model::Model(void)
    : mVertexBuffer(nullptr)
    , mIndexBuffer(nullptr)
    , mConstantBuffer(nullptr)
    , mShader(nullptr)
{
    mPosition = {0.0f, 0.0f, 0.0f};
    mRotation = {0.0f, 0.0f, 0.0f};
    mScale = {1.0f, 1.0f, 1.0f};

    // 行列初期化
    mWorldMatrix = XMMatrixIdentity();

    // モデルとして設定
    setType(ObjectType::eModel);
}

Model::~Model(void)
{

}

void Model::setup(void)
{
    mVertexBuffer   = nullptr;
    mIndexBuffer    = nullptr;
    mConstantBuffer = nullptr;
    mShader         = nullptr;

    mPosition = {0.0f, 0.0f, 0.0f};
    mRotation = {0.0f, 0.0f, 0.0f};
    mScale    = {1.0f, 1.0f, 1.0f};

    // 行列初期化
    mWorldMatrix = XMMatrixIdentity();

    // モデルとして設定
    setType(ObjectType::eModel);
}

void Model::update(void)
{
    updateWorldMatrix();
}

void Model::cleanup(void)
{

}

void Model::updateWorldMatrix(void)
{
    // 値に変更があった時だけ行列を計算する
    if(mIsDirty)
    {
        XMMATRIX scale     = XMMatrixScaling(mScale.x, mScale.y, mScale.z);
        XMMATRIX rotate    = XMMatrixRotationRollPitchYaw(mRotation.x, mRotation.y, mRotation.z);
        XMMATRIX translate = XMMatrixTranslation(mPosition.x, mPosition.y, mPosition.z);

        // SRT順（Scale -> Rotate -> Translation）で掛け合わせ
        mWorldMatrix = scale * rotate * translate;

        mIsDirty = false;
    }
}