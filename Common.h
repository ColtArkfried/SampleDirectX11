// ------------------------------------------------------------------
// *プログラム上で共通に扱う定義とヘッダーのインクルード
// ------------------------------------------------------------------
#pragma once

// ------------------------------------------------------------------
// *標準ライブラリインクルード
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <new>

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#include <windows.h>

// デバッグ時にインクルード
#ifdef _DEBUG
    #ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
        #define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
    #endif
    #ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT
        #define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT
    #endif
    #include <crtdbg.h>
#endif

// ------------------------------------------------------------------
// *変数やクラス名の設定
typedef unsigned long  ulong;
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef wchar_t        wchar;

typedef std::wstring                      WString;
typedef std::string                       String;
typedef std::vector<std::string>          StringStack;

// ------------------------------------------------------------------
// *常に使用するネームスペース
using namespace std;

// ------------------------------------------------------------------
// *マクロ関連
// ------------------------------------------------------------------
#define DELETE_OBJ(p){ if(p){ delete   (p); (p)=nullptr; } }
#define DELETE_ARY(p){ if(p){ delete[] (p); (p)=nullptr; } }
#define RELEASE(p){    if(p){ (p)->Release(); (p)=nullptr; } }

// ------------------------------------------------------------------
// *デバッグ時
#ifdef _DEBUG

    // ------------------------------------------------------------------
    // *メモリーリークチェック
    static void _leakcheck(void)
    {
        _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
    }

    // ------------------------------------------------------------------
    // *ログを出す
    static void _dump(const char* fmt,...)
    {
        char tmp[256] = {0};
        va_list args = nullptr;

        __crt_va_start(args,fmt);
        vsnprintf_s(tmp,sizeof(tmp),(sizeof(tmp) - 1),fmt,args);
        __crt_va_end(args);

        size_t len = strlen(tmp);
        if(len + 1 < (sizeof(tmp) - 1))
        {
            tmp[len] = '\n';
            tmp[len + 1] = '\0';
        }
        OutputDebugStringA(tmp);
    }

    // ------------------------------------------------------------------
    // *その場でプログラムを停止させてメッセージを出す
    static void _debugbreak(const char* view,const char* file,long line)
    {
        _dump("Error\n %s (%d)\n %s\n",file,line,view);
        DebugBreak();
    }

    #define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
    #define LEAKCHECK _leakcheck
    #define DUMP      _dump
    #define DEBUGBREAK(x) _debugbreak(x,__FILE__,__LINE__)


// ------------------------------------------------------------------
// *リリース時
#else

    #define NEW new
    #define LEAKCHECK __noop
    #define DUMP      __noop
    #define DEBUGBREAK     __noop

#endif