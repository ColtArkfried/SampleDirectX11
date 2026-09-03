#pragma once

// 内部で使用する標準ライブラリ
#include <unordered_map>
#include <typeinfo>
#include <utility>
#include <algorithm>
#include <cmath>

/*-------------------------------------------------------------------
*namespace Ark::Scene;
-------------------------------------------------------------------*/
namespace Ark
{
    namespace Scene
    {
        // 前方宣言
        class Camera;

        /*-------------------------------------------------------------------
        // ゲームオブジェクトマネージャー
        すべてのオブジェクトの配列と各タイプの配列、メモリプールを管理する
        -------------------------------------------------------------------*/
        class ObjectManager
        {
        public:
            /*==========================================================
            // オブジェクトプール対応の生成関数
            外部からは addObject ではなく、この関数を使って生成します。
            例: manager->createObject<ModelObject>(引数...);
            ==========================================================*/
            template<typename T, typename... Args>
            static T* createObject(Args&&... args)
            {
                // 生成するクラスの型情報をハッシュ値として取得
                size_t typeHash = typeid(T).hash_code();
                void* rawMemory = nullptr;

                // プールに待機中の空きメモリ(同じ型)があるか確認
                if(mPools.count(typeHash) && !mPools[typeHash].empty())
                {
                    // プールからメモリを取り出す（OSからの新規割り当てを回避）
                    rawMemory = mPools[typeHash].back();
                    mPools[typeHash].pop_back();
                }
                else
                {
                    // プールが空なら、型のサイズ分だけ純粋なメモリを新規確保
                    rawMemory = ::operator new(sizeof(T));
                }

                // 「配置new (Placement new)」を利用して、確保済みのメモリ上にコンストラクタを呼び出す
                T* obj = new(rawMemory) T(std::forward<Args>(args)...);

                // マスター配列に追加
                mAllObjects.push_back(obj);

                // 派生クラスで決定された mType によってカテゴリ別配列に振り分ける
                registerObject(obj);

                return obj;
            }


            // 初期化・更新処理などを想定
            static void setupAll(void);

            // 座標関連の処理
            static void updateAll(void);

            // 削除フラグ(mDeleteFlag)が立っているオブジェクトを一括破棄
            static void removeGarbage(void);

            // 全オブジェクトの破棄（アプリケーション終了時など）
            static void cleanupAll(void);

            // 描画処理
            static void drawAll(void);

        private:

            // クラスをインスタンス化せず関数を呼び出す設計のため削除
            ObjectManager(void) = delete;
            ObjectManager(const ObjectManager&) = delete;
            ~ObjectManager(void) = delete;
            ObjectManager& operator=(const ObjectManager&) = delete;

            // Swap and Pop を行うラムダ式（C++11互換）
            template<typename T>
            static void swapAndPopGarbage(std::vector<T*>& vec)
            {
                for(size_t i = 0; i < vec.size(); )
                {
                    if(vec[i]->getDeleteFlag())
                    {
                        vec[i] = vec.back();
                        vec.pop_back();
                    }
                    else
                    {
                        ++i;
                    }
                }
            }

            static void registerObject(Camera* camera) { mCameras.push_back(camera); }
            static void registerObject(Light* light)   { mLights.push_back(light);   }
            static void registerObject(Model* model)   { mModels.push_back(model);   }

            // 全てのBaseObjectのポインタを保持するマスター配列
            static std::vector<BaseObject*> mAllObjects;

            // 各カテゴリごとに区切った参照用配列
            static std::vector<Camera*> mCameras;
            static std::vector<Light*>  mLights;
            static std::vector<Model*>  mModels;

            // オブジェクトプール（型のハッシュ値 -> 空きメモリのポインタ配列）
            static std::unordered_map<size_t, std::vector<void*>> mPools;
        };
    }
}