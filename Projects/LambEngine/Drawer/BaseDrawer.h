/// ==============================
/// ==  BaseDrawerクラスの宣言  ==
/// ==============================

#pragma once
#include "Math/Matrix.h"
#include "Math/Quaternion.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/MathCommon.h"
#include <string>
#include "Engine/Graphics/GraphicsStructs.h"
#include "Transform/Transform.h"

/// <summary>
/// 描画クラスの基底クラス
/// </summary>
class BaseDrawer {
public:
	BaseDrawer();
	BaseDrawer(const BaseDrawer&) = default;
	BaseDrawer(BaseDrawer&&) = default;
	virtual ~BaseDrawer() = default;

public:
	BaseDrawer& operator=(const BaseDrawer&) = default;
	BaseDrawer& operator=(BaseDrawer&&) = default;

public:
	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="worldMatrix">ワールド行列</param>
	/// <param name="camera">カメラ行列</param>
	/// <param name="color">色</param>
	/// <param name="blend">ブレンドタイプ</param>
	virtual void Draw(const Mat4x4& worldMatrix, const Mat4x4& camera, uint32_t color, BlendType blend);

protected:
	class RenderSet* pRenderSet;
	class RenderSet* pMeshRenderSet;
	
	bool isUseMeshShader_ = false;
};

