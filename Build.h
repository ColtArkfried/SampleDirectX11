// ------------------------------------------------------------------
// // ビルドの設定
// ------------------------------------------------------------------
#pragma once

// ------------------------------------------------------------------
// ライブラリの使用
// ------------------------------------------------------------------
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// モデル読み込みライブラリ
#ifdef _DEBUG
#pragma comment(lib, "assimp-vc142-mtd.lib") // Debug 用
#else
#pragma comment(lib, "assimp-vc142-mt.lib")  // Release 用
#endif
