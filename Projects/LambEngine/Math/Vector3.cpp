#include "Vector3.h"
#include <cmath>
#include <numbers>
#include "Vector2.h"
#include "Quaternion.h"
#include "MathCommon.h"

#include "Error/Error.h"

constexpr Vector3::Vector3() noexcept :
	x(0.0f),
	y(0.0f),
	z(0.0f)
{}

Vector3::Vector3(float x, float y, float z) noexcept :
	x(x),
	y(y),
	z(z)
{}

Vector3::Vector3(const Vector2& right) noexcept {
	x = right.x;
	y = right.y;
	z = 0.0f;
}

Vector3::Vector3(const Vector2& right, float rightZ) noexcept {
	x = right.x;
	y = right.y;
	z = rightZ;
}

const Vector3 Vector3::kIdentity = { 1.0f,1.0f,1.0f };
const Vector3 Vector3::kZero = { 0.0f, 0.0f,0.0f };
const Vector3 Vector3::kXIdentity = { 1.0f,0.0f,0.0f };
const Vector3 Vector3::kYIdentity = { 0.0f,1.0f,0.0f };
const Vector3 Vector3::kZIdentity = { 0.0f,0.0f,1.0f };

Vector3 Vector3::operator+() const noexcept {
	return *this;
}
Vector3 Vector3::operator-() const noexcept {
	return Vector3(-x, -y, -z);
}

Vector3 Vector3::operator+(const Vector3& right) const noexcept {
	return Vector3(x + right.x, y + right.y, z + right.z);
}
Vector3 Vector3::operator-(const Vector3& right) const noexcept {
	return Vector3(x - right.x, y - right.y, z - right.z);
}
Vector3& Vector3::operator+=(const Vector3& right) noexcept {
	*this = *this + right;

	return *this;
}
Vector3& Vector3::operator-=(const Vector3& right) noexcept {
	*this = *this - right;

	return *this;
}

Vector3 Vector3::operator*(const Vector3& right) const noexcept {
	return Vector3(x * right.x, y * right.y, z * right.z);
}
Vector3& Vector3::operator*=(const Vector3& right) noexcept {
	*this = *this * right;

	return *this;
}

Vector3 Vector3::operator*(float scalar) const noexcept {
	return Vector3(x * scalar, y * scalar, z * scalar);
}
Vector3& Vector3::operator*=(float scalar) noexcept {
	*this = *this * scalar;

	return *this;
}

Vector3 Vector3::operator/(float scalar) const noexcept {
	return *this * (1.0f / scalar);
}
Vector3& Vector3::operator/=(float scalar) noexcept {
	*this = *this / scalar;

	return *this;
}

Vector3 Vector3::operator+(float right) const noexcept
{
	return Vector3(x + right, y + right, z + right);
}

Vector3 Vector3::operator-(float right) const noexcept
{
	return Vector3(x - right, y - right, z - right);
}

Vector3 operator+(float right, const Vector3& left) noexcept
{
	return left + right;
}

Vector3 operator-(float right, const Vector3& left) noexcept
{
	return Vector3(right - left.x, right - left.y, right - left.z);
}

Vector3 operator*(float right, const Vector3& left) noexcept
{
	return left * right;
}

Vector3 operator/(float right, const Vector3& left) noexcept
{
	return Vector3(right / left.x, right / left.y, right / left.z);
}


Vector3 Vector3::operator*(const Mat4x4& mat) const {
	Vector3 result;
	
	DirectX::XMVECTOR&& xmVector = DirectX::XMVector3Transform({ x,y,z }, mat.GetXMMatrix());
	DirectX::XMStoreFloat3(reinterpret_cast<DirectX::XMFLOAT3*>(result.data()), xmVector);

	return result;
}

Vector3& Vector3::operator*=(const Mat4x4& mat) {
	*this = *this * mat;

	return *this;
}

Vector3& Vector3::operator=(const Vector2& right) noexcept {
	x = right.x;
	y = right.y;

	return *this;
}

Vector3 Vector3::operator*(const Quaternion& right) const {
	return (right * Quaternion{ *this, 0.0f } * right.Inverce()).vector.vector3;
}

Vector3& Vector3::operator*=(const Quaternion& right) {
	*this = *this * right;

	return *this;
}

bool Vector3::operator==(const Vector3& right) const noexcept {
	return x == right.x && y == right.y && z == right.z;
}
bool Vector3::operator!=(const Vector3& right) const noexcept {
	return x != right.x || y != right.y || z != right.z;
}

float& Vector3::operator[](size_t index) {
	if (size() <= index) [[unlikely]] {
		throw Lamb::Error::Code<Vector3>("index is over", ErrorPlace);
	}
	return data()[index];
}

const float& Vector3::operator[](size_t index) const {
	if (size() <= index) [[unlikely]] {
		throw Lamb::Error::Code<Vector3>("index is over", ErrorPlace);
	}
	return data()[index];
}

float Vector3::Length() const noexcept {
	return std::sqrt(Dot(*this));
}
float Vector3::LengthSQ() const noexcept {
	return Dot(*this);
}

float Vector3::Dot(const Vector3& right) const noexcept {
	return x * right.x + y * right.y + z * right.z;
}

Vector3 Vector3::Cross(const Vector3& right) const noexcept {
	Vector3 result;

	result.x = y * right.z - z * right.y;
	result.y = z * right.x - x * right.z;
	result.z = x * right.y - y * right.x;

	return result;
}

Vector3 Vector3::Normalize() const noexcept {
	if (*this == Vector3::kZero) {
		return Vector3::kZero;
	}

	return *this / this->Length();
}


float& Vector3::at(size_t index)
{
	if (size() <= index) {
		throw Lamb::Error::Code<Vector3>("index is over", ErrorPlace);
	}
	return data()[index];
}
const float& Vector3::at(size_t index) const {
	if (size() <= index) {
		throw Lamb::Error::Code<Vector3>("index is over", ErrorPlace);
	}
	return data()[index];
}

Vector3 Vector3::Lerp(const Vector3& start, const Vector3& end, float t) {
	Vector3 result;

	result.x = std::lerp(start.x, end.x, t);
	result.y = std::lerp(start.y, end.y, t);
	result.z = std::lerp(start.z, end.z, t);

	return result;
}

Vector3 Vector3::Lerp(const Vector3& start, const Vector3& end, const Vector3& t)
{
	Vector3 result;

	result.x = std::lerp(start.x, end.x, t.x);
	result.y = std::lerp(start.y, end.y, t.y);
	result.z = std::lerp(start.z, end.z, t.z);

	return result;
}

Vector3 Vector3::CatmullRom(const Vector3& controlPoint0, const Vector3& controlPoint1, const Vector3& controlPoint2, const Vector3& controlPoint3, const float t) {
	Vector3 result{};
	result =
		((controlPoint0 * -1.0f + controlPoint1 * 3.0f + controlPoint2 * -3.0f + controlPoint3) * std::powf(t, 3.0f) +
			(controlPoint0 * 2.0f + controlPoint1 * -5.0f + controlPoint2 * 4.0f + controlPoint3 * -1.0f) * std::powf(t, 2.0f) +
			(controlPoint0 * -1.0f + controlPoint2) * t + controlPoint1 * 2.0f) * 0.5f;

	return result;
}

Vector3 Vector3::Step(const float a, const Vector3& x)
{
	return Vector3(Lamb::Step(a, x.x), Lamb::Step(a, x.y), Lamb::Step(a, x.z));
}

Vector3 Vector3::Reflect(const Vector3& i, const Vector3& n) {
	if (n == Vector3::kZero) {
		return n;
	}

	return i - (2.0f * i.Dot(n) * n);
}

Vector3 Vector3::ReflectNormal(const Vector3& i, const Vector3& r) {
	return (r - i).Normalize();
}

Vector3 Vector3::Avarage(std::initializer_list<Vector3> vecs) {
	if (vecs.size() == 0) {
		return Vector3::kZero;
	}

	Vector3 result;
	for (auto& i : vecs) {
		result += i;
	}
	
	return result / static_cast<float>(vecs.size());
}


Vector3 Vector3::Pararerl(const Vector3& i, const Vector3& n) {
	if (n == Vector3::kZero) {
		return n;
	}
	return i - (i.Dot(n) * n);
}

Vector3 Vector3::Clamp(const Vector3& num, const Vector3& min, const Vector3& max) {
	return Vector3(std::clamp(num.x, min.x, max.x), std::clamp(num.y, min.y, max.y), std::clamp(num.z, min.z, max.z));
}

Vector3 Vector3::TransformCoord(const Vector3& v, const Mat4x4& mat){
	Vector4 temp = Vector4(v.x, v.y, v.z, 1.0f); // 座標は w=1.0 で拡張
	temp = temp * mat; // 行列と掛ける

	// 同次座標変換（w で割る）
	if (temp.vec.w != 0.0f) {
		temp.vec.x /= temp.vec.w;
		temp.vec.y /= temp.vec.w;
		temp.vec.z /= temp.vec.w;
	}

	return Vector3(temp.vec.x, temp.vec.y, temp.vec.z);
}

Vector3 Project(const Vector3& vec1, const Vector3& vec2) {
	return vec2 * (vec1.Dot(vec2) / std::pow(vec2.Length(), 2.0f));
}

Vector3 ClosestPoint(const Vector3& point, const Segment& segment) {
	return segment.origin + Project(point - segment.origin, segment.diff);
}

void SetVector3ToDirectXXMFLOAT3(DirectX::XMFLOAT3& xmfloat3, const Vector3& vec3) {
	xmfloat3.x = vec3.x;
	xmfloat3.y = vec3.y;
	xmfloat3.z = vec3.z;
}
