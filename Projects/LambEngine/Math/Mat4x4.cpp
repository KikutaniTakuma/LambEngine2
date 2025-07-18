/// =====================================
/// ==  Matirx<float,4,4>クラスの宣言  ==
/// =====================================

#include "Matrix.h"
#include "Quaternion.h"
#include <cmath>
#include "Math/MathCommon.h"
#include <bit>

const Matrix<float,4,4> Matrix<float,4,4>::kIdentity = DirectX::XMMatrixIdentity();

const Matrix<float,4,4> Matrix<float,4,4>::kZero = Matrix<float,4,4>();

constexpr Matrix<float, 4, 4>& Matrix<float, 4, 4>::operator=(const DirectX::XMMATRIX& xmMatrix) {
	this->xmMatrix_ = xmMatrix;

	return *this;
}

Matrix<float,4,4> Matrix<float,4,4>::MakeTranslate(const Vector3& vec) {
	Matrix<float,4,4> result;

	result = DirectX::XMMatrixTranslation(vec.x, vec.y, vec.z);

	return result;
}

Matrix<float,4,4> Matrix<float,4,4>::MakeScale(const Vector3& vec) {
	Matrix<float,4,4> result;

	result = DirectX::XMMatrixScaling(vec.x, vec.y, vec.z);

	return result;
}

Matrix<float,4,4> Matrix<float,4,4>::MakeRotateX(float rad) {
	Matrix<float,4,4> result;

	result = DirectX::XMMatrixRotationX(rad);

	return result;
}

Matrix<float,4,4> Matrix<float,4,4>::MakeRotateY(float rad) {
	Matrix<float,4,4> result;

	result = DirectX::XMMatrixRotationY(rad);

	return result;
}

Matrix<float,4,4> Matrix<float,4,4>::MakeRotateZ(float rad) {
	Matrix<float,4,4> result;

	result = DirectX::XMMatrixRotationZ(rad);

	return result;
}

Matrix<float,4,4> Matrix<float,4,4>::MakeRotate(const Vector3& rad) {
	return DirectX::XMMatrixRotationRollPitchYawFromVector({ rad.x,rad.y, rad.z });
}

Matrix<float,4,4> Matrix<float,4,4>::MakeRotate(const Quaternion& rad)
{
	return DirectX::XMMatrixRotationQuaternion(rad.m128);
}

Matrix<float,4,4> Matrix<float,4,4>::MakeAffin(const Vector3& scale, const Vector3& rad, const Vector3& translate) {
	Matrix<float,4,4> result;

	/*Mat4x4 rotate = Mat4x4::MakeRotateX(rad.x) * Mat4x4::MakeRotateY(rad.y) * Mat4x4::MakeRotateZ(rad.z);

	result = Mat4x4(
		Mat4x4::vector_type{
			scale.x * rotate[0][0], scale.x * rotate[0][1],scale.x * rotate[0][2], 0.0f,
			scale.y * rotate[1][0], scale.y * rotate[1][1],scale.y * rotate[1][2], 0.0f,
			scale.z * rotate[2][0], scale.z * rotate[2][1],scale.z * rotate[2][2], 0.0f,
			translate.x, translate.y, translate.z, 1.0f
		}
	);*/

	Quaternion q = Quaternion::EulerToQuaternion(rad);

	result = MakeAffin(scale, q, translate);

	return result;
}

Matrix<float,4,4> Matrix<float,4,4>::MakeAffin(const Vector3& scale, const Quaternion& rad, const Vector3& translate)
{
	Matrix<float,4,4> result;

	result = DirectX::XMMatrixAffineTransformation({ scale.x, scale.y, scale.z }, {}, rad.m128, { translate.x,translate.y,translate.z });

	return result;
}

Matrix<float,4,4> Matrix<float,4,4>::MakeAffin(const Vector3& scale, const Vector3& from, const Vector3& to, const Vector3& translate) {
	Matrix<float,4,4> result;

	result = DirectX::XMMatrixAffineTransformation({ scale.x, scale.y, scale.z }, {}, Quaternion::DirectionToDirection(from, to).m128, { translate.x,translate.y,translate.z });

	return result;
}

Matrix<float,4,4> Matrix<float,4,4>::MakePerspectiveFov(float fovY, float aspectRatio, float nearClip, float farClip) {
	Matrix<float,4,4> result;

	/*
	result[0][0] = (1.0f / aspectRatio) * (1.0f / std::tan(fovY / 2.0f));
	result[1][1] = 1.0f / std::tan(fovY / 2.0f);
	result[2][2] = farClip / (farClip - nearClip);
	result[2][3] = 1.0f;
	result[3][2] = (-nearClip * farClip) / (farClip - nearClip);
	*/

	result = DirectX::XMMatrixPerspectiveFovLH(fovY, aspectRatio, nearClip, farClip);

	return result;
}

Matrix<float,4,4> Matrix<float,4,4>::MakeOrthographic(float width, float height, float nearClip, float farClip) {
	Matrix<float,4,4> result;

	/*
	result[0][0] = 2.0f / (right - left);
	result[1][1] = 2.0f / (top - bottom);
	result[2][2] = 1.0f / (farClip - nearClip);
	result[3][3] = 1.0f;

	result[3][0] = (left + right) / (left - right);
	result[3][1] = (top + bottom) / (bottom - top);
	result[3][2] = nearClip / (nearClip - farClip);
	*/

	result = DirectX::XMMatrixOrthographicLH(width, height, nearClip, farClip);

	return result;
}

Matrix<float, 4, 4> Matrix<float, 4, 4>::ConvertPerspectiveFovToOrthographic(float fovY, float aspectRatio, float nearClip, float farClip, float distanceZ) {
	// height の計算
	float height = 2.0f * std::clamp(distanceZ, nearClip, farClip) * std::tan(fovY * 0.5f);

	// width の計算
	float width = aspectRatio * height;

	return MakeOrthographic(width, height, nearClip, farClip);
}

Matrix<float, 4, 4> Matrix<float, 4, 4>::MakeViewPort(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix<float, 4, 4> result;

	result[0][0] = width * 0.5f;
	result[1][1] = height * - 0.5f;
	result[2][2] = maxDepth - minDepth;
	result[3][3] = 1.0f;

	result[3][0] = left + (width * 0.5f);
	result[3][1] = top + (height * 0.5f);
	result[3][2] = minDepth;

	return result;
}

Matrix<float,4,4> Matrix<float,4,4>::DirectionToDirection(const Vector3& from, const Vector3& to) {
	Vector3 normal;

	if (from.Dot(to) == -1.0f) {
		if (from.x != 0.0f or from.y != 0.0f) {
			normal = Vector3{ from.y, -from.x, 0.0f }.Normalize();
		}
		else if (from.x != 0.0f or from.z != 0.0f) {
			normal = Vector3{ from.z, 0.0f, -from.x }.Normalize();
		}
	}
	else {
		normal = from.Cross(to).Normalize();
	}


	float theataCos = from.Dot(to);
	float theataSin = from.Cross(to).Length();

	Matrix<float,4,4> result = 
		vector_type{
			normal.x * normal.x * (1.0f - theataCos) + theataCos,
			normal.x * normal.y * (1.0f - theataCos) + normal.z * theataSin,
			normal.x * normal.z * (1.0f - theataCos) - normal.y * theataSin,
			0.0f,

			normal.y * normal.x * (1.0f - theataCos) - normal.z * theataSin,
			normal.y * normal.y * (1.0f - theataCos) + theataCos,
			normal.y * normal.z * (1.0f - theataCos) + normal.x * theataSin,
			0.0f,

			normal.z * normal.x * (1.0f - theataCos) + normal.y * theataSin,
			normal.z * normal.y * (1.0f - theataCos) - normal.x * theataSin,
			normal.z * normal.z * (1.0f - theataCos) + theataCos,
			0.0f,

			0.0f,0.0f,0.0f,1.0f
		};


	return result;
}

Matrix<float,4,4> Matrix<float,4,4>::MakeRotateAxisAngle(const Vector3& axis, float angle) {
	return DirectX::XMMatrixRotationAxis({ axis.x,axis.y,axis.z }, angle);
}


Vector3 Matrix<float,4,4>::GetTranslate() const
{
	return Vector3(matrix_[3][0], matrix_[3][1], matrix_[3][2]);
}
Vector3 Matrix<float,4,4>::GetScale() const {
	return Vector3(
		Lamb::Math::Length({ matrix_[0][0], matrix_[0][1], matrix_[0][2] }),
		Lamb::Math::Length({ matrix_[1][0], matrix_[1][1], matrix_[1][2] }),
		Lamb::Math::Length({ matrix_[2][0], matrix_[2][1], matrix_[2][2]})
	);
}
Quaternion Matrix<float,4,4>::GetRotate() const {
	Quaternion rotate;

	rotate.m128 = DirectX::XMQuaternionRotationMatrix(this->xmMatrix_);

	return rotate.Normalize();
}

void Matrix<float,4,4>::Decompose(Vector3& scale, Quaternion& rotate, Vector3& translate) const {
	DirectX::XMVECTOR outScale;
	DirectX::XMVECTOR outTranslate;

	DirectX::XMMatrixDecompose(
		&outScale,
		&rotate.m128,
		&outTranslate,
		this->xmMatrix_
		);

	DirectX::XMStoreFloat3(std::bit_cast<DirectX::XMFLOAT3*>(scale.data()), outScale);
	DirectX::XMStoreFloat3(std::bit_cast<DirectX::XMFLOAT3*>(translate.data()), outTranslate);
}

void Matrix<float,4,4>::Decompose(Vector3& scale, Vector3& rotate, Vector3& translate) const {
	Quaternion quaternion;

	DirectX::XMVECTOR outScale;
	DirectX::XMVECTOR outTranslate;


	DirectX::XMMatrixDecompose(
		&outScale,
		&quaternion.m128,
		&outTranslate,
		this->xmMatrix_
	);

	DirectX::XMStoreFloat3(std::bit_cast<DirectX::XMFLOAT3*>(scale.data()), outScale);
	DirectX::XMStoreFloat3(std::bit_cast<DirectX::XMFLOAT3*>(translate.data()), outTranslate);

	rotate = Quaternion::QuaternionToEuler(quaternion);
}