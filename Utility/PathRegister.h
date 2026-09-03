#pragma once
#include <fstream>
#include <unordered_map>

/*-------------------------------------------------------------------
// namespace Ark::Utility;
-------------------------------------------------------------------*/
namespace Ark
{
    namespace Utility
    {
        // コンテナの名前をつける
        typedef std::unordered_map<std::wstring, std::wstring> StringMap;

        /*-------------------------------------------------------------------
        // ファイルパス登録システム
        -------------------------------------------------------------------*/
        class PathRegister
        {
        public:

            // 登録
            static bool set(const wchar* key, const wchar* path)
            {
                try
                {
                    if(key == nullptr || path == nullptr)
                    {
                        throw("エラー:引数が入力されていません");
                    }

                    // キーの重複チェック
                    auto it = mPathList.find(key);
                    if(it != mPathList.end())
                    {
                        throw("エラー:すでにキーが登録されています");
                    }

                    // ワイド文字列パスの存在チェック（ifstreamを使用）
                    std::ifstream check(path);
                    if(!check.is_open())
                    {
                        throw("エラー:ファイルが開けません(パスが存在しない可能性があります)");
                    }
                    check.close();

                    // マップに登録
                    mPathList[key] = path;
                }
                catch(const char* msg)
                {
                    // デバッグ時にメッセージを出す
                    DEBUGBREAK(msg);
                    return false;
                }

                return true;
            }

            // 取り出す
            static WString get(const wchar* key)
            {
                try
                {
                    if(key == nullptr)
                    {
                        throw("エラー:引数が入力されていません");
                    }

                    // キーが存在するか確認
                    auto it = mPathList.find(key);
                    if(it == mPathList.end())
                    {
                        throw("エラー:対応するパスが見つかりません");
                    }

                    return it->second; // 正常なパス文字列を返す
                }
                catch(const char* msg)
                {
                    // デバッグ時にメッセージを出す
                    DEBUGBREAK(msg);
                }

                return L""; // エラー時は空文字を返す
            }
        private:
            static StringMap mPathList;
        };
    }

}
