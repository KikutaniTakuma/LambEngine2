/// =========================================
/// ==  VertexIndexDataLoaderクラスの定義  ==
/// =========================================


#include "VertexIndexDataLoader.h"
#include "Error/Error.h"
#include "Utils/SafePtr.h"
#include "../TextureManager/TextureManager.h"
#include "../../Core/DirectXDevice/DirectXDevice.h"

#include <filesystem>
#include <unordered_map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

std::chrono::steady_clock::time_point VertexIndexDataLoader::loadStartTime_;

ModelData VertexIndexDataLoader::LoadModel(const std::string& fileName)
{
	StartLoadTimeCount_();

	Assimp::Importer importer;
	Lamb::SafePtr<const aiScene> scene = ReadFile_(importer, fileName);
	if (not scene->HasMeshes()) [[unlikely]] {
		throw Lamb::Error::Code<VertexIndexDataLoader>("This file does not have meshes -> " + fileName, ErrorPlace);
	}

	std::filesystem::path path = fileName;
	bool isGltf = (path.extension() == ".gltf") or (path.extension() == ".glb");


	std::string&& directorypath = path.parent_path().string();
	std::vector<uint32_t> textures;

	LoadMtl_(scene.get(), directorypath, textures);
	
	ModelData result;

	// mesh解析
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		Lamb::SafePtr<aiMesh> mesh = scene->mMeshes[meshIndex];
		if (not mesh->HasNormals()) [[unlikely]] {
			throw Lamb::Error::Code<VertexIndexDataLoader>("This file does not have normal -> " + fileName, ErrorPlace);
		}
		if (not mesh->HasTextureCoords(0)) [[unlikely]] {
			throw Lamb::Error::Code<VertexIndexDataLoader>("This file does not have texcoord -> " + fileName, ErrorPlace);
		}

		// 要素数追加
		result.vertices.reserve(result.vertices.size() + mesh->mNumVertices);

		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; vertexIndex++) {
			aiVector3D& position = mesh->mVertices[vertexIndex];
			aiVector3D& normal = mesh->mNormals[vertexIndex];
			aiVector3D& texcoord = mesh->mTextureCoords[0][vertexIndex];

			result.vertices.push_back(
				Vertex{
					.pos = { -position.x,position.y,position.z, 1.0f},
					.normal = {-normal.x,normal.y,normal.z},
					.uv = {texcoord.x, texcoord.y},
					.textureID = textures.empty() ? 0 : textures[mesh->mMaterialIndex - (isGltf ? 0 : 1)]
				}
			);
		}

		uint32_t latestIndexSize = static_cast<uint32_t>(result.indices.size());
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++) {
			aiFace& face = mesh->mFaces[faceIndex];
			if (face.mNumIndices != 3) {
				throw Lamb::Error::Code<VertexIndexDataLoader>("face indices is not 3", ErrorPlace);
			}

			for (uint32_t element = 0; element < face.mNumIndices; element++) {
				uint32_t vertexIndex = face.mIndices[element];
				result.indices.push_back(latestIndexSize + vertexIndex);
			}
		}

		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {
			aiBone* bone = mesh->mBones[boneIndex];
			std::string jointName = bone->mName.C_Str();
			JointWeightData& jointWeightData = result.skinClusterData[jointName];

			aiMatrix4x4 bindMatrixAssimp = bone->mOffsetMatrix.Inverse();
			aiVector3D scale, translate;
			aiQuaternion rotate;
			bindMatrixAssimp.Decompose(scale, rotate, translate);
			Mat4x4 bindPoseMatrix = Mat4x4::MakeAffin(
				{ scale.x, scale.y, scale.z },
				{ rotate.x, -rotate.y, -rotate.z, rotate.w },
				{ -translate.x, translate.y, translate.z }
			);
			jointWeightData.inverseBindPoseMatrix = bindPoseMatrix.Inverse();

			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++) {
				jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight,bone->mWeights[weightIndex].mVertexId });
			}
		}
	}

	result.rootNode = ReadNode_(scene->mRootNode);


	EndLoadTimeCountAndAddLog_(fileName);


	return result;
}

Animations* VertexIndexDataLoader::LoadAnimation(const std::string& fileName)
{
	StartLoadTimeCount_();

	std::unique_ptr result = std::make_unique<Animations>();
	Assimp::Importer importer;
	Lamb::SafePtr<const aiScene> scene = ReadFile_(importer, fileName);
	if (not (scene->mNumAnimations != 0)) [[unlikely]] {
		throw Lamb::Error::Code<VertexIndexDataLoader>("This file does not have animation -> " + fileName, ErrorPlace);
	}

	result->data.resize(scene->mNumAnimations);

	for (uint32_t animtionIndex = 0; animtionIndex < scene->mNumAnimations; animtionIndex++) {
		Lamb::SafePtr<aiAnimation> animationAssimp = scene->mAnimations[animtionIndex];
		Animation& animation = result->data[animtionIndex];
		animation.duration = static_cast<float>(animationAssimp->mDuration / animationAssimp->mTicksPerSecond);
		
		for (uint32_t channelIndex = 0; channelIndex < animationAssimp->mNumChannels; channelIndex++) {
			Lamb::SafePtr<aiNodeAnim> nodeAnimationAssimp = animationAssimp->mChannels[channelIndex];
			NodeAnimation& nodeAnimation = animation.nodeAnimations[nodeAnimationAssimp->mNodeName.C_Str()];

			// Translation
			for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumPositionKeys; keyIndex++) {
				aiVectorKey& translation = nodeAnimationAssimp->mPositionKeys[keyIndex];
				KeyFrameVector3 keyFrame;

				keyFrame.time = static_cast<float>(translation.mTime / animationAssimp->mTicksPerSecond);
				keyFrame.value = { -translation.mValue.x, translation.mValue.y, translation.mValue.z };
				nodeAnimation.translation.keyFrames.push_back(keyFrame);
			}
			// rotation
			for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumRotationKeys; keyIndex++) {
				aiQuatKey& rotate = nodeAnimationAssimp->mRotationKeys[keyIndex];
				KeyFrameQuaternion keyFrame;

				keyFrame.time = static_cast<float>(rotate.mTime / animationAssimp->mTicksPerSecond);
				keyFrame.value = { rotate.mValue.x, -rotate.mValue.y, -rotate.mValue.z,  rotate.mValue.w };
				nodeAnimation.rotate.keyFrames.push_back(keyFrame);
			}
			// scale
			for (uint32_t keyIndex = 0; keyIndex < nodeAnimationAssimp->mNumScalingKeys; keyIndex++) {
				aiVectorKey& scale = nodeAnimationAssimp->mScalingKeys[keyIndex];
				KeyFrameVector3 keyFrame;

				keyFrame.time = static_cast<float>(scale.mTime / animationAssimp->mTicksPerSecond);
				keyFrame.value = { scale.mValue.x, scale.mValue.y, scale.mValue.z };
				nodeAnimation.sacle.keyFrames.push_back(keyFrame);
			}
		}
	}

	EndLoadTimeCountAndAddLog_(fileName);


	return result.release();
}

const aiScene* VertexIndexDataLoader::ReadFile_(Assimp::Importer& importer, const std::string& fileName)
{
	std::filesystem::path path = fileName;

	// ファイル見つかんない
	if (not std::filesystem::exists(path)) [[unlikely]] {
		throw Lamb::Error::Code<VertexIndexDataLoader>("This file does not find -> " + fileName, ErrorPlace);
	}
	// objかgltfかglbではない
	if (not (path.extension() == ".obj" or path.extension() == ".gltf" or path.extension() == ".glb")) [[unlikely]] {
		throw Lamb::Error::Code<VertexIndexDataLoader>("This file(" + path.extension().string() + ") does not support -> " + fileName, ErrorPlace);
	}

	return importer.ReadFile(fileName.c_str(), aiProcess_FlipWindingOrder | aiProcess_FlipUVs);
}

Node VertexIndexDataLoader::ReadNode_(aiNode* node)
{
	Node result;
	aiVector3D scale, translate;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, translate);

	result.transform.scale = { scale.x, scale.y, scale.z };
	result.transform.rotate = { rotate.x,-rotate.y,-rotate.z,rotate.w };
	result.transform.translate = { -translate.x, translate.y, translate.z };
	result.loacalMatrix = result.transform.CreateMatrix();

	result.name = node->mName.C_Str();
	result.children.resize(node->mNumChildren);

	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; childIndex++) {
		result.children[childIndex] = ReadNode_(node->mChildren[childIndex]);
	}

	return result;
}

void VertexIndexDataLoader::LoadMtl_(const aiScene* scene, const std::string& directorypath, std::vector<uint32_t>& result)
{
	std::vector<std::string> textureFileNames;

	for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; ++materialIndex) {
		Lamb::SafePtr<aiMaterial> material = scene->mMaterials[materialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0) {
			aiString textureFilePath;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFilePath);
			std::filesystem::path tmpTextureFilePath = textureFilePath.C_Str();
			textureFileNames.push_back(directorypath + "/" + tmpTextureFilePath.filename().string());
		}
	}

	TextureManager* const textureMaanger = TextureManager::GetInstance();

	for (const auto& i : textureFileNames) {
		textureMaanger->LoadTexture(i);
		result.push_back(textureMaanger->GetHandle(i));
	}
}

void VertexIndexDataLoader::StartLoadTimeCount_() {
	loadStartTime_ = std::chrono::steady_clock::now();
}

void VertexIndexDataLoader::EndLoadTimeCountAndAddLog_(const std::string& fileName) {
	auto loadEndTime = std::chrono::steady_clock::now();

	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(loadEndTime - loadStartTime_);

	Lamb::AddLog("Load succeeded : " + fileName + " : " +  Lamb::TimeToString(time));
}
