#include <windows.h>
#include <memory>
#include "WinApp.h"
#include "MyGame.h"
#include "D3DResourceCheck.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // リソースリーク検出用
    D3DResourceLeakChecker leakCheck;

    try {
        // COM初期化
        CoInitializeEx(0, COINIT_MULTITHREADED);

        // WindowsAPIの初期化 - unique_ptrで管理
        std::unique_ptr<WinApp> winApp = std::make_unique<WinApp>();
        winApp->Initialize();

        // ゲームの作成 - unique_ptrで管理
        std::unique_ptr<MyGame> game = std::make_unique<MyGame>();

        // WinAppを設定
        game->SetWinApp(winApp.get());

        // ゲームのメインループを実行
        game->Run();

        // ゲームのリソース解放（unique_ptrにより自動的に行われる）
        game.reset();

        // WindowsAPIの終了処理
        winApp->Finalize();
        winApp.reset();

        // COM終了処理
        CoUninitialize();
    }
    catch (const std::exception& e) {
        // 例外をキャッチしてメッセージボックスで表示
        MessageBoxA(nullptr, e.what(), "エラーが発生しました", MB_OK | MB_ICONERROR);
        return -1;
    }

    return 0;
}