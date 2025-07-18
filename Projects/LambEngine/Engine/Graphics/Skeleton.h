/// ===========================
/// ==  Skeltonクラスの宣言  ==
/// ===========================


#pragma once
#include <optional>
#include <vector>
#include <unordered_map>

#include "Transform/Transform.h"
#include "GraphicsStructs.h"


struct Joint {
	Joint() = default;
	Joint(const Joint&) = default;
	Joint(Joint&&) = default;
	~Joint() = default;

	Joint& operator=(const Joint&) = default;
	Joint& operator=(Joint&&) = default;

	QuaternionTransform transform;
	Mat4x4 localMatrix;
	Mat4x4 skeletonSpaceMatrix;
	std::string name;
	std::vector<int32_t> children;
	int32_t index = 0;
	std::optional<int32_t> parent;
};

struct Skeleton {
	Skeleton() = default;
	Skeleton(const Skeleton&) = default;
	Skeleton(Skeleton&&) = default;
	~Skeleton() = default;

	Skeleton& operator=(const Skeleton&) = default;
	Skeleton& operator=(Skeleton&&) = default;

	void Update();
	void Draw(const Mat4x4& worldMatrix, const Mat4x4& camera);

	int32_t root = 0;
	std::unordered_map<std::string, int32_t> jointMap;
	std::vector<Joint> joints;
};

struct VertexInfluence {
	static constexpr uint32_t kNumMaxInfluence = 8u;
	std::array<float, kNumMaxInfluence> weights;
	std::array<int32_t, kNumMaxInfluence> jointIndices;
};

struct WellForGpu {
	Mat4x4 skeletonSpaceMatrix;
	Mat4x4 skeletonSpaceInverseTransposeMatrix;
};

struct SkinCluster {
public:
	static [[nodiscard]] SkinCluster* CreateSkinCluster(const Skeleton& skeleton, const ModelData& modelData);

private:
	SkinCluster() = default;

public:
	SkinCluster(const SkinCluster&) = default;
	SkinCluster(SkinCluster&&) noexcept = default;

	SkinCluster& operator=(const SkinCluster&) = default;
	SkinCluster& operator=(SkinCluster&&) noexcept = default;

public:
	~SkinCluster();

public:
	void Update(const Skeleton& skeleton);

	std::vector<Mat4x4> inverseBindPoseMatrices;
	Lamb::LambPtr<ID3D12Resource> influenceResource;
	D3D12_VERTEX_BUFFER_VIEW infliuenceBufferView{};
	std::span<VertexInfluence> mappedInfluence;
	std::array<StructuredBuffer<WellForGpu>, DirectXSwapChain::kBackBufferNumber> paletteBuffer;
};

namespace Lamb {
	Skeleton CreateSkeleton(const Node& rootNode);

	int32_t CreateJoint(
		const Node& rootNode,
		const std::optional<int32_t> parent,
		std::vector<Joint>& joints
	);
}