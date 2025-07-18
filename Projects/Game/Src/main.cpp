#include <Windows.h>
#include <memory>
#include "World/World.h"

/// <summary>
/// エントリーポイント
/// </summary>
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	std::unique_ptr<Framework> game = std::make_unique<World>();

	// 実行
	game->Execution();

	return 0;
}