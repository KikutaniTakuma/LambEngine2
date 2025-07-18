/// ==================================
/// ==  PipelineObjectクラスの定義  ==
/// ==================================


#include "PipelineObject.h"
#include "Engine/Graphics/Shader/ShaderManager/ShaderManager.h"
#include <cassert>

void PipelineObject::LoadShader(
	const std::string& vsFileName,
	const std::string& psFileName
) {
	static ShaderManager* const shaderManager = ShaderManager::GetInstance();
	shader_.vertex = shaderManager->LoadVertexShader(vsFileName);
	shader_.pixel = shaderManager->LoadPixelShader(psFileName);
}