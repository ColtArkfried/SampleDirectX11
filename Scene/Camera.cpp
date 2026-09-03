#include "../Common.h"
#include "../Utility.h"
#include "../Device.h"
#include "Camera.h"
using namespace DirectX;
using namespace Ark::Scene;

Camera::Camera(void) 
    : mTargetObject(nullptr)
    , mLookAt(0.0f, 0.0f, 0.0f)
    , mUp(0.0f, 1.0f, 0.0f)
    , mMove(0.0f, 0.0f, 0.0f)
    , mFovAngleY(DirectX::XM_PIDIV4)
    , mAspectRatio(16.0f / 9.0f)
    , mNearZ(0.1f)
    , mFarZ(1000.0f)
{
    mViewMatrix = XMMatrixIdentity();
    mProjectionMatrix = XMMatrixIdentity();

    // カメラとして設定
    setType(ObjectType::eCamera);
}

Camera::~Camera(void)
{
}

void Camera::setup(void)
{
    mTargetObject = nullptr;
    mPosition = XMFLOAT3(0.0f, 2.0f, -10.0f);
    mLookAt   = XMFLOAT3(0.0f, 1.0f, 0.0f);
    mUp       = XMFLOAT3(0.0f, 1.0f, 0.0f);

    mMove = XMFLOAT3(0.0f, 0.0f, 0.0f);

    mIsDirty = true;

    // パラメータ設定とプロジェクション行列の初期計算
    setPerspectiveFovLH(XM_PIDIV4, 16.0f / 9.0f, 0.1f, 1000.0f);

    // ビュー行列の初期計算
    updateViewMatrix();
}

void Camera::update(void)
{
    // ターゲットの有無で挙動を切り替える
    if(mTargetObject)
    {
        updateTrackingCamera();
    }
    else
    {
        updateFreeCamera();
    }

    // 毎フレーム変わるのは位置や注視点なのでビュー行列のみ更新
    if(mIsDirty)
    {
        updateViewMatrix();
        mIsDirty = false;
    }
}

void Camera::cleanup(void)
{
    mTargetObject = nullptr;
}

void Camera::setPerspectiveFovLH(float fovAngleY, float aspectRatio, float nearZ, float farZ)
{
    mFovAngleY = fovAngleY;
    mAspectRatio = aspectRatio;
    mNearZ = nearZ;
    mFarZ = farZ;

    // 設定変更時のみプロジェクション行列を再計算
    updateProjectionMatrix();
}

void Camera::updateProjectionMatrix(void)
{
    mProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(mFovAngleY, mAspectRatio, mNearZ, mFarZ);
}

void Camera::updateViewMatrix(void)
{
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&mPosition);
    DirectX::XMVECTOR lookAt = DirectX::XMLoadFloat3(&mLookAt);
    DirectX::XMVECTOR up = DirectX::XMLoadFloat3(&mUp);

    mViewMatrix = DirectX::XMMatrixLookAtLH(pos, lookAt, up);
}

void Camera::updateTrackingCamera(void)
{
    // 追従対象の現在の位置を取得し、カメラの注視点に設定
    mLookAt = mTargetObject->getPosition();

    // ※ここに「ターゲットの背後にカメラの mPos を移動させる」処理などを記述
    // 例: ターゲットの後ろ 10.0f、上に 5.0f の位置に配置するなど
}

// 自由移動カメラの処理
void Camera::updateFreeCamera(void)
{
    // -------------------------------------------------------------
    // 右クリックドラッグによる視点回転（フリールック）
    // -------------------------------------------------------------
    if(Input::isMouseDown(Ark::Device::MouseButton::eRight))
    {
        DirectX::XMFLOAT2 delta = Input::getMouseDelta();

        if(delta.x != 0.0f || delta.y != 0.0f)
        {
            float rotateSpeed = 0.003f; // 感度

            XMVECTOR pos = XMLoadFloat3(&mPosition);
            XMVECTOR look = XMLoadFloat3(&mLookAt);
            XMVECTOR dir = XMVectorSubtract(look, pos); // 視線ベクトル

            // Y軸（左右）回転
            float yaw = delta.x * rotateSpeed;
            XMMATRIX rotY = XMMatrixRotationY(yaw);
            dir = XMVector3TransformNormal(dir, rotY);

            // ローカルX軸（上下）回転
            float pitch = delta.y * rotateSpeed;
            XMVECTOR up = XMLoadFloat3(&mUp);
            XMVECTOR right = XMVector3Normalize(XMVector3Cross(up, dir));
            XMMATRIX rotX = XMMatrixRotationAxis(right, pitch);
            dir = XMVector3TransformNormal(dir, rotX);

            // 新しい注視点を設定
            XMFLOAT3 newLookAt;
            XMStoreFloat3(&newLookAt, XMVectorAdd(pos, dir));
            setLookAt(newLookAt);
        }
    }

    // -------------------------------------------------------------
    // 向いている方向に応じた WASD 移動
    // -------------------------------------------------------------
    // 現在の「前方向（Forward）」ベクトルを求める (mLookAt - mPosition)
    XMVECTOR posVector = XMLoadFloat3(&mPosition);
    XMVECTOR lookVector = XMLoadFloat3(&mLookAt);
    XMVECTOR forward = XMVector3Normalize(XMVectorSubtract(lookVector, posVector));

    // 現在の「右方向（Right）」ベクトルを求める (Up × Forward)
    XMVECTOR upVector = XMLoadFloat3(&mUp);
    XMVECTOR right = XMVector3Normalize(XMVector3Cross(upVector, forward));

    // 移動ベクトル（キー入力を蓄積）
    XMVECTOR moveDir = XMVectorZero();

    if(Input::isKeyDown('W')) moveDir = XMVectorAdd(moveDir,      forward); // 前進
    if(Input::isKeyDown('S')) moveDir = XMVectorSubtract(moveDir, forward); // 後退
    if(Input::isKeyDown('D')) moveDir = XMVectorAdd(moveDir,      right);   // 右移動
    if(Input::isKeyDown('A')) moveDir = XMVectorSubtract(moveDir, right);   // 左移動

    // 斜め移動したときに移動速度が速くならないよう正規化
    if(XMVector3LengthSq(moveDir).m128_f32[0] > 0.0001f)
    {
        moveDir = XMVector3Normalize(moveDir);

        // 速度を掛けて mMove に加算
        XMVECTOR moveVelocity = XMVectorScale(moveDir, mMoveSpeed);

        XMFLOAT3 moveDelta;
        XMStoreFloat3(&moveDelta, moveVelocity);

        // カメラ位置と注視点の両方を平行移動させる
        addPosition(moveDelta);
        addLookAt(moveDelta);
    }
}