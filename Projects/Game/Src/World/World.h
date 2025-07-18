#pragma once
#include "Framework/Framework.h"

/// <summary>
/// ゲーム固有の処理を含めた初期化や描画など
/// </summary>
class World final : public Framework {
public:
	World() = default;
	World(const World&) = delete;
	World(World&&) = delete;
	~World() = default;

	World& operator=(const World&) = delete;
	World& operator=(World&&) = delete;

private:
	void Initialize() override;

	void Finalize() override;

	void Update() override;

	void Draw()override;

private:
};