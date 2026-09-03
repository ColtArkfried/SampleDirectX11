#pragma once

// DirectX用
#include <DirectXMath.h>
#include <d3d11.h>

/*-------------------------------------------------------------------
*namespace Ark::Scene;
-------------------------------------------------------------------*/
namespace Ark
{
    namespace Scene
    {
        /*-------------------------------------------------------------------
        // オブジェクトカテゴリ
        mType（BaseObject::setType）に設定する種別として使用する。
        -------------------------------------------------------------------*/
        enum class ObjectType : unsigned int
        {
            eNullObject = 0, // 未使用
            eLight,          // ライト
            eCamera,         // カメラ
            eModel,          // 3Dモデル
        };

        /*-------------------------------------------------------------------
        // ベースクラス
        -------------------------------------------------------------------*/
        class BaseObject
        {
        public:

            BaseObject(void);
            virtual ~BaseObject(void);

            BaseObject(const BaseObject& copy);

            const BaseObject& operator=(const BaseObject& copy);

            virtual void setup(void)=0;
            virtual void update(void)=0;
            virtual void cleanup(void)=0;

            const ObjectType getType(void) const { return mType; }

            // 活性・非活性のフラグ
            void processDisable(void) { mProcessFlag = false; }
            void processEnable(void)  { mProcessFlag = true;  }
            const bool getProcessFlag(void) const { return mProcessFlag; }

            // 削除フラグ
            void deleteObject(void) { mDeleteFlag = true; }
            const bool getDeleteFlag(void) const { return mDeleteFlag; }

            const DirectX::XMFLOAT3& getPosition(void) const { return mPosition; }
            void setPosition(const DirectX::XMFLOAT3& pos) { mPosition = pos; mIsDirty = true; }
            void setPosition(float vx, float vy, float vz) { mPosition.x = vx; mPosition.y = vy; mPosition.z = vz; mIsDirty = true; }

            void addPosition(const DirectX::XMFLOAT3& pos){
                mPosition.x += pos.x;
                mPosition.y += pos.y;
                mPosition.z += pos.z;
                mIsDirty = true;
            }
            void addPosition(float vx, float vy, float vz){
                mPosition.x += vx;
                mPosition.y += vy;
                mPosition.z += vz;
                mIsDirty = true; 
            }

        protected:

            // 識別の変更は外側から行えない
            void setType(ObjectType type) { mType = type; }

            ObjectType mType;        // 識別用
            bool       mProcessFlag; // マネージャーに処理を行わせるフラグ
            bool       mDeleteFlag;  // マネージャーに破棄させるフラグ

            DirectX::XMFLOAT3 mPosition;
            bool mIsDirty = true; // 値に変更があった時だけ行列を計算するフラグ
        };
    }
}