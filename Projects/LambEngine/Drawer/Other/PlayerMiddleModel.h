#pragma once
#pragma once
#include "../BaseDrawer.h"

class PlayerMiddleModel : public BaseDrawer {
public:
	static constexpr uint32_t kMaxDrawCount = 16;

public:
	struct PlayerModelData {
		float32_t3 playerFrontPosition;
		float32_t pad0 = 0.0f;
		float32_t3 playerBackPosition;
		float32_t pad1 = 0.0f;
		float32_t baseLength = 1.0f;
		float32_t middleSize = 1.0f;
		float32_t sideSize = 1.0f;
		int32_t isLighting = 1;

		int32_t isEffect = 0;
		int32_t backGroundTextureIndex = 0;
		struct GaussianState {
			float32_t2 dir;
			float32_t sigma = 0.0f;
			int32_t kernelSize = 0;
		} gausState;

#ifdef USE_DEBUG_CODE
		void Debug(const std::string& guiName);
#endif // USE_DEBUG_CODE

	};

public:
	PlayerMiddleModel() = default;
	PlayerMiddleModel(const PlayerMiddleModel&) = default;
	PlayerMiddleModel(PlayerMiddleModel&& right) noexcept = default;
	virtual ~PlayerMiddleModel() = default;

	PlayerMiddleModel& operator=(const PlayerMiddleModel& right) = default;
	PlayerMiddleModel& operator=(PlayerMiddleModel&& right) noexcept = default;

public:
	/// <summary>
	/// ロード
	/// </summary>
	/// <param name="fileName">ファイルパス</param>
	virtual void Load(const std::string& fileName = "./Resources/Player/Player_Middle_Model.glb");

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="worldMatrix">ワールド行列</param>
	/// <param name="camera">カメラ行列</param>
	/// <param name="color">色</param>
	/// <param name="blend">ブレンドタイプ</param>
	virtual void Draw(
		const Mat4x4& worldMatrix,
		const Mat4x4& camera,
		PlayerModelData playerModelData,
		uint32_t color,
		BlendType blend
	);

private:
	int32_t backGroundTextureIndex_ = 0;
};