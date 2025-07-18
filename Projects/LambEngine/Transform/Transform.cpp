#include "Transform.h"

#ifdef USE_DEBUG_CODE
#include "imgui.h"
#include "ImGuizmo.h"
#include <xhash>
#endif // USE_DEBUG_CODE


QuaternionTransform& QuaternionTransform::operator=(const Transform& transform) {
	this->scale = transform.scale;
	this->rotate = Quaternion::EulerToQuaternion(transform.rotate);
	this->translate = transform.translate;

	return *this;
}

Transform& Transform::operator=(const QuaternionTransform& transform) {
	this->scale = transform.scale;
	this->rotate = Quaternion::QuaternionToEuler(transform.rotate);
	this->translate = transform.translate;

	return *this;
}

Mat4x4 Transform::CreateMatrix() const
{
	return Mat4x4::MakeAffin(scale, rotate, translate);
}

void Transform::Debug([[maybe_unused]]const std::string& guiName) {
#ifdef USE_DEBUG_CODE
	if (ImGui::TreeNode(guiName.c_str())) {
		ImGui::DragFloat3("スケール", scale.data(), 0.01f);
		ImGui::DragFloat3("回転", rotate.data(), 0.01f);
		ImGui::DragFloat3("ポジション", translate.data(), 0.01f);
		ImGui::TreePop();
	}
#endif // USE_DEBUG_CODE
}

Mat4x4 QuaternionTransform::CreateMatrix() const
{
	return Mat4x4::MakeAffin(scale, rotate, translate);
}

void QuaternionTransform::Debug([[maybe_unused]] const std::string& guiName) {
#ifdef USE_DEBUG_CODE
	if (ImGui::TreeNode(guiName.c_str())) {
		ImGui::DragFloat3("スケール", scale.data(), 0.01f);
		ImGui::DragFloat4("回転", rotate.data(), 0.01f);
		rotate = rotate.Normalize();
		ImGui::DragFloat3("ポジション", translate.data(), 0.01f);
		ImGui::TreePop();
	}
#endif // USE_DEBUG_CODE
}

namespace Lamb {
	namespace Guizmo {
		void Manipulate(
			[[maybe_unused]]Mat4x4& worldMatrix, 
			[[maybe_unused]]const Mat4x4& view, 
			[[maybe_unused]]const Mat4x4& ndc, 
			[[maybe_unused]]Type type
		) {
#ifdef USE_DEBUG_CODE
			ImGuizmo::SetID(static_cast<int>(std::hash<float*>()(worldMatrix.data())));

			ImGuizmo::Manipulate(
				view.data(),
				ndc.data(),
				static_cast<ImGuizmo::OPERATION>(type),
				ImGuizmo::WORLD,
				worldMatrix.data()
			);
#endif // USE_DEBUG_CODE
		}
	};
};