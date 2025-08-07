#include <Windows.h>
#include "Framework/Framework.h"
#include <memory>

/// <summary>
/// エントリーポイント
/// </summary>
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	std::unique_ptr<Lamb::Framework> framework = std::make_unique<Lamb::Framework>();

	framework->Execution();

	return 0;
}