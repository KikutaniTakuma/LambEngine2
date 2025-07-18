#pragma once
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
#include "Math/Matrix.h"



/// <summary>
/// トランスフォーム
/// </summary>
struct Transform {
	Transform() = default;
	Transform(const Transform&) = default;
	Transform(Transform&&) = default;

	Vector3 scale = Vector3::kIdentity;
	Vector3 rotate;
	Vector3 translate;

	Transform& operator=(const Transform&) = default;
	Transform& operator=(Transform&&) = default;

	Transform& operator=(const struct QuaternionTransform& transform);
	Mat4x4 CreateMatrix() const;
	void Debug(const std::string& guiName);
};

struct QuaternionTransform {
	QuaternionTransform() = default;
	QuaternionTransform(const QuaternionTransform&) = default;
	QuaternionTransform(QuaternionTransform&&) = default;

	Vector3 scale = Vector3::kIdentity;
	Quaternion rotate = Quaternion::kIdentity;
	Vector3 translate;

	QuaternionTransform& operator=(const Transform& transform);
	QuaternionTransform& operator=(const QuaternionTransform&) = default;
	QuaternionTransform& operator=(QuaternionTransform&&) = default;

	Mat4x4 CreateMatrix() const;
	void Debug(const std::string& guiName);
};

namespace Lamb {
	namespace Guizmo {
		enum class Type {
			TRANSLATE = 7,
			ROTATE =  120,
			SCALE = 896
		};

		/// <summary>
		/// ギズモ
		/// </summary>
		/// <param name="worldMatrix">ワールド行列(一時変数を使わないこと)</param>
		/// <param name="view">view行列</param>
		/// <param name="projection">projection行列</param>
		/// <param name="type">ギズモタイプ</param>
		void Manipulate(
			Mat4x4& worldMatrix, 
			const Mat4x4& view, 
			const Mat4x4& projection,
			Type type
		);
	};
};