#pragma once

// 継承元
#include "BaseObject.h"

/*-------------------------------------------------------------------
*namespace Ark::Scene;
-------------------------------------------------------------------*/
namespace Ark
{
    namespace Scene
    {
        /*-------------------------------------------------------------------
        // カメラ制御クラス
        -------------------------------------------------------------------*/
        class Camera : public BaseObject
        {
        public:
            Camera(void);
            virtual ~Camera(void) override;

            virtual void setup(void) override;
            virtual void update(void) override;
            virtual void cleanup(void) override;

            // 速度
            void setMoveSpeed(float f) { mMoveSpeed = f; }
            const float getMoveSpeed(void) const { return mMoveSpeed; }

            void setMove(const DirectX::XMFLOAT3& v) { mMove = v; }
            void setMove(float vx, float vy, float vz) { mMove.x = vx; mMove.y = vy; mMove.z = vz; }
            const DirectX::XMFLOAT3& getMove(void) const { return mMove; }

            // 追従ターゲットの設定
            void setTarget(const BaseObject* target) { mTargetObject = target; }

            // カメラの各種パラメータ設定
            void setLookAt(const DirectX::XMFLOAT3& lookAt) { mLookAt = lookAt; mIsDirty = true; }
            void setLookAt(float vx, float vy, float vz) { mLookAt.x = vx; mLookAt.y = vy; mLookAt.z = vz; mIsDirty = true; }

            void addLookAt(const DirectX::XMFLOAT3& lookAt) { mLookAt.x += lookAt.x; mLookAt.y += lookAt.y;  mLookAt.z += lookAt.z;  mIsDirty = true; }
            void addLookAt(float vx, float vy, float vz) { mLookAt.x += vx; mLookAt.y += vy; mLookAt.z += vz;mIsDirty = true; }

            void setUpVector(const DirectX::XMFLOAT3& up) { mUp = up; mIsDirty = true; }
            void setUpVector(float vx, float vy, float vz) { mUp.x = vx; mUp.y = vy; mUp.z = vz; mIsDirty = true; }

            // 行列の取得
            DirectX::XMMATRIX getViewMatrix(void) const { return mViewMatrix; };
            DirectX::XMMATRIX getProjectionMatrix(void) const { return mProjectionMatrix; };

        private:

            void setPerspectiveFovLH(float fovAngleY, float aspectRatio, float nearZ, float farZ);
            void updateProjectionMatrix(void);
            void updateViewMatrix(void);

            void updateFreeCamera(void);
            void updateTrackingCamera(void);

            // 追従対象のオブジェクト
            const BaseObject* mTargetObject;

            // カメラの姿勢情報
            DirectX::XMFLOAT3 mLookAt; // 注視点
            DirectX::XMFLOAT3 mUp;     // 上方向ベクトル

            float       mMoveSpeed   = 0.5f;   // 移動速度;
            float       mRotateSpeed = 0.005f; // 回転速度;
            DirectX::XMFLOAT3 mMove;           // 移動量

            // 行列データ
            DirectX::XMMATRIX mViewMatrix;
            DirectX::XMMATRIX mProjectionMatrix;


            // プロジェクション用パラメータ
            float mFovAngleY;
            float mAspectRatio;
            float mNearZ;
            float mFarZ;


        };
    }
}


