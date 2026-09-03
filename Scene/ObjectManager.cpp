#include "../Common.h"
#include "../Utility.h"
#include "../Device.h"
#include "../Scene.h"
using namespace Microsoft::WRL;
using namespace DirectX;
using namespace Ark::Device;
using namespace Ark::Scene;

/*-------------------------------------------------------------------
*スタティックメンバー
-------------------------------------------------------------------*/

std::vector<BaseObject*>                       ObjectManager::mAllObjects;
std::vector<Camera*>                           ObjectManager::mCameras;
std::vector<Light*>                            ObjectManager::mLights;
std::vector<Model*>                            ObjectManager::mModels;
std::unordered_map<size_t, std::vector<void*>> ObjectManager::mPools;

void ObjectManager::setupAll()
{
    for(auto* obj : mAllObjects)
    {
        // 処理フラグが有効な場合のみセットアップ
        if(obj->getProcessFlag() && !obj->getDeleteFlag())
        {
            obj->setup();
        }
    }
}

void ObjectManager::updateAll(void)
{
    // 1. カメラの処理
    for(auto* camera : mCameras)
    {
        if(camera->getProcessFlag() && !camera->getDeleteFlag())
        {
            camera->update();
        }
    }

    // 2. ライトの処理
    for(auto* light : mLights)
    {
        if(light->getProcessFlag() && !light->getDeleteFlag())
        {
            light->update();
        }
    }

    // 3. 3Dモデルの移動や回転などの処理
    for(auto* model : mModels)
    {
        if(model->getProcessFlag() && !model->getDeleteFlag())
        {
            model->update();
        }
    }

    // 3. 3Dモデルの処理 (アルファブレンドを考慮したZソート)
    // ※半透明オブジェクトを描画する場合は、カメラから遠い順(奥から手前)にソートする必要があります。
    if(!mModels.empty() && !mCameras.empty())
    {
        // メインカメラの座標を取得 (mCameras[0]をメインカメラと仮定)
        DirectX::XMFLOAT3 cameraPos = mCameras[0]->getPosition();

        std::sort(mModels.begin(), mModels.end(), [&cameraPos](BaseObject* a, BaseObject* b) {
            DirectX::XMFLOAT3 posA = a->getPosition();
            DirectX::XMFLOAT3 posB = b->getPosition();

            // カメラからの距離の二乗を計算 (sqrtを省いて高速化)
            float distA = (posA.x - cameraPos.x) * (posA.x - cameraPos.x) +
                (posA.y - cameraPos.y) * (posA.y - cameraPos.y) +
                (posA.z - cameraPos.z) * (posA.z - cameraPos.z);

            float distB = (posB.x - cameraPos.x) * (posB.x - cameraPos.x) +
                (posB.y - cameraPos.y) * (posB.y - cameraPos.y) +
                (posB.z - cameraPos.z) * (posB.z - cameraPos.z);

            // 遠い順(降順)にソート
            return distA > distB;
        });
    }
}

// 描画処理
void ObjectManager::drawAll(void)
{
    ComPtr<ID3D11DeviceContext> context = DirectXManager::getContext();
    if(context == nullptr)
    {
        return;
    }

    if(mCameras.empty() || mModels.empty()) return;

    // 描画ループに入る前に、RenderContextを組み立てる
    static RenderContext mainContext = {0};
    mainContext.pContext         = context.Get();
    mainContext.viewMatrix       = mCameras[0]->getViewMatrix();       // メインカメラのView行列
    mainContext.projectionMatrix = mCameras[0]->getProjectionMatrix(); // メインカメラのProj行列

    // ソート済みの3Dモデルを描画
    for(auto* model : mModels)
    {
        if(model->getProcessFlag() && !model->getDeleteFlag())
        {
            model->draw(mainContext);
        }
    }
}


void ObjectManager::removeGarbage(void)
{
    // 各カテゴリ配列から除外（この時点ではポインタの整理のみ）
    swapAndPopGarbage(mCameras);
    swapAndPopGarbage(mLights);
    swapAndPopGarbage(mModels);

    // ==========================================================
    // 2. マスター配列からの Swap and Pop と、プールへのメモリ返却
    // ==========================================================
    for(size_t i = 0; i < mAllObjects.size(); )
    {
        BaseObject* obj = mAllObjects[i];
        if(obj->getDeleteFlag())
        {
            // 終了処理を実行
            obj->cleanup();

            // 実行時型情報(RTTI)を利用して、実際の派生クラスの型ハッシュを取得
            // (※BaseObjectに仮想関数が含まれているため、動的に正しい型が取得できます)
            size_t typeHash = typeid(*obj).hash_code();

            // デストラクタを明示的に呼び出す（※メモリ領域自体は OS に返却しない）
            obj->~BaseObject();

            // 空きメモリとしてプールにストックする
            mPools[typeHash].push_back(obj);

            // Swap and Pop でマスター配列から高速に除外
            mAllObjects[i] = mAllObjects.back();
            mAllObjects.pop_back();
        }
        else
        {
            ++i;
        }
    }
}

void ObjectManager::cleanupAll(void)
{
    // 1. 現在アクティブなオブジェクトの終了処理と完全解放
    for(auto* obj : mAllObjects)
    {
        obj->cleanup();
        obj->~BaseObject();       // デストラクタ呼び出し
        ::operator delete(obj);   // メモリ領域を完全にOSへ返却
    }

    mAllObjects.clear();
    mCameras.clear();
    mLights.clear();
    mModels.clear();

    // 2. プールで待機中の未使用メモリも完全解放
    for(auto& pair : mPools)
    {
        for(void* ptr : pair.second)
        {
            ::operator delete(ptr);
        }
    }
    mPools.clear();
}