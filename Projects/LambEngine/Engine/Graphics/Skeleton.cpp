/// ===========================
/// ==  Skeltonクラスの定義  ==
/// ===========================



#include "Skeleton.h"
#include "Engine/Core/DescriptorHeap/CbvSrvUavHeap.h"
#include <algorithm>

#include "Utils/EngineInfo.h"

#ifdef USE_DEBUG_CODE
#include "Drawer/Line/Line.h"
#endif // USE_DEBUG_CODE

namespace Lamb {
    Skeleton CreateSkeleton(const Node& rootNode)
    {
        Skeleton skeleton;
        skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);

        for (const Joint& joint : skeleton.joints) {
            skeleton.jointMap.emplace(joint.name, joint.index);
        }

        skeleton.Update();

        return skeleton;
    }

    int32_t CreateJoint(const Node& rootNode, const std::optional<int32_t> parent, std::vector<Joint>& joints)
    {
        Joint joint;
        joint.name = rootNode.name;
        joint.localMatrix = rootNode.loacalMatrix;
        joint.skeletonSpaceMatrix = Mat4x4::kIdentity;
        joint.transform = rootNode.transform;
        joint.index = static_cast<int32_t>(joints.size());
        joint.parent = parent;
        joints.push_back(joint);
        for (const Node& child : rootNode.children) {
            int32_t childIndex = CreateJoint(child, joint.index, joints);
            joints[joint.index].children.push_back(childIndex);
        }
        return joint.index;
    }
}

void Skeleton::Update() {
    for (Joint& joint : joints) {
        joint.localMatrix = joint.transform.CreateMatrix();
        if (joint.parent) {
            joint.skeletonSpaceMatrix = joint.localMatrix * joints[*joint.parent].skeletonSpaceMatrix;
        }
        else {
            joint.skeletonSpaceMatrix = joint.localMatrix;
        }
    }
}

void Skeleton::Draw([[maybe_unused]] const Mat4x4& worldMatrix, [[maybe_unused]]const Mat4x4& camera) {
#ifdef USE_DEBUG_CODE
    for (Joint& joint : joints) {
        if (not joint.parent) {
            continue;
        }
        Vector3&& start = joint.skeletonSpaceMatrix.GetTranslate();
        Vector3&& end = joints[*joint.parent].skeletonSpaceMatrix.GetTranslate();


        Line::Draw(
            start * worldMatrix,
            end * worldMatrix,
            camera,
            std::numeric_limits<uint32_t>::max()
        );
    }
#endif // USE_DEBUG_CODE
}

SkinCluster* SkinCluster::CreateSkinCluster(const Skeleton& skeleton, const ModelData& modelData)
{
    Lamb::SafePtr<CbvSrvUavHeap> heap = CbvSrvUavHeap::GetInstance();
    Lamb::SafePtr device = DirectXDevice::GetInstance();

    std::unique_ptr<SkinCluster> result;
    result.reset(new SkinCluster());
    std::for_each(
        result->paletteBuffer.begin(),
        result->paletteBuffer.end(),
        [&skeleton](auto& n) {
            n.CreateBuffer(static_cast<uint32_t>(skeleton.joints.size()));
        }
    );



    uint32_t bufferSize = static_cast<uint32_t>(sizeof(VertexInfluence) * modelData.vertices.size());
    result->influenceResource = device->CreateBufferResuorce(bufferSize);
    Lamb::SafePtr<VertexInfluence> mappedInfluence;
    result->influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
    std::memset(mappedInfluence.get(), 0, bufferSize);
    result->mappedInfluence = { mappedInfluence.release(), modelData.vertices.size() };

    result->infliuenceBufferView.BufferLocation = result->influenceResource->GetGPUVirtualAddress();
    result->infliuenceBufferView.SizeInBytes = bufferSize;
    result->infliuenceBufferView.StrideInBytes = sizeof(VertexInfluence);

    result->inverseBindPoseMatrices.resize(skeleton.joints.size());
    std::ranges::fill(result->inverseBindPoseMatrices, Mat4x4::kIdentity);


    for (const auto& jointWeight : modelData.skinClusterData) {
        auto itr = skeleton.jointMap.find(jointWeight.first);
        if (itr == skeleton.jointMap.end()) {
            continue;
        }

        result->inverseBindPoseMatrices[itr->second] = jointWeight.second.inverseBindPoseMatrix;
        for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
            auto& currentInfuluence = result->mappedInfluence[vertexWeight.vertexIndex];
            for (uint32_t index = 0; index < VertexInfluence::kNumMaxInfluence; index++) {
                if (currentInfuluence.weights[index] == 0.0f) {
                    currentInfuluence.weights[index] = vertexWeight.weight;
                    currentInfuluence.jointIndices[index] = itr->second;
                    break;
                }
            }
        }
    }


    return result.release();
}

SkinCluster::~SkinCluster()
{
}

void SkinCluster::Update(const Skeleton& skeleton) {
    for (size_t jointIndex = 0; jointIndex < skeleton.joints.size(); jointIndex++) {
        if (skeleton.joints.size() <= jointIndex) {
            throw Lamb::Error::Code<SkinCluster>("jointIndex is over to joints size of skeleton", ErrorPlace);
        }

        this->paletteBuffer[Lamb::GetGraphicBufferIndex()][jointIndex].skeletonSpaceMatrix =
            this->inverseBindPoseMatrices[jointIndex] * skeleton.joints[jointIndex].skeletonSpaceMatrix;
        this->paletteBuffer[Lamb::GetGraphicBufferIndex()][jointIndex].skeletonSpaceInverseTransposeMatrix =
            this->paletteBuffer[Lamb::GetGraphicBufferIndex()][jointIndex].skeletonSpaceMatrix.Inverse().Transepose();
    }
}
