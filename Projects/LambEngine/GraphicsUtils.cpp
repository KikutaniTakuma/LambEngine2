#include "GraphicsUtils.h"

#include "Drawer/DrawerManager.h"

#include <Engine/Core/EffekseerControler/EffekseerControler.h>

namespace Lamb {

	TextureID LoadTexture(const std::string& fileName)
	{
		Lamb::SafePtr draweManager = DrawerManager::GetInstance();

		draweManager->LoadTexture(fileName);

		TextureID id = draweManager->GetTexture(fileName);

		return id;
	}

	ModelID LoadModel(const std::string& fileName)
	{
		Lamb::SafePtr draweManager = DrawerManager::GetInstance();

		ModelID id = draweManager->LoadModel(fileName);

		return id;
	}

	void DrawTexture(
		TextureID textureID, 
		const Mat4x4& worldMatrix, 
		const Mat4x4& uvTransform, 
		const Mat4x4& camera, 
		uint32_t color, 
		BlendType blend
	) {
		Lamb::SafePtr tex2D = DrawerManager::GetInstance()->GetTexture2D();

		tex2D->Draw(
			worldMatrix,
			uvTransform,
			camera,
			textureID,
			color,
			blend
		);
	}

	void DrawModel(
		ModelID modelID, 
		const Mat4x4& worldMatrix, 
		const Mat4x4& camera, 
		uint32_t color, 
		BlendType blend, 
		const ModelShaderData& shaderData
	) {
		Lamb::SafePtr model = DrawerManager::GetInstance()->GetModel(modelID);

		model->Draw(
			worldMatrix,
			camera,
			color,
			blend,
			Model::ShaderData{
				.isLighting = shaderData.isLighting,
				.isEffect = shaderData.isEffect,
				.backGroundTextureIndex = 0,
				.gausState = {
					.dir = shaderData.gausState.dir,
					.sigma = shaderData.gausState.sigma,
					.kernelSize = shaderData.gausState.kernelSize
				}
			}
		);
	}

	const SafePtr<const Descriptor> GetTextureDescriptor(const std::string& fileName)
	{
		return TextureManager::GetInstance()->GetTexture(fileName)->GetBaseClassPtr();
	}

	Vector2 GetTexturePixelSize(const std::string& fileName)
	{
		return TextureManager::GetInstance()->GetTexture(fileName)->getSize();
	}

	void EffekseerUpdate(float time, const Mat4x4& viewMatrix, const Mat4x4& projectionMatrix) {
		EffekseerControler::GetInstance()->Update(
			time,
			viewMatrix,
			projectionMatrix
		);
	}

	void EffekseerSetUpdateInterval(float freq)
	{
		EffekseerControler::GetInstance()->SetUpdateFrequency(freq);
	}


}
