// main.cpp - UnoEngineを使用したサンプル
#include "UnoEngine.h"
#include "GameSceneFactory.h"
#include "D3DResourceCheck.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    // リソースリーク検出用
    D3DResourceLeakChecker leakCheck;

    try {
        // COM初期化
        CoInitializeEx(0, COINIT_MULTITHREADED);

        // エンジンのインスタンスを取得
        UnoEngine* engine = UnoEngine::GetInstance();

        // エンジンの初期化
        engine->Initialize();

        // シーンファクトリーの作成と設定
        SceneFactory* sceneFactory = new GameSceneFactory();
        engine->SetSceneFactory(sceneFactory);

        // 初期シーンへの遷移（SceneManager経由）
        engine->GetSceneManager()->ChangeScene("Title");

        // ゲームループの実行
        engine->Run();

        // COM終了処理
        CoUninitialize();
    }
    catch (const std::exception& e) {
        // 例外発生時のエラーメッセージ表示
        MessageBoxA(nullptr, e.what(), "エラーが発生しました", MB_OK | MB_ICONERROR);
        return -1;
    }

    return 0;
}